#pragma once

#include "QDialog"

#include "settingsdata.hpp"

namespace Ui
{
class SaveConfigDialog;
}

class SaveConfigDialog : public QDialog
{
    Q_OBJECT

public:

    static SaveConfigDialog *create(QWidget * = nullptr);

private:

    SaveConfigDialog(QWidget * = nullptr);

    QHash<QString, SettingsData> *m_config;
};
