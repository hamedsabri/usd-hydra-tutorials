#pragma once

#include <pxr/usd/usd/stage.h>

#include <hvt/engine/framePass.h>

namespace HVW_NS
{

class UsdCamera;
class ViewportEngine final
{
public:
    ViewportEngine();
    ~ViewportEngine();

    void initialize(PXR_NS::UsdStageRefPtr stage);
    void render(UsdCamera* camera, double width, double height);

    std::string rendererName() const;
    std::string hgiName() const;

private:
    hvt::RenderIndexProxyPtr             m_renderIndex;
    hvt::FramePassPtr                    m_sceneFramePass;
    
    pxr::GlfSimpleLight                  m_cameraLight;
    pxr::GlfSimpleLightingContextRefPtr  m_pLightingContext;
};

} // namespace HVW_NS