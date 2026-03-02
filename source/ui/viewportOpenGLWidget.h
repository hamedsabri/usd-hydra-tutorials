#pragma once

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>

namespace HVW_NS
{

/**
 * @brief OpenGL viewport widget.
 */
class ViewportOpenGLWidget
    : public QOpenGLWidget
    , public QOpenGLFunctions_4_5_Core
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the viewport widget.
     */
    ViewportOpenGLWidget(QWidget* parent = nullptr);

    /**
     * @brief Destroys the viewport widget and releases OpenGL resources.
     */
    virtual ~ViewportOpenGLWidget() = default;

protected:
    /**
     * @brief Initializes OpenGL state and rendering resources.
     */
    void initializeGL() override;

    /**
     * @brief Handles viewport resize events.
     *
     * @param w New viewport width in pixels.
     * @param h New viewport height in pixels.
     */
    void resizeGL(int w, int h) override;

    /**
     * @brief Renders the current frame.
     */
    void paintGL() override;

    /**
     * @brief Handles mouse wheel events for camera interaction.
     */
    void wheelEvent(QWheelEvent* event) override;

    /**
     * @brief Handles mouse press events.
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse move events for interactive camera control.
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse release events.
     */
    void mouseReleaseEvent(QMouseEvent* event) override;
};

} // namespace HydraViewport
