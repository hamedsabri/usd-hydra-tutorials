#pragma once

#include <QMainWindow>

namespace HVW_NS
{

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow() = default;
};

} // namespace HVW_NS
