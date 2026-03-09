# What is Hydra?

Hydra is the rendering architecture in OpenUSD that decouples scene description from rendering, allowing the same USD scene graph data to be efficiently rendered using different renderers (real-time, ray-traced, etc.).

Hydra come in three main parts:

1. `Scene Delegate`( UsdImagingDelegate ) / Scene Index ( UsdImagingStageSceneIndex )

    This part provides the scene information either via:

   - `UsdImaging` (the standard USD integration): This takes a USD scene (for example a UsdStage containing geometry, materials, lights, cameras, animations, instancing, and more) and efficiently translates or marshals that data into a format Hydra can use for rendering. It supports both the legacy Hydra 1.0 delegate system and the modern Hydra 2.0 scene index system.

       Note: UsdImaging's scene delegate mode is now deprecated in favor of scene index mode. Accordingly, `USDIMAGINGGL_ENGINE_ENABLE_SCENE_INDEX defaults` to 1, and a single-shot deprecation notice will be issued if this is overridden back to 0. The deprecation warning can be suppressed by overriding `USDIMAGINGGL_ENGINE_ENABLE_SCENE_INDEX_DEPRECATION_WARNING` to 0.

    Hydra Primitives types: Hydra does not mirror USD’s type system directly.Instead, it categorizes data by rendering role:
      1. Rprim (Renderable Primitive)
      2. Sprim (State Primitive)
      3. Bprim (Buffer Primitive)
        
    e.g
    ```
    UsdGeomMesh   →  HdMesh (Rprim)
    UsdGeomCamera → HdCamera (Sprim)
    UsdUVTexture  → HdTexture (Bprim)
    ``` 

    - `Custom implementations`: Developers can provide their own Scene Delegate (HdSceneDelegate) or Scene Index(es) to feed data.

1. `Render Index` (HdRenderIndex)

     The Hydra render index is a **flattened representation of the client scene graph**. It pulls data from the scene delegate/index, syncs updates, and serves as the interface between scene data and the renderer.
The Render Index uses an internal change tracker (HdChangeTracker) to mark only the dirty/affected parts.During the "sync" phase (e.g., when HdRenderIndex::Sync() is called), it efficiently pulls only the changed data from the scene delegate/index.

     [HdRenderIndex](https://openusd.org/dev/api/class_hd_render_index.html#details)

2. `Render Delegate`: Renderer-specific plugin that implements creation/sync of primitives (meshes, materials, etc.) and rendering execution to create the final image. (e.g HdStorm, HdArlnold, HdPrman, etc...)

Here are the diagram images that are often illustrate these three steps:

**Screenshot taken from Pixar's Siggraph presentaion**
<img width="848" height="697" alt="hydra_image" src="https://github.com/user-attachments/assets/c3d82721-f7f9-46c5-99df-51470d5b1095" />


**Screenshot taken from Hydra - Nvidia Learn OpenUsd series**
<img width="761" height="368" alt="hydra_image2" src="https://github.com/user-attachments/assets/a39c5099-a72e-4ba7-9834-f180b47df9b1" />

**Screenshot taken from Autodesk's Adding Vulkan to Pixar's Hydra Storm Renderer presentaion**
<img width="1111" height="564" alt="hydra_image3" src="https://github.com/user-attachments/assets/cb7e4b37-a501-4b86-9bd3-b816462e1791" />

**Screenshot taken from Universal Scene Description (OpenUSD): Dynamic Data Science Pipelines with Hydra**
<img width="1383" height="770" alt="Screenshot 2026-03-09 041931" src="https://github.com/user-attachments/assets/d3f09123-8664-4ac8-a635-4fad5b94ffbd" />

## Hydra 1.0 vs Hydra 2.0

Hydra 1.0 and Hydra 2.0 refer to two generations of the scene data handling architecure in OpenUSD's Hydra rendering architecture. The core goal remains the same which is decoupling scene description from rendering but Hydra 2.0 introduced gradually which brings major improvements in flexibility, extensibility, and support for procedural/runtime transformations. You can think of Hydra 2.0 as a `Composable SceneIndex pipeline` where scene is represented as a `chain of HdSceneIndex layers`. Each layer can Filter, Modify, and Procedurally generate data.

# What is Storm (HdStorm)?

Storm is Hydra's `real-time rasterizing render delegate`. Originally built on `OpenGL`, Storm later adopted the `Hydra Graphics Interface` (Hgi) an abstraction layer for modern low-level graphics APIs to enable broader support. A few years ago, the "HgiMetal" backend was added to leverage Apple's Metal API, significantly boosting performance on macOS and iOS. From Collaborative effort from Autodesk, Pixar, and Adobe the "HgiVulkan" backend was also introduced in `OpenUSD 24.08`.

## HGI ( Hydra Graphic Interface )

Hgi is an abstraction layer within OpenUSD's Hydra rendering framework to let storm renderercommunicate with different modern low-level graphics APIs ( e.g Vulkan, Dirext12, OpenGL, Metal ) without being tied to any single one.

- **HgiGL** — For OpenGL (the original/default backend).
- **HgiMetal** — For Apple's Metal API (added to support macOS and iOS efficiently).
- **HgiVulkan** — Experimental support for Khronos Vulkan (added in OpenUSD 24.08; collaborative work from Pixar, Autodesk, Adobe).

- [Hgi Class Reference](https://openusd.org/release/api/class_hgi.html#details)

# Revelant Api Documentations
- [**Hd** : The Hydra Framework](https://openusd.org/docs/api/hd_page_front.html)
- [**HdSt** : Rendering functionality for HdStorm](https://openusd.org/docs/api/hd_st_page_front.html)
- [**HdStorm** : Real-time Hydra renderer plugin](https://openusd.org/docs/api/hd_storm_page_front.html)
- [**Hdx** : Hydra extensions](https://openusd.org/docs/api/hdx_page_front.html)
