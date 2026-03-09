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
Finally, during destruction, we need to release renderIndexPtr.

```cpp
ViewportEngine::~ViewportEngine()
{
    // The order is important here
    m_taskControllerPtr  = nullptr;
    m_renderIndexPtr     = nullptr;
    m_renderDelegatePtr  = nullptr;
}
```
