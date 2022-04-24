#pragma once

#include <mutex>

#include "QDialog"

namespace Ui
{
class LoggerDialog;
}

class LoggerDialog : public QDialog
{
    Q_OBJECT

public:

    typedef enum Level
    {
        Verbose, Debug, Info, Warning, Error, Off
    } Level;

    static LoggerDialog *create(QWidget *parent = nullptr);

    ~LoggerDialog();

    void write(Level, QString &);

    void write(Level, const char *);

private slots:

    void on_saveBtn_clicked();

    void on_clearBtn_clicked();

private:

    LoggerDialog(QWidget *parent = nullptr);

    void printTimeStamp(time_t);

    Ui::LoggerDialog *m_ui;

    Level m_logLevel;

    char m_log[128];

    std::mutex m_mutex;
};

