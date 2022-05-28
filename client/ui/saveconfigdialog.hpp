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

    static SaveConfigDialog *create(QHash<QString, SettingsData> *,
        QWidget * = nullptr);

    ~SaveConfigDialog();

    QString getName() const;

    void reset();

    void reject() override;

protected:

    void keyPressEvent(QKeyEvent *) override;

private slots:

    void on_name_textChanged(const QString &);

    void on_okBtn_clicked(bool);

    void on_cancelBtn_clicked(bool);

private:

    SaveConfigDialog(QWidget * = nullptr);

    Ui::SaveConfigDialog *m_ui;

    QHash<QString, SettingsData> *m_config;
};
