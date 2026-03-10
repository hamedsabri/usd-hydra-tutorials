#include "viewportEngine.h"
#include "camera/usdCamera.h"
#include <pxr/usd/usd/prim.h>

PXR_NAMESPACE_USING_DIRECTIVE

namespace HVW_NS
{

void ViewportEngine::initialize(const PXR_NS::UsdStageRefPtr& stage)
{
    [[maybe_unused]] PXR_NS::SdfPathVector excludedPaths;
    m_engine = std::make_unique<PXR_NS::UsdImagingGLEngine>(stage->GetPseudoRoot().GetPath(), excludedPaths);

    // camera light
    m_cameraLight.SetAmbient({ 0.1, 0.1, 0.1, 1.0 });
    m_cameraLight.SetDiffuse({ 1.0, 1.0, 1.0, 1.f });
    m_cameraLight.SetSpecular({ 0.1, 0.1, 0.1, 1.f });
    m_cameraLight.SetPosition({ 10, 10, 10, 1.0 });
    m_lights.push_back(m_cameraLight);
}

void ViewportEngine::render(const PXR_NS::UsdStageRefPtr& stage, 
                            UsdCamera* camera,
                            double width, double height)
{

    camera->setAspectRatio(width / std::max(1.0, height));
    camera->updateTransform();

    // update camera light position
    GfVec3d cameraPos = camera->getCamera().GetFrustum().GetPosition();
    m_cameraLight.SetPosition(GfVec4f(cameraPos[0], cameraPos[1], cameraPos[2], 1.0));
    m_cameraLight.SetTransform(camera->getCamera().GetTransform());

    // update the light state
    m_lights[0] = m_cameraLight;

    PXR_NS::GlfSimpleMaterial defaultMaterial;
    defaultMaterial.SetDiffuse(GfVec4f(0.8f, 0.8f, 0.8f, 1.0f)); 
    defaultMaterial.SetSpecular(GfVec4f(0.0f, 0.0f, 0.0f, 1.0f));
    defaultMaterial.SetEmission(GfVec4f(0.0f));
    defaultMaterial.SetShininess(0.0f);

    GfVec4f defaultAmbient(0.2f, 0.2f, 0.2f, 1.0f);

    m_engine->SetLightingState(m_lights, defaultMaterial, defaultAmbient);

    m_engine->SetCameraState(camera->getViewMatrix(), camera->getProjectionMatrix());

    m_engine->SetRenderViewport(PXR_NS::GfVec4d(0, 0, width, height));

    m_renderParams.cullStyle = UsdImagingGLCullStyle::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED;
    m_renderParams.clearColor = GfVec4f(0.2f, 0.2f, 0.2f, 1.0f);
    m_renderParams.forceRefresh = false;
    m_renderParams.enableLighting = true;
    m_renderParams.enableSampleAlphaToCoverage = false;
    m_renderParams.enableSceneMaterials = true;
    m_renderParams.enableSceneLights = true;
    m_renderParams.flipFrontFacing = true;
    m_renderParams.gammaCorrectColors = true;
    m_renderParams.highlight = true;
    m_renderParams.showGuides = true;
    m_renderParams.showProxy = true;
    m_renderParams.showRender = true;
    m_renderParams.complexity = 1.0;

    m_engine->Render(stage->GetPseudoRoot(), m_renderParams);
}

std::string ViewportEngine::rendererName() const
{
    return m_engine->GetRendererDisplayName(m_engine->GetCurrentRendererId());
}

std::string ViewportEngine::hgiName() const
{
    return m_engine->GetHgi()->GetAPIName().GetString();
}

} // namespace HVW_NS
