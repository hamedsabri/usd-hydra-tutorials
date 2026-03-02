#pragma once

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>

#include <memory>

namespace HVW_NS
{

class UsdCamera;
class ViewportEngine final
{
    using Ptr = std::unique_ptr<PXR_NS::UsdImagingGLEngine>;
public:
    ViewportEngine() = default;
    ~ViewportEngine() = default;

    void initialize(const PXR_NS::UsdStageRefPtr& stage);

    void render(const PXR_NS::UsdStageRefPtr& stage, 
                UsdCamera* camera,
                double width, double height);

    std::string rendererName() const;
    std::string hgiName() const;

private:
    Ptr                              m_engine;
    PXR_NS::UsdImagingGLRenderParams m_renderParams;
};

} // namespace HVW_NS