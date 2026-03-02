#include "loggerWidget.h"

#include <QColor>
#include <QContextMenuEvent>
#include <QDateTime>
#include <QFile>
#include <QMenu>
#include <QTextEdit>
#include <QTextStream>

namespace HVW_NS
{

std::once_flag LogWidget::s_initFlag;

LogWidget* LogWidget::s_instance = nullptr;

LogWidget::LogWidget(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
}

LogWidget& LogWidget::instance(QWidget* parent)
{
    std::call_once(s_initFlag, [parent]() {
        s_instance = new LogWidget(parent);
    });
    return *s_instance;
}

void LogWidget::installMessageHandler()
{
    qInstallMessageHandler(LogWidget::messageHandler);
}

void LogWidget::clearLog()
{
    QMetaObject::invokeMethod(this, &QPlainTextEdit::clear, Qt::QueuedConnection);
}

void LogWidget::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);
    instance().appendMessage(type, msg);
}

void LogWidget::appendMessage(QtMsgType type, const QString& msg)
{
    QMutexLocker locker(&m_mutex);

    QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString level;
    QString color;

    // https://doc.qt.io/qt-6/qtlogging.html
    switch (type) {
        case QtDebugMsg:    level = "DEBUG";   color = "lightgray"; break;
        case QtInfoMsg:     level = "INFO";    color = "white";     break;
        case QtWarningMsg:  level = "WARN";    color = "orange";    break;
        case QtCriticalMsg: level = "ERROR";   color = "red";       break;
        case QtFatalMsg:    level = "FATAL";   color = "darkred";   break;
    }

    QString formatted = QString("[%1] [%2] %3").arg(time, level, msg);
    QString escaped = formatted.toHtmlEscaped();
    QString html = QString("<span style='color:%1;'>%2</span>").arg(color, escaped);

    QMetaObject::invokeMethod(this, [this, html]() {
        this->appendHtml(html);
    }, Qt::QueuedConnection);

    if (type == QtFatalMsg) {
        abort();
    }
}

void LogWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);

    menu.addAction(tr("Clear Log"), this, &LogWidget::clearLog);

    menu.addSeparator();
    menu.addActions(createStandardContextMenu()->actions());

    menu.exec(event->globalPos());
}

} // namespace HVW_NS