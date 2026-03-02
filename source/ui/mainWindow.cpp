#include "mainWindow.h"
#include "loggerWidget.h"
#include "mainMenuBar.h"
#include "model/usdDocument.h"
#include "viewportOpenGLWidget.h"

#include "DockManager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStatusBar>

namespace HVW_NS
{

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("UsdViewer: untitled");

    auto dockManager = new ads::CDockManager(this);
    auto mainMenuBar = new MainMenuBar(this);
    auto usdDocument = new UsdDocument(this);
    auto viewportGLWidget = new ViewportOpenGLWidget(usdDocument, this);

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

    // connection signal/slots
    connect(mainMenuBar, &MainMenuBar::newStageSignal, usdDocument, &UsdDocument::createNewStageInMemory);
    connect(mainMenuBar, &MainMenuBar::openStageSignal, usdDocument, &UsdDocument::openStage);
    connect(usdDocument, &UsdDocument::stageOpened, [this](const QString& filePath) {
        QString baseName = QFileInfo(filePath).fileName();
        QString title = QString("%1 - UsdViewer: %2").arg(baseName, QDir::toNativeSeparators(filePath));
        setWindowTitle(title);
    });
}

} // namespace HVW_NS