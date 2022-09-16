#include "pendinglist.hpp"
#include "../view/ui_pendinglist.h"

PendingList *PendingList::create(QWidget *parent)
{
    PendingList *ret(nullptr);

    try
    {
        ret = new PendingList(parent);
        ret->m_ui = new Ui::PendingList;
    }
    catch(...)
    {
        if (ret) delete ret;
        return nullptr;
    }

    ret->m_model = PendingListModel::create(ret);
    if (!ret->m_model)
    {
        delete ret;
        return nullptr;
    }

    ret->m_ui->setupUi(ret);
    return ret;
}

// private
PendingList::PendingList(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_model(nullptr)
{}

PendingList::~PendingList()
{
    if (m_ui) delete m_ui;
    if (m_model) delete m_model;
}
