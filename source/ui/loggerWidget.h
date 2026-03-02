#pragma once

#include <QPlainTextEdit>
#include <QMutex>
#include <QFile>

namespace HVW_NS
{

/**
 * @brief Centralized log output widget for the application.
 *
 * LogWidget captures Qt log messages and displays them in a
 * scrollable text widget.The widget is thread-safe and can receive log messages
 * from non-UI threads via Qt's message handling system.
 */
class LogWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    /**
     * @brief Returns the singleton LogWidget instance.
     *
     * The widget is lazily constructed on first access.
     *
     */
    static LogWidget& instance(QWidget* parent = nullptr);

    /**
     * @brief Installs the LogWidget as the Qt message handler.
     *
     * After installation, all Qt log output (qDebug, qWarning,
     * qCritical, etc.) will be forwarded to this widget.
     */
    void installMessageHandler();

    /**
     * @brief Clears all logged messages from the widget.
     */
    void clearLog();

protected:
    /**
     * @brief Handles context menu events.
     *
     * Provides a custom context menu for log-related actions,
     * such as clearing or copying log contents.
     *
     * @param event Context menu event information.
     */
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    /**
     * @brief Private constructor to enforce singleton usage.
     *
     * @param parent Optional parent widget.
     */
    LogWidget(QWidget* parent = nullptr);

    /**
     * @brief Qt message handler callback.
     *
     * This static function is registered with Qt and receives
     * all log messages, which are then forwarded to the
     * LogWidget instance.
     *
     * @param type Logging message type.
     * @param context Context information provided by Qt.
     * @param msg Log message text.
     */
    static void messageHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& msg);

    /**
     * @brief Appends a formatted log message to the widget.
     *
     * This function is responsible for synchronizing access
     * and updating the UI safely.
     *
     * @param type Logging message type.
     * @param msg Log message text.
     */
    void appendMessage(QtMsgType type, const QString& msg);

private:
    QMutex                m_mutex;
    static LogWidget*     s_instance;
    static std::once_flag s_initFlag;
};

} // namespace HVW_NS
