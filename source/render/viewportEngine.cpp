#include "viewportEngine.h"
#include "camera/usdCamera.h"

#include <pxr/imaging/hgiGL/hgi.h>
#include <pxr/base/gf/frustum.h>

#include <hvt/engine/hgiInstance.h>
#include <hvt/engine/viewportEngine.h>

PXR_NAMESPACE_USING_DIRECTIVE

namespace HVW_NS
{

ViewportEngine::ViewportEngine()
{
    hvt::HgiInstance::instance().create(HgiTokens->OpenGL);
}

ViewportEngine::~ViewportEngine()
{
    // order is important
    m_sceneFramePass = nullptr;
    m_renderIndex = nullptr;
    hvt::HgiInstance::instance().destroy();
}

void ViewportEngine::initialize(PXR_NS::UsdStageRefPtr stage)
{
    hvt::RendererDescriptor renderDesc;
    renderDesc.hgiDriver    = hvt::HgiInstance::instance().hgiDriver();
    renderDesc.rendererName = hvt::HgiInstance::instance().defaultSceneRendererName();
    hvt::ViewportEngine::CreateRenderer(m_renderIndex, renderDesc);

    // Creates the scene index
    pxr::HdSceneIndexBaseRefPtr sceneIndex = hvt::ViewportEngine::CreateUSDSceneIndex(stage);

    m_renderIndex->RenderIndex()->InsertSceneIndex(sceneIndex, pxr::SdfPath::AbsoluteRootPath());

    // Create the frame pass
    hvt::FramePassDescriptor passDesc;
    passDesc.renderIndex = m_renderIndex->RenderIndex();
    passDesc.uid         = pxr::SdfPath("/sceneFramePass");
    m_sceneFramePass     = hvt::ViewportEngine::CreateFramePass(passDesc);
}

void ViewportEngine::render(UsdCamera* camera,
                            double width, double height)
{
    // Updating the camera state and Setting the view and projection matrices for the free camera
    camera->setAspectRatio(width / std::max(1.0, height));
    camera->updateTransform();
    auto cameraFrustum = camera->getCamera().GetFrustum();

    // Populate the fram pass parameters
    auto& params                     = m_sceneFramePass->params();
    params.renderBufferSize          = pxr::GfVec2i(width, height);
    params.viewInfo.viewMatrix       = cameraFrustum.ComputeViewMatrix();
    params.viewInfo.projectionMatrix = cameraFrustum.ComputeProjectionMatrix();
    params.viewInfo.framing          = hvt::ViewParams::GetDefaultFraming(width, height);
    params.viewInfo.ambient          = pxr::GfVec4f(0.1f, 0.1f, 0.1f, 0.0f);;
    params.colorspace                = pxr::HdxColorCorrectionTokens->sRGB;
    params.backgroundColor           = PXR_NS::GfVec4f(0.5f, 0.7f, 0.5f, 1.f);
    params.enablePresentation        = true;

    auto cameraPosition = camera->getCamera().GetTransform().ExtractTranslation();
    m_cameraLight.SetAmbient(PXR_NS::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
    m_cameraLight.SetPosition(PXR_NS::GfVec4f((float)cameraPosition[0], (float)cameraPosition[1], (float)cameraPosition[2], 1.f));
    params.viewInfo.lights = {m_cameraLight};

    m_sceneFramePass->Render();
}

std::string ViewportEngine::rendererName() const
{
    return hvt::HgiInstance::instance().defaultSceneRendererName();
}

std::string ViewportEngine::hgiName() const
{
    return hvt::HgiInstance::instance().hgi()->GetAPIName().GetString();
}

} // namespace HVW_NS