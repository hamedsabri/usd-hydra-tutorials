#include "mainWindow.h"
#include "loggerWidget.h"
#include "mainMenuBar.h"
#include "viewportOpenGLWidget.h"

#include "DockManager.h"

#include <QStatusBar>

namespace HVW_NS
{

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("UsdViewer: untitled");

    auto dockManager = new ads::CDockManager(this);
    auto mainMenuBar = new MainMenuBar(this);
    auto viewportGLWidget = new ViewportOpenGLWidget(this);

    LogWidget& loggerWidget = LogWidget::instance(this);

    // menubar
    setMenuBar(mainMenuBar);

    // statusbar
    auto statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // ads styles heet
    QFile StyleSheetFile(":/drak_ads.css");
    if (StyleSheetFile.open(QIODevice::ReadOnly))
    {
        QTextStream StyleSheetStream(&StyleSheetFile);
        auto        Stylesheet = StyleSheetStream.readAll();
        StyleSheetFile.close();
        dockManager->setStyleSheet(Stylesheet);
    }

    // openGlViewport dockWidget
    ads::CDockWidget* openGlViewportDockWidget = new ads::CDockWidget("Viewport");
    openGlViewportDockWidget->setWidget(viewportGLWidget);

    openGlViewportDockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromDockWidget);
    openGlViewportDockWidget->setMinimumSize(600, 450);

    openGlViewportDockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    openGlViewportDockWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
    openGlViewportDockWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);

    dockManager->addDockWidget(ads::CenterDockWidgetArea, openGlViewportDockWidget);
    mainMenuBar->getPanelsMenu()->addAction(openGlViewportDockWidget->toggleViewAction());

    // logger
    ads::CDockWidget* loggerDockWidget = new ads::CDockWidget("Logger");
    loggerDockWidget->setWidget(&loggerWidget);
    loggerDockWidget->setMinimumSizeHintMode(ads::CDockWidget::MinimumSizeHintFromDockWidget);
    loggerDockWidget->setMinimumSize(340, 50);
    dockManager->addDockWidget(ads::BottomDockWidgetArea, loggerDockWidget);
    mainMenuBar->getPanelsMenu()->addAction(loggerDockWidget->toggleViewAction());
}

} // namespace HVW_NS