#include "ui/mainWindow.h"
#include "ui/loggerWidget.h"

#include <QApplication>
#include <QFile>
#include <QGuiApplication>

int main(int argc, char** argv)
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication app(argc, argv);

    QFile styleFile(":/darktheme.css");
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        QString darkStyle = styleFile.readAll();
        app.setStyleSheet(darkStyle);
    }

    HVW_NS::LogWidget::instance().installMessageHandler();

    HVW_NS::MainWindow mainWindow;
    mainWindow.resize(1600, 800);
    mainWindow.show();

    return app.exec();
}