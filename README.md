# Minimal Viewport Implementation using Hydra 1.0 APIs

Hydra has a rich and sometimes intimidating vocabulary. Hydra itself is not a renderer and is rather a rendering framework/architecture that decouples "scene description" from actual "rendering execution". Because of this flexibility, the architecture is layered and highly modular which explains the number of classes and concepts. Although Hydra 1.0 will eventually be deprecated, understanding its core concepts remains valuable, as many of them are still relevant and carry over into the newer Hydra 2.0 API.

If you browse the API, you’ll quickly encounter many classes with prefixes like:

- **Hd** ( Hydra Core )  Scene representation, Render index, Task execution, Engine orchestration.
- **Hdx** ( Hydra Extensions ) Utility and higher-level helpers built on top of core Hydra.
- **UsdImaging** ( Hydra Translation Layer ) Bridges USD scene data into Hydra e.g Reading a USD stage, Translating USD prims into Hydra prims, Populating the HdRenderIndex.
- **Hgi** ( Hydra Graphics Interface ) Abstraction layer over the GPU backend (OpenGL, Metal, Directx12, Vulkan, etc.).

At first glance, names like:

- UsdImagingDelegate
- HdRenderIndex
- HdPluginRenderDelegateUniqueHandle
- HdRprimCollection
- HdxTaskController
- HdEngine
- HgiUniquePtr
- HdDriver
- HdxRenderTaskParams
- HdxSelectionTrackerSharedPtr
- ....

can feel cryptic and overwhelming. However, these prefixes are not arbitrary and they reflect Hydra’s architectural layers and responsibilities.
Once you understand Who produces scene data, Who renders the data, Who executes tasks, Who drives the GPU, etc... things become much easier to digest. 

## ViewportEngine
To implement the Hydra viewport using the Hydra 1.0 APIs, we will need to replace the relevant rendering code with UsdImagingGLEngine. For the sake of simplicity, all Hydra-related code will be implemented inside this class. However, if you want a more scalable and maintainable architectural design, you should consider separating the Hydra rendering logic into dedicated classes or modules.

To start things off, our ViewportEngine remains structurally the same as before. The public API does not change, we still expose an initialize() function for setup and a render() function that is called every frame.
Destructor is no longer defaulted since we need to clean up Hydra objects.

```cpp
class UsdCamera;
class ViewportEngine final
{
public:
    ViewportEngine() = default;
    ~ViewportEngine();

    void initialize(const PXR_NS::UsdStageRefPtr& stage);

    void render(const PXR_NS::UsdStageRefPtr& stage, 
                UsdCamera* camera,
                double width, double height);

    std::string rendererName() const;
    std::string hgiName() const;
};
```
If you recall, Hydra is conceptually built around three core components:

1. Scene Delegate – Provides scene data (prims, topology, transforms, materials) to Hydra.
2. Render Index – The central data structure that connects scene data to the renderer.
3. Render Delegate – The backend renderer responsible for drawing (e.g., Storm).

<img width="848" height="697" alt="hydra_image" src="https://github.com/user-attachments/assets/666577ac-6b77-410e-b343-fe452dd84ef1" />

We also don't just create these three objects directly, we also need a few supporting pieces to make the pipeline functional, such as a task controller and GPU interface.

A minimal setup looks like this:

```h
std::unique_ptr<PXR_NS::UsdImagingDelegate> m_sceneDelegatePtr;
std::unique_ptr<PXR_NS::HdRenderIndex>      m_renderIndexPtr; 
PXR_NS::HdPluginRenderDelegateUniqueHandle  m_renderDelegatePtr;
std::unique_ptr<PXR_NS::HdxTaskController>  m_taskControllerPtr;
PXR_NS::HdEngine                            m_engine;
PXR_NS::HgiUniquePtr                        m_hgiPtr;
PXR_NS::HdDriver                            m_hgiDriver;
```

**[UsdImagingDelegate](https://openusd.org/dev/api/class_usd_imaging_delegate.html)**
This is our scene delegate. It translates data from a UsdStage into Hydra primitives and feeds them into the render index.

**[HdRenderIndex](https://openusd.org/dev/api/class_hd_render_index.html)**
This is the central registry of all renderable data. It sits between the scene delegate and the render delegate.

**[HdPluginRenderDelegateUniqueHandle](https://openusd.org/release/api/class_hd_plugin_render_delegate_unique_handle.html)**
This loads and owns the actual renderer (for example, Storm).

**[HdxTaskController](https://openusd.org/release/api/hdx_page_front.html)**
This is the convenience layer that sets up common rendering tasks such as:

- Render setup
- Lighting
- Camera state

**[HdEngine](https://openusd.org/dev/api/class_hd_engine.html#details)**
Executes Hydra tasks every frame.

**Hgi + HdDriver**
The GPU abstraction layer (e.g., OpenGL backend). Hgi represents the graphics interface, and HdDriver connects it to Hydra.

Let’s now take a look at how to implement these components manually.

For the sake of brevity, this example intentionally avoids object validation and error checking. In production code, you should always validate pointers, plugin availability, stage validity, and backend support before proceeding.

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

	// create the Render Index
    m_renderIndexPtr.reset(PXR_NS::HdRenderIndex::New( m_renderDelegatePtr.Get(), {&m_hgiDriver} ));

    // create the scene delegate
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
}
```
Hydra objects have strict ownership and dependency relationships and therefore you need to make sure you destroy them in proper order.

```cpp
ViewportEngine::~ViewportEngine()
{
	// The order is important here
	m_taskControllerPtr  = nullptr;
	m_sceneDelegatePtr   = nullptr;
	m_renderIndexPtr     = nullptr;
	m_renderDelegatePtr  = nullptr;
}
```
```cpp
void ViewportEngine::render(const PXR_NS::UsdStageRefPtr& stage, 
                            UsdCamera* camera,
                            double width, double height)
{
    // Updating the camera state and Setting the view and projection matrices for the free camera
    camera->setAspectRatio(width / std::max(1.0, height));
    camera->updateTransform();
    auto cameraFrustum = camera->getCamera().GetFrustum();
    m_taskControllerPtr->SetFreeCameraMatrices(cameraFrustum.ComputeViewMatrix(), cameraFrustum.ComputeProjectionMatrix());

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
```
And that’s basically it! If you run the application now, you should be able to load a USD file and see it rendered in our Hydra-powered viewport. Note that we currently don’t have any scene lights, so everything will appear dark. Next, we’ll add a camera light to illuminate the scene.

```h
pxr::GlfSimpleLight                         m_cameraLight;
pxr::GlfSimpleLightingContextRefPtr         m_pLightingContext;
```
```cpp
void ViewportEngine::initialize(const PXR_NS::UsdStageRefPtr& stage)
{
	....
	// Create a simple light context
	m_pLightingContext = PXR_NS::GlfSimpleLightingContext::New();
}
```
```cpp
void ViewportEngine::render(const PXR_NS::UsdStageRefPtr& stage, 
                            UsdCamera* camera,
                            double width, double height)
{
	....

    // Camera Light
    auto cameraPosition = camera->getCamera().GetTransform().ExtractTranslation();
    m_cameraLight.SetAmbient(PXR_NS::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
    m_cameraLight.SetPosition(PXR_NS::GfVec4f((float)cameraPosition[0], (float)cameraPosition[1], (float)cameraPosition[2], 1.f));

    m_pLightingContext->SetLights({m_cameraLight});
    m_pLightingContext->SetSceneAmbient(PXR_NS::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
    m_pLightingContext->SetUseLighting(true);

    m_taskControllerPtr->SetLightingState(m_pLightingContext);
}
```
![demo_tutorial4](https://github.com/user-attachments/assets/3b6c58f9-41b3-492b-8981-aee66bb925e1)


Lastly, let’s implement the functions to retrieve the renderer and HGI names for debugging purposes:
```cpp
std::string ViewportEngine::rendererName() const
{
    return "HdStormRendererPlugin";
}

std::string ViewportEngine::hgiName() const
{
    return m_hgiPtr->GetAPIName().GetString();
}
```
That’s a wrap for this short tutorial! I hope you found it useful and that it gives you a good starting point for building your own Hydra-powered viewport.
