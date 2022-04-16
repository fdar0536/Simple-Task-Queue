#include <new>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(nullptr)
{
}

MainWindow::~MainWindow()
{
    if (m_ui) delete m_ui;
}

// public member functions
uint8_t MainWindow::init(MainWindow *w)
{
    if (!w) return 1;

    w->m_ui = new (std::nothrow) Ui::MainWindow;
    if (!w->m_ui) return 1;

    w->m_ui->setupUi(w);
    return 0;
}
