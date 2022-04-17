#include <new>

#include "QDir"
#include "QFileDialog"
#include "QMessageBox"

#include "logger.hpp"
#include "ui_logger.h"

Logger *Logger::create(QWidget *parent)
{
    Logger *ret = new (std::nothrow) Logger(parent);
    if (!ret) return nullptr;

    ret->m_ui = new (std::nothrow) Ui::Logger;
    if (!ret->m_ui)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    return ret;
}

Logger::~Logger()
{
    if (m_ui) delete m_ui;
}

void Logger::write(Level level, QString &msg)
{
    write(level, msg.toLocal8Bit().constData());
}

void Logger::write(Level level, const char *in)
{
    if (!in || m_logLevel == Level::Off || level < m_logLevel) return;

    std::unique_lock<std::mutex> lock(m_mutex);
    time_t current = time(nullptr);
    printTimeStamp(current);
    char label[8192];
    label[0] = '\0';
    switch (level)
    {
    case Verbose:
    {
        sprintf(label, "%s Verbose: %s\n", m_log, in);
        break;
    }
    case Debug:
    {
        sprintf(label, "%s Debug: %s\n", m_log, in);
        break;
    }
    case Info:
    {
        sprintf(label, "%s Info: %s\n", m_log, in);
        break;
    }
    case Warning:
    {
        sprintf(label, "%s Warning: %s\n", m_log, in);
        break;
    }
    case Error:
    {
        sprintf(label, "%s Error: %s\n", m_log, in);
        break;
    }
    default:
    {
        break;
    }
    }

    m_ui->log->appendPlainText(label);
}

// private slots
void Logger::on_saveBtn_clicked()
{
#ifdef Q_OS_WINDOWS
    QString res = QFileDialog::getSaveFileName(this,
                                               "Save log",
                                               QDir::currentPath(),
                                               "log (*.log)");
#else
    QString res = QFileDialog::getSaveFileName(this,
                                               "Save log",
                                               QDir::homePath(),
                                               "log (*.log)");
#endif

    if (res.isEmpty()) return;

    std::unique_lock<std::mutex> lock(m_mutex);
    FILE *f(nullptr);
    f = fopen(res.toLocal8Bit().constData(), "w");
    if (!f)
    {
        QMessageBox::critical(this, "Error", "Fail to open file.");
        return;
    }

    auto text = m_ui->log->toPlainText().toLocal8Bit();
    fwrite(text.constData(), 1, text.size(), f);
    fclose(f);
}

void Logger::on_clearBtn_clicked()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_ui->log->clear();
}

// private member functions
Logger::Logger(QWidget *parent) :
    QDialog(parent),
    m_ui(nullptr),
    m_logLevel(Info)
{}

void Logger::printTimeStamp(time_t in)
{
    struct tm *tm = localtime(&in);
    if (!tm) return;

    sprintf(m_log,
            "%d-%02d-%02d %02d:%02d:%02d",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec);
}
