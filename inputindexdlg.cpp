#include "inputindexdlg.h"
#include "ui_inputindexdlg.h"

inputIndexDlg::inputIndexDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::inputIndexDlg)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(":/icons/256_colors.png"));
    setWindowTitle("Добавить индексное изображение");

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

inputIndexDlg::~inputIndexDlg()
{
    delete ui;
}

void inputIndexDlg::setSpectralRange(QVector<double> &wls)
{
    int count = wls.count();    if (count == 0) return;
    double begin = wls[0];    double end = wls[count - 1];
    double width = (end - begin) / count;
    ui->labelSpectralRangeInfo->setText(
       QString("каналов - %1 ширина - %2 нм диапазон - %3  .. %4 нм")
                .arg(count).arg(width, 0, 'f', 1).arg(begin, 0, 'f', 0).arg(end, 0, 'f', 0));
}

QString inputIndexDlg::get_formula()
{
    return ui->lineEditInputFormula->text();
}
