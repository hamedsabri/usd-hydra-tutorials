# Minimal Viewport Implementation using Autodesk Hydra Viewport Toolbox

The [**Hydra Viewport Toolbox (HVT)**](https://github.com/Autodesk/hydra-viewport-toolbox) is a collection of utilities designed to simplify integrating OpenUSD Hydra into an application's graphics viewport. HVT currently includes the following features but it is being expanded to include even more.

- Layering of Hydra render delegate output, optionally from different render delegates ("passes").
- Management of multiple viewports.
- Hydra task management, supporting application-defined lists of tasks.
- Management of data commonly needed for tasks, e.g. render buffers and lighting.
- Tasks for features commonly needed for viewports, e.g. antialiasing and ambient occlusion.
- User interaction for common operations, e.g. selection and camera manipulation.

## How to Build HVT
For build instructions, refer to the [**README**](https://github.com/Autodesk/hydra-viewport-toolbox/blob/main/README.md) page of the Hydra Viewport Toolbox repository. After building and installing the Hydra Viewport Toolbox, the installation directory should have a structure similar to the following:

The include/hvt directory contains the public headers you will use in your application. The lib folder provides the compiled libraries and CMake configuration files required for integration, and bin contains runtime binaries.

```console
├───bin
├───include
│   └───hvt
│       ├───dataSource
│       ├───engine
│       ├───geometry
│       ├───resources
│       │   ├───gizmos
│       │   └───shaders
│       ├───sceneIndex
│       └───tasks
└───lib
    └───cmake
        └───hvt
```

## CMake configuration
The Hydra Viewport Toolbox installation includes ready-to-use CMake configuration files. After installation, you should find the following files under `lib/cmake/hvt`:

```cmake
hvtConfig.cmake
hvtConfigVersion.cmake
hvtTargets-relwithdebinfo.cmake
hvtTargets.cmake
```
## How to Consume HVT
Once the Hydra Viewport Toolbox is built and installed, integrating it into your CMake project is straightforward using exported CMake package configuration.

First, point CMake to the installation directory:

```cmake
set(HVT_LOCATION "<your_path_to_hydra_toolbox_install_directory>" CACHE PATH "Hydra Toolbox Viewport Install Location")

list(APPEND CMAKE_PREFIX_PATH
    ....
    ${HVT_LOCATION}
)

# HVT
find_package(hvt CONFIG REQUIRED)
if(hvt_FOUND)
    message(STATUS "Using hvt ${hvt_VERSION}")
endif()
```
After the package is found, simply link against the exported target. There is no need to manually add include directories or library paths. The **hvt::hvt** imported target already carries its include paths, compile definitions, and link dependencies.
```cmake
target_link_libraries(${TARGET_NAME}
    PRIVATE
        hvt::hvt
)
```
## Implementing ViewportEngine using HVT

This example demonstrates the bare-minimum setup required to render a USD stage using the Autodesk Hydra Viewport Toolbox. The goal is to strip the ViewportEngine class down to its essential components: creating an Hgi backend, building a Hydra render index, inserting a USD Scene Index, and executing a single FramePass.

ViewportEngine now owns a RenderIndex proxy and a FramePass.

```h
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
```
At initialization time, the engine creates an Hgi instance using the OpenGL backend.

```cpp
ViewportEngine::ViewportEngine()
{
    hvt::HgiInstance::instance().create(HgiTokens->OpenGL);
}
```
Since Hgi represents the GPU abstraction layer used by the Autodesk Hydra Viewport Toolbox, destroying it ensures that all GPU resources, driver objects, and backend state are properly released before application shutdown.

```cpp
ViewportEngine::~ViewportEngine()
{
    // order is important
    m_sceneFramePass = nullptr;
    m_renderIndex = nullptr;
    hvt::HgiInstance::instance().destroy();
}
```
Once the Hgi driver is available, we construct a renderer through the Viewport Toolbox helper API. Internally, this sets up a Hydra render index and configures the default scene renderer plugin.

```cpp
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
```
Rendering itself is performed through a FramePass. The FramePass encapsulates the execution of Hydra’s render tasks and ultimately triggers HdEngine::Execute() under the hood.

```cpp
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
```
And that’s it. With just a renderer, a Scene Index, and a single FramePass, we now have a fully minimal Hydra 2.0 viewport.
