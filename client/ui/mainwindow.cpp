#include <new>

#include "QMessageBox"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(nullptr),
    m_logger(nullptr)
{
}

MainWindow::~MainWindow()
{
    if (m_ui) delete m_ui;
    if (m_logger) delete m_logger;
}

// public member functions
uint8_t MainWindow::init(MainWindow *w)
{
    if (!w) return 1;

    w->m_ui = new (std::nothrow) Ui::MainWindow;
    if (!w->m_ui) return 1;

    w->m_ui->setupUi(w);

    w->m_logger = LoggerDialog::create(w);
    if (!w->m_logger)
    {
        return 1;
    }

    return 0;
}

// private slots
void MainWindow::on_actionLog_triggered()
{
    m_logger->open();
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this);
}
