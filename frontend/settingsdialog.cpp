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
    if (GS.load_resent_project) ui->checkBox_load_recent_file->setCheckState(Qt::Checked);
    else ui->checkBox_load_recent_file->setCheckState(Qt::Unchecked);

    if (GS._zobject_list_load) ui->checkBox_zobject_list_load->setCheckState(Qt::Checked);
    else ui->checkBox_zobject_list_load->setCheckState(Qt::Unchecked);

    if (GS._index_save) ui->checkBox_index_save->setCheckState(Qt::Checked);
    else ui->checkBox_index_save->setCheckState(Qt::Unchecked);

    if (GS._index_load) ui->checkBox_index_load->setCheckState(Qt::Checked);
    else ui->checkBox_index_load->setCheckState(Qt::Unchecked);

    if (GS._channel_list_load) ui->checkBox_channel_list_load->setCheckState(Qt::Checked);
    else ui->checkBox_channel_list_load->setCheckState(Qt::Unchecked);

    if (GS._masks_save) ui->checkBox_masks_save->setCheckState(Qt::Checked);
    else ui->checkBox_masks_save->setCheckState(Qt::Unchecked);

    if (GS._masks_load) ui->checkBox_masks_load->setCheckState(Qt::Checked);
    else ui->checkBox_masks_load->setCheckState(Qt::Unchecked);

    ui->lineEdit_global_angle->setText(QString("%1").arg(GS.main_rgb_rotate_start,0,'f',0));
    ui->lineEdit_global_scale->setText(QString("%1").arg(GS.main_rgb_scale_start,0,'f',0));
    if (GS.restore_project_settings_view) ui->checkBox_load_project_settings_view->setCheckState(Qt::Checked);
    else ui->checkBox_load_project_settings_view->setCheckState(Qt::Unchecked);
}


void settingsDialog::getGlobalSettings(J09::globalSettingsType &GS)
{
    GS.load_resent_project = ui->checkBox_load_recent_file->checkState() == Qt::Checked;
    GS._zobject_list_load = ui->checkBox_zobject_list_load->checkState() == Qt::Checked;
    GS._index_save = ui->checkBox_index_save->checkState() == Qt::Checked;
    GS._index_load = ui->checkBox_index_load->checkState() == Qt::Checked;
    GS._channel_list_load = ui->checkBox_channel_list_load->checkState() == Qt::Checked;
    GS._masks_save = ui->checkBox_masks_save->checkState() == Qt::Checked;
    GS._masks_load = ui->checkBox_masks_load->checkState() == Qt::Checked;

    GS.main_rgb_rotate_start = ui->lineEdit_global_angle->text().toDouble();
    GS.main_rgb_scale_start = ui->lineEdit_global_scale->text().toDouble();
    GS.restore_project_settings_view = ui->checkBox_load_project_settings_view->checkState() == Qt::Checked;
}
