#pragma once

#include <QMenu>
#include <QMenuBar>

namespace HVW_NS
{

/**
 * @brief Application main menu bar.
 *
 */
class MainMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    /**
     * @brief Constructs the main menu bar.
     *
     * @param parent Optional parent widget.
     */
    MainMenuBar(QWidget* parent = nullptr);

    /**
     * @brief Default destructor.
     */
    virtual ~MainMenuBar() = default;

    /**
     * @brief Returns the menu containing panel-related actions.
     *
     * @return Pointer to the panels menu.
     */
    QMenu* getPanelsMenu() const;

private:
    /**
     * @brief Creates menus and populates menu actions.
     */
    void setupMenus();

private:
    QMenu* m_panelsMenu;
};

} // namespace HVW_NS