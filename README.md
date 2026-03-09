# Minimal Viewport Implementation using Hydra 2.0 APIs

Switching to the Hydra 2.0 architecture also known as the Scene Index–based pipeline is surprisingly straightforward. Rather than implementing a monolithic scene delegate as in Hydra 1.0, Hydra 2.0 builds the scene representation through a chain of Scene Indices. Each Scene Index can transform, filter, or augment the scene data before passing it downstream. Once this chain is assembled, the resulting **finalSceneIndex** is inserted into the render index, which becomes the entry point for scene data flowing into Hydra.

After inserting this Scene Index into the render index, Hydra becomes aware of the scene graph and begins translating USD prims into Hydra primitives internally.

```cpp
PXR_NS::UsdImagingCreateSceneIndicesInfo info;
info.displayUnloadedPrimsWithBounds = false;
info.stage                          = stage;
const PXR_NS::UsdImagingSceneIndices sceneIndices = UsdImagingCreateSceneIndices(info);

PXR_NS::HdSceneIndexBaseRefPtr usdSceneIndex = sceneIndices.finalSceneIndex;
m_renderIndexPtr->InsertSceneIndex(usdSceneIndex, SdfPath::AbsoluteRootPath());
```
Everything else remains the same as before:

```cpp
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

    PXR_NS::UsdImagingCreateSceneIndicesInfo info;
    info.displayUnloadedPrimsWithBounds = false;
    info.stage                          = stage;
    const PXR_NS::UsdImagingSceneIndices sceneIndices = UsdImagingCreateSceneIndices(info);

    // insert scene index
    PXR_NS::HdSceneIndexBaseRefPtr usdSceneIndex = sceneIndices.finalSceneIndex;
    m_renderIndexPtr->InsertSceneIndex(usdSceneIndex, SdfPath::AbsoluteRootPath());

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
    m_taskControllerPtr->SetEnablePresentation(true);

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

    // setting per-frame rendering options for Hydra tasks
    PXR_NS::HdxRenderTaskParams params;
    params.enableLighting = true;
    m_taskControllerPtr->SetRenderParams(params);
    
    // This is the final step of our Hydra frame rendering
    // actually executing the tasks and producing the final rendered image.
    PXR_NS::HdTaskSharedPtrVector tasks = m_taskControllerPtr->GetRenderingTasks();
    m_engine.Execute(m_renderIndexPtr.get(), &tasks);
}
```
