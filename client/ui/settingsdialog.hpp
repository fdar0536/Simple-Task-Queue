#pragma once

#include "QDialog"
#include "QHash"
#include "QJsonObject"
#include "QRegularExpressionValidator"

#include "settingsdata.hpp"

namespace Ui
{
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:

    static SettingsDialog *create(QWidget *parent = nullptr);

    ~SettingsDialog();

private slots:

    void on_hosts_currentIndexChanged(int);

private:

    SettingsDialog(QWidget *parent = nullptr);

    Ui::SettingsDialog *m_ui;

    QString m_configPath;

    QHash<QString, SettingsData> m_config;

    QRegularExpressionValidator *m_regex;

    uint8_t initConfigFile();

    void saveConfigFile();

    void setupData(SettingsData *);
};
