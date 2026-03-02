#include "viewportEngine.h"
#include "camera/usdCamera.h"

#include <pxr/imaging/glf/drawTarget.h>
#include <pxr/imaging/hd/rendererPluginRegistry.h>
#include <pxr/imaging/hgi/tokens.h>
#include <pxr/imaging/hgiGL/texture.h>
#include <pxr/usd/usd/stage.h>

PXR_NAMESPACE_USING_DIRECTIVE

namespace HVW_NS
{

void ViewportEngine::initialize(const PXR_NS::UsdStageRefPtr& stage)
{
    // create the Render Delegate (Storm in this case)
    PXR_NS::HdRendererPluginRegistry& registry = PXR_NS::HdRendererPluginRegistry::GetInstance();
    m_renderDelegatePtr = registry.CreateRenderDelegate(pxr::TfToken("HdStormRendererPlugin"));

    // create Hgi (Graphics Interface) + Driver
    // You can also use a Helper function ( CreatePlatformDefaultHgi ) 
    // to return a Hgi object for the current platform.
    m_hgiPtr = Hgi::CreateNamedHgi(HgiTokens->OpenGL);
    m_hgiDriver = { PXR_NS::HgiTokens->renderDriver, PXR_NS::VtValue(m_hgiPtr.get()) };
    m_renderIndexPtr.reset(PXR_NS::HdRenderIndex::New( m_renderDelegatePtr.Get(), {&m_hgiDriver} ));

    // create the Render Index
    m_sceneDelegatePtr = std::make_unique<PXR_NS::UsdImagingDelegate>(m_renderIndexPtr.get(), PXR_NS::SdfPath("/"));

    // populate Hydra from the USD stage
    m_sceneDelegatePtr->Populate(stage->GetPseudoRoot());

    // task controller
    auto controllerID = "/MyUniqueTaskControllerID";
    m_taskControllerPtr = std::make_unique<PXR_NS::HdxTaskController>(m_renderIndexPtr.get(), PXR_NS::SdfPath(controllerID));

    // Specify which render tags we want to draw
    // Render tags determine which categories of primitives will be drawn.
    // Common tags include:
    // geometry: Regular scene geometry
    // render: Renderable objects
    // guide: Guides and helpers
    // proxy: Proxy representations
    m_taskControllerPtr->SetRenderTags({ PXR_NS::HdRenderTagTokens->geometry, PXR_NS::HdRenderTagTokens->render });

    // Enable / disable presenting the render to bound framebuffer.
    // When enabled: Hydra renders into an to a bound framebuffer.bliting happens using HdxPresentTask
    // If disabled: You are responsible for presenting the result yourself.
    m_taskControllerPtr->SetEnablePresentation(false);

    // Set the list of outputs to be rendered. You could add extra outout ( e.g depth, normal, primId, etc. )
    m_taskControllerPtr->SetRenderOutputs({ PXR_NS::HdAovTokens->color });

    // Color correction
    PXR_NS::HdxColorCorrectionTaskParams colorParams;
    colorParams.colorCorrectionMode = PXR_NS::HdxColorCorrectionTokens->sRGB;
    m_taskControllerPtr->SetColorCorrectionParams(colorParams);

    // We need to SetTaskContextData for selection
    // Without it, Hydra may throw errors when tasks try to access the selection state:
    // Coding Error: in _GetTaskContextData Token selectionState missing from task context
    HdxSelectionTrackerSharedPtr selectionTracker = std::make_shared<pxr::HdxSelectionTracker>();
    m_engine.SetTaskContextData(
        pxr::HdxTokens->selectionState,
        pxr::VtValue(selectionTracker)
    );

    // Create a simple light context
    m_pLightingContext = PXR_NS::GlfSimpleLightingContext::New();
}

ViewportEngine::~ViewportEngine()
{
    // The order is important here
    m_taskControllerPtr  = nullptr;
    m_sceneDelegatePtr   = nullptr;
    m_renderIndexPtr     = nullptr;
    m_renderDelegatePtr  = nullptr;
}

void ViewportEngine::render(const PXR_NS::UsdStageRefPtr& stage, 
                            UsdCamera* camera,
                            double width, double height)
{
    // Updating the camera state and Setting the view and projection matrices for the free camera
    camera->setAspectRatio(width / std::max(1.0, height));
    camera->updateTransform();
    auto cameraFrustum = camera->getCamera().GetFrustum();
    m_taskControllerPtr->SetFreeCameraMatrices(cameraFrustum.ComputeViewMatrix(), cameraFrustum.ComputeProjectionMatrix());

    // Camera Light
    auto cameraPosition = camera->getCamera().GetTransform().ExtractTranslation();
    m_cameraLight.SetAmbient(PXR_NS::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
    m_cameraLight.SetPosition(PXR_NS::GfVec4f((float)cameraPosition[0], (float)cameraPosition[1], (float)cameraPosition[2], 1.f));

    m_pLightingContext->SetLights({m_cameraLight});
    m_pLightingContext->SetSceneAmbient(PXR_NS::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
    m_pLightingContext->SetUseLighting(true);

    m_taskControllerPtr->SetLightingState(m_pLightingContext);

    // Optional: settoing the Color AOV Clear Value
    // AOVs are defined via HdAovDescriptor and managed by the HdRenderPassState. 
    // The backend renderer (e.g., HdStormRendererPlugin for OpenGL) populates these buffers during execution.
    PXR_NS::HdAovDescriptor aovDesc = m_taskControllerPtr->GetRenderOutputSettings(PXR_NS::HdAovTokens->color);
    aovDesc.clearValue = PXR_NS::VtValue(PXR_NS::GfVec4f(0.5f, 0.7f, 0.5f, 1.f));
    m_taskControllerPtr->SetRenderOutputSettings(PXR_NS::HdAovTokens->color, aovDesc);

    // define where Hydra should render within the framebuffer.
    // (0, 0) : lower-left corner
    // width, height : viewport dimensions in pixels.
    m_taskControllerPtr->SetRenderViewport(PXR_NS::GfVec4f(0, 0, static_cast<float>(width), static_cast<float>(height)));

    // Set the size of the render buffers baking the AOVs.
    // GUI applications should set this to the size of the window.
    m_taskControllerPtr->SetRenderBufferSize(PXR_NS::GfVec2i(width, height));

    // Render the camera image from (0,0) to (width,height) directly into a buffer of the same size
    PXR_NS::GfRange2f displayWindow(PXR_NS::GfVec2f(0, 0), PXR_NS::GfVec2f( static_cast<float>(width), static_cast<float>(height) ) );
    PXR_NS::GfRect2i renderBufferRect(PXR_NS::GfVec2i(0, 0), width, height);
    PXR_NS::CameraUtilFraming framing(displayWindow, renderBufferRect);
    m_taskControllerPtr->SetFraming(framing);

    // setting per-frame rendering options for Hydra tasks
    PXR_NS::HdxRenderTaskParams params;
    params.viewport = PXR_NS::GfVec4f(0, 0, static_cast<float>(width), static_cast<float>(height));
    params.enableLighting = true;
    m_taskControllerPtr->SetRenderParams(params);
    
    // This is the final step of our Hydra frame rendering
    // actually executing the tasks and producing the final rendered image.
    PXR_NS::HdTaskSharedPtrVector tasks = m_taskControllerPtr->GetRenderingTasks();
    m_engine.Execute(m_renderIndexPtr.get(), &tasks);
}

std::string ViewportEngine::rendererName() const
{
    return "HdStormRendererPlugin";
}

std::string ViewportEngine::hgiName() const
{
    return m_hgiPtr->GetAPIName().GetString();
}

uint32_t ViewportEngine::getColorAovTextureId() const
{
    PXR_NS::VtValue aov;
    //  ask the engine for the data associated with the "color" AOV.
    //  This is stored in the task context after rendering tasks execute.
    if (!m_engine.GetTaskContextData(PXR_NS::HdAovTokens->color, &aov)) {
        return 0;  // No color AOV data available → render probably didn't produce it or failed
    }

    // check if the retrieved value actually holds an HgiTextureHandle
    if (!aov.IsHolding<PXR_NS::HgiTextureHandle>()) {
        return 0;
    }

    // check if the texture handle is valid
    PXR_NS::HgiTextureHandle texHandle = aov.Get<PXR_NS::HgiTextureHandle>();
    if (!texHandle) {
        return 0;
    }

    // since we're using the OpenGL backend (Storm with HgiGL), cast to the GL-specific subclass.
    PXR_NS::HgiGLTexture* glTex = dynamic_cast<PXR_NS::HgiGLTexture*>(texHandle.Get());
    if (!glTex) {
        return 0;
    }

    // finally retrieve the OpenGL texture name/ID.
    return glTex->GetTextureId();
}

} // namespace HVW_NS
