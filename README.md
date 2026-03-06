# UsdImagingGLEngine

UsdImagingGLEngine serves as the main high-level entry point / convenience API for rendering USD scenes in an OpenGL context. This is a convenience layer that wraps Hydra engine (HdEngine) to make rendering a USD stage straightforward in an existing OpenGL context.

With UsdImagingGLEngine, you can easily render a USD stage, set up render parameters using `UsdImagingGLRenderParams`, and call simple functions like `Render()`, `TestIntersection()` without dealing with the underlying Hydra components such as the scene delegate / index , render index, or tasks.

- [UsdImagingGLEngine Class Reference](https://openusd.org/24.08/api/class_usd_imaging_g_l_engine.html)
- [UsdImagingGLRenderParams Class Reference](https://openusd.org/24.08/api/class_usd_imaging_g_l_render_params.html#details)

Here, we are going to implement a bare-minimum code required to render a USD stage in a viewport powered by UsdImagingGLEngine. For the sake of simplicity, I won’t be covering Qt or CMake build configuration, as this material assumes you’re already familiar with both.

# How to Build
In order to build the project in this branch, you just need to provide additional path to openusd install directory:

```
cmake -GNinja -DCMAKE_MAKE_PROGRAM="<path_to_ninja_exe>" -DQT_LOCATION="<path_to_qt_install_directory>" -DOPENUSD_LOCATION="<path_to_openusd_install_directory>" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="<install_path>" ..
```

# Creating a New Stage and Opening a Stage from File
To support creating and loading USD stages, we introduce a small HVW_NS::UsdDocument class. This class owns the currently `active UsdStage` and exposes a simple interface for either creating a new in-memory stage or opening an existing stage from disk.

## HVW_NS::UsdDocument
This class provides two main functions:

1. `createNewStageInMemory`: creates a fresh stage in memory
2. `openStage`: loads a stage from a file

In both cases, the active stage is stored internally and a stageOpened signal is emitted so the rest of the application can react to the change.

# ViewportEngine
ViewportEngine is simply a wrapper around OpenUSD’s UsdImagingGLEngine that initializes and manages a Hydra-based OpenGL renderer for drawing a UsdStage with a given camera and render settings.

**SetCameraState**: to define the camera's view and projection matrices
**SetRenderViewport**: to define the render area
**Render**: to execute the Hydra render pass

`UsdImagingGLRenderParams` controls how the scene is rendered. It basically defines rendering behavior such as:

enableLighting – Toggles scene lighting on or off.
drawMode – Controls how geometry is drawn (e.g., shaded, wireframe).
showGuides / showProxy / showRender – Determines which purpose types are visible.
enableSceneMaterials – Enables or disables material shading.
cullStyle – Controls backface/frontface culling behavior.
clearColor – Defines the background color.

```cpp
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
```
# Camera
UsdCamera is a simple interactive camera controller built on `pxr::GfCamera` that provides orbit, pan, dolly, zoom, framing, and clipping controls for navigating a USD scene within a viewport.

`getViewMatrix()` returns the camera’s view matrix
`getProjectionMatrix()` returns the projection matrix
`updateTransform()` recalculates and applies the camera’s transform

```cpp
void ViewportEngine::render(const PXR_NS::UsdStageRefPtr& stage, 
                            UsdCamera* camera,
                            double width, double height)
{

    camera->setAspectRatio(width / std::max(1.0, height));
    camera->updateTransform();
    m_engine->SetCameraState(camera->getViewMatrix(), camera->getProjectionMatrix());

    ...
}
```

# ViewportOpenGLWidget
ViewportOpenGLWidget is an OpenGL viewport that integrates UsdImagingGLEngine into a QOpenGLWidget, managing the GL context, user interaction, camera control, and delegating actual scene drawing to ViewportEngine.

```cpp
void ViewportOpenGLWidget::initialize()
{
    if (!m_usdDocument->getCurrentStage()) {
        return;
    }

    m_camera = std::make_unique<UsdCamera>(m_usdDocument->getCurrentStage());
    m_viewportEngine = std::make_unique<ViewportEngine>();
    m_viewportEngine->initialize(m_usdDocument->getCurrentStage());

    qDebug() << "[Viewport] Created.";
    qDebug() << "[Viewport]"
             << QStringLiteral("Renderer: %1").arg(QString::fromStdString(m_viewportEngine->rendererName()))
             << QStringLiteral("Hgi: %1").arg(QString::fromStdString(m_viewportEngine->hgiName()));
}

void ViewportOpenGLWidget::resizeGL(int w, int h)
{
    m_width = w * devicePixelRatio();
    m_height = h * devicePixelRatio();

    glViewport(0, 0, w, h);
}

void ViewportOpenGLWidget::paintGL()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    if (!m_usdDocument->getCurrentStage()) {
        return;
    }

    m_viewportEngine->render(m_usdDocument->getCurrentStage(), m_camera.get(), m_width, m_height);
}
```
This covers the fundamental building blocks required to implement a minimal Hydra-based viewport using UsdImagingGLEngine.
From here, you can explore more advanced topics such as selection, picking, custom render passes (e.g., using GlfDrawTargetRefPtr), and renderer AOVs by studying the following open-source projects:

- [tinkerusd](https://github.com/hamedsabri/TinkerUsd)
- [usdtweak](https://github.com/cpichard/usdtweak)

# Executable Demo
![demo_tutorial3](https://github.com/user-attachments/assets/6b27d477-4a08-4796-b7fd-e1bafd270bd9)

