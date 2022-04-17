#pragma once

#include <mutex>

#include "QDialog"

namespace Ui
{
class Logger;
}

class Logger : public QDialog
{
    Q_OBJECT

public:

    typedef enum Level
    {
        Verbose, Debug, Info, Warning, Error, Off
    } Level;

    static Logger *create(QWidget *parent = nullptr);

    ~Logger();

    void write(Level, QString &);

    void write(Level, const char *);

private slots:

    void on_saveBtn_clicked();

    void on_clearBtn_clicked();

private:

    Logger(QWidget *parent = nullptr);

    void printTimeStamp(time_t);

    Ui::Logger *m_ui;

    Level m_logLevel;

    char m_log[128];

    std::mutex m_mutex;
};

