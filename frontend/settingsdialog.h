#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <SpectralImage.h>

namespace Ui {
class settingsDialog;
}

class settingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit settingsDialog(QWidget *parent = nullptr);
    ~settingsDialog();
    Ui::settingsDialog *ui;
    void setGlobalSettings(J09::globalSettingsType GS);
    void getGlobalSettings(J09::globalSettingsType &GS);

private:

};

#endif // SETTINGSDIALOG_H
