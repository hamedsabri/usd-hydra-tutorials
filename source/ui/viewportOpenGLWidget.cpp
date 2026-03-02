#include "viewportOpenGLWidget.h"

#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QWheelEvent>

#define SAMPLE_AMOUNT 8

namespace HVW_NS
{

ViewportOpenGLWidget::ViewportOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setSamples(SAMPLE_AMOUNT);
    setFormat(format);
}

void ViewportOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

void ViewportOpenGLWidget::resizeGL(int w, int h)
{
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
}

void ViewportOpenGLWidget::wheelEvent(QWheelEvent* event)
{
    update();
}

void ViewportOpenGLWidget::mousePressEvent(QMouseEvent* event)
{
}

void ViewportOpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    update();
}

void ViewportOpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
}

} // namespace HVW_NS