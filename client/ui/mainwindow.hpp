#pragma once

#include "QMainWindow"

#include "loggerdialog.hpp"

#include "settingsdialog.hpp"

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

    void on_actionSettings_triggered();

    void on_actionLog_triggered();

    void on_actionAbout_Qt_triggered();

private:

    Ui::MainWindow *m_ui;

    LoggerDialog *m_logger;

    SettingsDialog *m_settingsDialog;
};

