#pragma once

#include <QOpenGLFunctions_4_5_Core>

namespace HVW_NS
{

class DrawTarget final : public QOpenGLFunctions_4_5_Core
{
public:
    DrawTarget();
    virtual ~DrawTarget();

    void initialize();
    void resize(int width, int height);
    void draw(uint32_t textureId);

private:
    void initializeQuad();
    void createShaders();

private:
    GLuint m_vao{0};
    GLuint m_vbo{0};
    GLuint m_shaderProgram{0};
};

} // namespace HVW_NS