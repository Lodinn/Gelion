#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QAbstractButton>

settingsDialog::settingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settingsDialog)
{
    ui->setupUi(this);

    ui->buttonBox->buttons().at(1)->setText("Отмена");

    setWindowIcon(QIcon(":/icons/256_colors.png"));

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

settingsDialog::~settingsDialog()
{
    delete ui;
}

void settingsDialog::setGlobalSettings(J09::globalSettingsType GS)
{

}

void settingsDialog::getGlobalSettings(J09::globalSettingsType &GS)
{

}
