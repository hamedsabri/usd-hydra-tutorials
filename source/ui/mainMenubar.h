#pragma once

#include <QMenu>
#include <QMenuBar>

namespace HVW_NS
{

/**
 * @brief Application main menu bar.
 */
class MainMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    MainMenuBar(QWidget* parent = nullptr);
    virtual ~MainMenuBar() = default;

    QMenu* getPanelsMenu() const;

Q_SIGNALS:
    void newStageSignal();
    void openStageSignal(const QString& path);

private:
    void setupMenus();

private:
    QMenu* m_panelsMenu;
};

} // namespace HVW_NS