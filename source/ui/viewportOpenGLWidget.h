#pragma once

#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLWidget>

#include "camera/usdCamera.h"
#include "render/viewportEngine.h"

namespace HVW_NS
{

class UsdDocument;

class ViewportOpenGLWidget
    : public QOpenGLWidget
    , public QOpenGLFunctions_4_5_Core
{
    Q_OBJECT
public:
    ViewportOpenGLWidget(UsdDocument* document, QWidget* parent = nullptr);
    virtual ~ViewportOpenGLWidget() = default;

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private Q_SLOTS:
    void onStageOpened(const QString& filePath);

private:
    void initialize();

private:
    UsdDocument*                    m_usdDocument;
    std::unique_ptr<ViewportEngine> m_viewportEngine;
    std::unique_ptr<UsdCamera>      m_camera;

    double                          m_height{1};
    double                          m_width{1};

    QPoint                          m_lastMousePosition;
};

} // namespace HVW_NS
