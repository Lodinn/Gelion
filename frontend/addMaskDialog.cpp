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
    connect( ui->checkBoxInverse, SIGNAL(stateChanged(int)), this, SLOT(changeInversion(int)) );
}

addMaskDialog::~addMaskDialog()
{
    delete ui;
}

void addMaskDialog::setData(QString title, QString formula)
{
    m_title = title;
    m_formula = formula;
    ui->checkBoxInverse->setChecked(false);
    ui->lineEditTitle->setText(getTitle());
    ui->textEditFormula->setText(getFormula());
}

QString addMaskDialog::getFormula()
{
    if (ui->checkBoxInverse->isChecked()) return m_formula + "(inv)";
    else return m_formula;
}

QString addMaskDialog::getTitle()
{
    if (ui->checkBoxInverse->isChecked()) return m_title + "(Инвертированная)";
    else return m_title;

}

void addMaskDialog::changeInversion(int)
{
    ui->lineEditTitle->setText(getTitle());
    ui->textEditFormula->setText(getFormula());
}

void addMaskDialog::getData(QString &title, QString &formula, bool &inverse)
{
    title = ui->lineEditTitle->text();
    inverse = ui->checkBoxInverse->isChecked();
}
