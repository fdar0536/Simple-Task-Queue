#pragma once

#include "QMainWindow"

#include "logger.hpp"

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

private slots:

    void on_actionLog_triggered();

private:

    Ui::MainWindow *m_ui;

    Logger *m_logger;
};

