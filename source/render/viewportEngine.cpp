#include "viewportEngine.h"
#include "camera/usdCamera.h"
#include <pxr/usd/usd/prim.h>

PXR_NAMESPACE_USING_DIRECTIVE

namespace HVW_NS
{

void ViewportEngine::initialize(const PXR_NS::UsdStageRefPtr& stage)
{
    PXR_NS::SdfPathVector excludedPaths;
    m_engine = std::make_unique<PXR_NS::UsdImagingGLEngine>(stage->GetPseudoRoot().GetPath(), excludedPaths);
}

void ViewportEngine::render(const PXR_NS::UsdStageRefPtr& stage, 
                            UsdCamera* camera,
                            double width, double height)
{

    camera->setAspectRatio(width / std::max(1.0, height));
    camera->updateTransform();

    m_engine->SetCameraState(camera->getViewMatrix(), camera->getProjectionMatrix());

    m_engine->SetRenderViewport(PXR_NS::GfVec4d(0, 0, width, height));

    m_renderParams.cullStyle = UsdImagingGLCullStyle::CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED;
    m_renderParams.clearColor = GfVec4f(0.2f, 0.2f, 0.2f, 1.0f);
    m_renderParams.forceRefresh = false;
    m_renderParams.enableLighting = false;
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
