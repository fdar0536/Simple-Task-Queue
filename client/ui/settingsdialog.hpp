#pragma once

#include "QDialog"
#include "QHash"
#include "QJsonObject"
#include "QRegularExpressionValidator"

#include "saveconfigdialog.hpp"

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

    void reject() override;

private slots:

    void on_hosts_currentIndexChanged(int);

    void on_ip_textChanged(const QString &);

    void on_port_valueChanged(int);

    void on_saveBtn_clicked(bool);

    void on_exitBtn_clicked(bool);

    void onSaveAccepted();

private:

    SettingsDialog(QWidget *parent = nullptr);

    Ui::SettingsDialog *m_ui;

    QString m_configPath;

    QHash<QString, SettingsData> m_config;

    QRegularExpressionValidator *m_regex;

    SaveConfigDialog *m_saveConfigDialog;

    uint8_t initConfigFile();

    void saveConfigFile();

    void setupData(SettingsData *);

    void verifyIP();
};
