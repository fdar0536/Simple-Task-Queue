#pragma once

#include "QMainWindow"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    static uint8_t init(MainWindow *w);

    ~MainWindow();

private:
    Ui::MainWindow *m_ui;
};

