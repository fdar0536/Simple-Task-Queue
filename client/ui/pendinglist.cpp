#include "pendinglist.hpp"
#include "ui_pendinglist.h"

PendingList::PendingList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PendingList)
{
    ui->setupUi(this);
}

PendingList::~PendingList()
{
    delete ui;
}
