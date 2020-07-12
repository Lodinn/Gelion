#include "addMaskDialog.h"
#include "ui_addMaskDialog.h"

#include <QAbstractButton>

addMaskDialog::addMaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::addMaskDialog)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/256_colors.png"));
    setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );
    ui->buttonBox->buttons().at(1)->setText("Отмена");
}

addMaskDialog::~addMaskDialog()
{
    delete ui;
}

void addMaskDialog::setData(QString title, QString formula)
{
    ui->lineEditTitle->setText(title);
    ui->lineEditFormula->setText(formula);
}

void addMaskDialog::getData(QString &title)
{
    title = ui->lineEditTitle->text();
}
