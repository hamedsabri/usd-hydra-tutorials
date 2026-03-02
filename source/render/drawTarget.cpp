#include "drawTarget.h"


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
        vec2 rOffset = vec2(0.02, 0.0);
        vec2 bOffset = vec2(-0.02, 0.0);

        float r = texture(screenTexture, TexCoord + rOffset).r;
        float g = texture(screenTexture, TexCoord).g;
        float b = texture(screenTexture, TexCoord + bOffset).b;

        FragColor = vec4(r, g, b, 1.0);
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
    glUniform1i(glGetUniformLocation(m_shaderProgram, "AOV_COLOR_TEXTURE"), 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

} // namespace HVW_NS