#pragma once

#include <pxr/usd/usd/stage.h>

#include "pxr/imaging/hdx/taskController.h"
#include "pxr/usdImaging/usdImagingGL/engine.h"
#include <pxr/imaging/hd/pluginRenderDelegateUniqueHandle.h>
#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/imaging/hgiGL/hgi.h>
#include <pxr/usdImaging/usdImaging/delegate.h>

#include <memory>

namespace HVW_NS
{

class UsdCamera;
class ViewportEngine final
{
    using Ptr = std::unique_ptr<PXR_NS::UsdImagingGLEngine>;

public:
    ViewportEngine() = default;
    ~ViewportEngine();

    void initialize(const PXR_NS::UsdStageRefPtr& stage);

    void render(const PXR_NS::UsdStageRefPtr& stage, 
                UsdCamera* camera,
                double width, double height);

    uint32_t getColorAovTextureId() const;

    std::string rendererName() const;
    std::string hgiName() const;

private:

    std::unique_ptr<PXR_NS::UsdImagingDelegate> m_sceneDelegatePtr;
    
    std::unique_ptr<PXR_NS::HdRenderIndex>      m_renderIndexPtr; 
    
    PXR_NS::HdPluginRenderDelegateUniqueHandle  m_renderDelegatePtr;
    
    std::unique_ptr<PXR_NS::HdxTaskController>  m_taskControllerPtr;
    
    PXR_NS::HdEngine                            m_engine;
    
    PXR_NS::HgiUniquePtr                        m_hgiPtr;
    PXR_NS::HdDriver                            m_hgiDriver;

    pxr::GlfSimpleLight                         m_cameraLight;
    pxr::GlfSimpleLightingContextRefPtr         m_pLightingContext;
};

} // namespace HVW_NS