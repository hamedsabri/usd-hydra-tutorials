# Custom Presentation 
When presentation flag is enabled (this by default), HdxTaskController automatically includes an HdxPresentTask in the render task chain. This task handles the final compositing of Hydra's rendered AOVs (primarily the color buffer, and optionally depth) onto the currently bound framebuffer which is typically your application's default OpenGL framebuffer (e.g., the Qt window's back buffer). It then performs any necessary format conversion, gamma correction, or simple blitting/compositing, so the rendered result appears on screen without additional work from your application.
When presenation is didabled, Hydra skips the HdxPresentTask entirely. This means, No automatic compositing or blitting occurs from Hydra to your framebuffer and you become fully responsible for presenting the rendered result yourself. Common use cases could be Offscreen rendering, Custom compositing, etc...

## Retrieving the Color AOV Texture from Hydra

Let's start disabling the default behaviour presenation and when you run and load a USD stage, you shouldn't see anything drawn anymore.
```cpp
 m_taskControllerPtr->SetEnablePresentation(false);
```
We retrieve the OpenGL texture ID for the rendered color AOV by querying the task context from HdEngine, extracting the HgiTextureHandle, casting it to the OpenGL-specific HgiGLTexture, and then calling GetTextureId().
```cpp
uint32_t ViewportEngine::getColorAovTextureId() const
{
    PXR_NS::VtValue aov;
    //  ask the engine for the data associated with the "color" AOV.
    //  This is stored in the task context after rendering tasks execute.
    if (!m_engine.GetTaskContextData(PXR_NS::HdAovTokens->color, &aov)) {
        return 0;  // No color AOV data available → render probably didn't produce it or failed
    }

    // check if the retrieved value actually holds an HgiTextureHandle
    if (!aov.IsHolding<PXR_NS::HgiTextureHandle>()) {
        return 0;
    }

    // check if the texture handle is valid
    PXR_NS::HgiTextureHandle texHandle = aov.Get<PXR_NS::HgiTextureHandle>();
    if (!texHandle) {
        return 0;
    }

    // since we're using the OpenGL backend (Storm with HgiGL), cast to the GL-specific subclass.
    PXR_NS::HgiGLTexture* glTex = dynamic_cast<PXR_NS::HgiGLTexture*>(texHandle.Get());
    if (!glTex) {
        return 0;
    }

    // finally retrieve the OpenGL texture name/ID.
    return glTex->GetTextureId();
}
```
### Draw target
Next, we create a simple draw target for presenting Hydra’s rendered output to the screen. Since the color AOV is produced as an OpenGL texture, this class simply draws that texture onto a full-screen quad.

```cpp
namespace
{
const char* vertexShaderSrc = R"(#version 450 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main()
    {
        TexCoord = aTexCoord;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
    )";

const char* fragmentShaderSrc = R"(#version 450 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTexture;

        void main()
        {
            vec4 linearColor = texture(screenTexture, TexCoord);
            FragColor = vec4(linearColor.rgb, linearColor.a);
        }
    )";
} // namespace

namespace HVW_NS
{

DrawTarget::DrawTarget()
{
    initializeOpenGLFunctions();
}

DrawTarget::~DrawTarget()
{
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

void DrawTarget::initialize()
{
    createShaders();
    initializeQuad();
}

void DrawTarget::createShaders()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vs);

    GLint success = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(vs, 1024, nullptr, infoLog);
        qWarning() << "Vertex shader compilation failed:\n" << infoLog;
        glDeleteShader(vs);
        return;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(fs, 1024, nullptr, infoLog);
        qWarning() << "Fragment shader compilation failed:\n" << infoLog;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return;
    }

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void DrawTarget::initializeQuad()
{
    static const float quadVertices[] = {
        // pos      // uv
        -1.f,  1.f, 0.f, 1.f,
         1.f,  1.f, 1.f, 1.f,
         1.f, -1.f, 1.f, 0.f,
        -1.f, -1.f, 0.f, 0.f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    glBindVertexArray(0);
}

void DrawTarget::draw(uint32_t textureId)
{
    if (textureId == 0)
        return;

    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "screenTexture"), 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

} // namespace HVW_NS
```
Inside ViewportOpenGLWidget, we now create a DrawTarget object to render Hydra’s color output texture onto the screen.

```h
std::unique_ptr<DrawTarget> m_drawTarget;
```
To ensure all OpenGL resources are created safely, we move the initialization of the `DrawTarget` object into the `initializeGL()` method as this the only place where Qt guarantees an active and current OpenGL context. The `paintGL()` function now simply renders the USD scene via the viewport engine, and uses our `DrawTarget` to blit the resulting color texture (AOV) to the screen as a full-screen textured quad.

```cpp
void ViewportOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    m_drawTarget = std::make_unique<DrawTarget>();
    m_drawTarget->initialize();
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
    m_drawTarget->draw(m_viewportEngine->getColorAovTextureId());
}
```
Lets do a very simple post-processing effect by offseting Red and Blue channels slighlty to create a glitchy look.

```glsl
const char* fragmentShaderSrc = R"(#version 450 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D screenTexture;

    void main()
    {
        vec2 rOffset = vec2(0.02, 0.0);
        vec2 bOffset = vec2(-0.02, 0.0);

        float r = texture(screenTexture, TexCoord + rOffset).r;
        float g = texture(screenTexture, TexCoord).g;
        float b = texture(screenTexture, TexCoord + bOffset).b;

        FragColor = vec4(r, g, b, 1.0);
    }
)";
```
<img width="1285" height="998" alt="hydra_image4" src="https://github.com/user-attachments/assets/2d27df93-5534-44e1-bbac-c636ebda8ae2" />

