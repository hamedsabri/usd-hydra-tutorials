#include "mainMenuBar.h"

#include <QAction>
#include <QApplication>

namespace HVW_NS
{

MainMenuBar::MainMenuBar(QWidget* parent)
    : QMenuBar(parent)
{
    setupMenus();
}

QMenu* MainMenuBar::getPanelsMenu() const 
{ 
    return m_panelsMenu; 
}

void MainMenuBar::setupMenus()
{
    // file
    QMenu* fileMenu = addMenu("File");

    QAction* newStageAction = new QAction("New Stage", this);
    QAction* openStageAction = new QAction("Open Stage", this);
    QAction* quitAction = new QAction("Quit", this);

    fileMenu->addAction(newStageAction);
    fileMenu->addAction(openStageAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    // panels
    m_panelsMenu = addMenu("Panels");

    connect(newStageAction, &QAction::triggered, [this]() {
    });
    connect(openStageAction, &QAction::triggered, [this]() {
    });
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

} // namespace HVW_NS