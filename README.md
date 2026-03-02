# Minimal Viewport Implementation using Hydra 2.0 APIs

Switching to the new Hydra 2.0 architecture (also known as the Scene Index–based pipeline) is surprisingly straightforward. In essence, instead of creating a classic scene delegate as in Hydra 1.0, we construct a chain of Scene Indices and insert the resulting index into the render index. We still have a render delegate, a render index, and a task controller but the way scene data flows into Hydra is now driven through Scene Indices rather than a monolithic delegate.

## USD Stage Scene Index

**UsdImagingCreateSceneIndices** initializes and assembles the Scene Index pipeline based on the supplied stage configuration. 

```h
#include <pxr/usdImaging/usdImaging/stageSceneIndex.h>

PXR_NS::UsdImagingStageSceneIndexRefPtr m_stageSceneIndex;
```
The **stageSceneIndex** acts as the bridge between the USD stage and Hydra, while the **finalSceneIndex** represents the fully composed Scene Index chain that Hydra will consume.

At this point, Hydra is aware of the scene graph and can begin translating USD prims into Hydra primitives internally. From there, we create an HdxTaskController, which is responsible for setting up and managing the render tasks (render task, selection task, lighting task, etc.) required to drive the frame.

```cpp
PXR_NS::UsdImagingCreateSceneIndicesInfo info;
info.displayUnloadedPrimsWithBounds = false;
info.stage                          = stage;
const PXR_NS::UsdImagingSceneIndices sceneIndices = UsdImagingCreateSceneIndices(info);

// scene delegate
m_stageSceneIndex = sceneIndices.stageSceneIndex;
m_stageSceneIndex->SetStage(stage);
m_stageSceneIndex->SetTime(UsdTimeCode::Default());

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
    m_renderIndexPtr     = nullptr;
    m_renderDelegatePtr  = nullptr;
}
```
