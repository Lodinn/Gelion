#include "zgraphparamdlg.h"
#include "ui_zgraphparamdlg.h"

zgraphParamDlg::zgraphParamDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::zgraphParamDlg)
{
    ui->setupUi(this);

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

}

zgraphParamDlg::~zgraphParamDlg()
{
    delete ui;
}

/*QDialog dlg(this);
dlg.setWindowTitle(tr("My dialog"));

QLineEdit *ledit1 = new QLineEdit(&dlg);
QLineEdit *ledit2 = new QLineEdit(&dlg);

QDialogButtonBox *btn_box = new QDialogButtonBox(&dlg);
btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

QFormLayout *layout = new QFormLayout();
layout->addRow(tr("Line edit 1:"), ledit1);
layout->addRow(tr("Line edit 2:"), ledit2);
layout->addWidget(btn_box);

dlg.setLayout(layout);

// В случае, если пользователь нажал "Ok".
if(dlg.exec() == QDialog::Accepted) {
    const QString &str1 = ledit1->text();
    const QString &str2 = ledit2->text();
}*/

//void zgraphParamDlg::onOkClicked(bool b)
//{
//    result = true;
//    close();
//    Q_UNUSED(b);
//}

//void zgraphParamDlg::onCancelClicked(bool b)
//{
//    close();
//    Q_UNUSED(b);
//}
