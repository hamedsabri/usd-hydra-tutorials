#include "viewportOpenGLWidget.h"
#include "model/usdDocument.h"

#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QWheelEvent>

#define SAMPLE_AMOUNT 8

namespace HVW_NS
{

ViewportOpenGLWidget::ViewportOpenGLWidget(UsdDocument* document, QWidget* parent)
    : QOpenGLWidget(parent)
    , m_usdDocument(document)
{
    QSurfaceFormat format;
    format.setSamples(SAMPLE_AMOUNT);
    setFormat(format);

    connect(m_usdDocument, &UsdDocument::stageOpened, this, &ViewportOpenGLWidget::onStageOpened);
}

void ViewportOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
}

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

void ViewportOpenGLWidget::onStageOpened(const QString& filePath)
{
    initialize();

    update();
}

void ViewportOpenGLWidget::wheelEvent(QWheelEvent* event)
{
    double angleDelta = static_cast<double>(event->angleDelta().y()) / 1000.0;
    m_camera->adjustDistance(1.0 - std::max(-0.5, std::min(0.5, angleDelta)));

    update();
}

void ViewportOpenGLWidget::mousePressEvent(QMouseEvent* event)
{
    m_lastMousePosition = event->pos() * devicePixelRatio();

    if (event->modifiers() & (Qt::AltModifier | Qt::MetaModifier))
    {
        if (event->button() == Qt::LeftButton)
        {
            m_camera->setDragMode(UsdCamera::DragMode::DOLLY);
        }
        else if (event->button() == Qt::RightButton)
        {
            m_camera->setDragMode(UsdCamera::DragMode::ZOOM);
        }
        else if (event->button() == Qt::MiddleButton)
        {
            m_camera->setDragMode(UsdCamera::DragMode::PAN);
        }
    }
}

void ViewportOpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint currentMousePosition = event->pos() * devicePixelRatio();

    QPoint delta = currentMousePosition - m_lastMousePosition;
    if (delta.x() == 0 && delta.y() == 0)
    {
        return;
    }

    if (m_camera->getDragMode() == UsdCamera::DragMode::DOLLY)
    {
        m_camera->dolly(0.25 * delta.x(), 0.25 * delta.y());
    }
    else if (m_camera->getDragMode() == UsdCamera::DragMode::PAN)
    {
        auto pixelsToWorld = m_camera->computePixelsToWorldFactor(m_height);
        m_camera->pan(-delta.x() * pixelsToWorld, delta.y() * pixelsToWorld);
    }
    else if (m_camera->getDragMode() == UsdCamera::DragMode::ZOOM)
    {
        auto zoomDelta = -.002 * (delta.x() + delta.y());
        m_camera->zoom(zoomDelta);
    }

    m_lastMousePosition = event->pos() * devicePixelRatio();

    update();
}

void ViewportOpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    m_camera->setDragMode(UsdCamera::DragMode::NONE);
}

} // namespace HVW_NS