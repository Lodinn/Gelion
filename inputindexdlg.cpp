#include "inputindexdlg.h"
#include "ui_inputindexdlg.h"

#include <QMenu>

inputIndexDlg::inputIndexDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::inputIndexDlg)
{
    ui->setupUi(this);

    ui->buttonBox->buttons().at(1)->setText("Отмена");

    setWindowIcon(QIcon(":/icons/256_colors.png"));
    setWindowTitle("Добавить индексное изображение");

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
// "Normalized difference water index"
    QAction *action = new QAction("Normalized difference water index", this);
    action->setData("(R800-R650)/(R800+R650)");  predefined_index_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(predefined_index_menu()));
// "Green Difference Vegetation Index (GDVI)"
    action = new QAction("Green Difference Vegetation Index (GDVI)", this);
    action->setData("R800-R650");  predefined_index_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(predefined_index_menu()));
// "Pan NDVI"
    action = new QAction("Pan NDVI", this);
    action->setData("(r800-r550-r640-r450)/(r800+r550+r640+r450)");  predefined_index_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(predefined_index_menu()));
// "NIR/Blue Blue-normalized difference vegetation index"
    action = new QAction("NIR/Blue Blue-normalized difference vegetation index", this);
    action->setData("(r800-r450)/(r800+r450)");  predefined_index_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(predefined_index_menu()));
// "Curvative Index (CI)"
    action = new QAction("Curvative Index (CI)", this);
    action->setData("r675*r690/r683");  predefined_index_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(predefined_index_menu()));
// "Difference 833/658"
    action = new QAction("Difference 833/658", this);
    action->setData("r833-r658");  predefined_index_list_acts.append(action);
    connect(action, SIGNAL(triggered()), this, SLOT(predefined_index_menu()));

// https://www.indexdatabase.de/db/i.php

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

QString inputIndexDlg::get_index_title()
{
    return ui->lineEditInputTitle->text();
}

QString inputIndexDlg::get_formula()
{
    return ui->lineEditInputFormula->text();
}

void inputIndexDlg::predefined_index_menu()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        ui->lineEditInputTitle->setText(action->text());
        ui->lineEditInputFormula->setText(action->data().toString());
    }  // if
}

void inputIndexDlg::on_pushButtonIndexes_clicked()
{
    QPoint pos = ui->pushButtonIndexes->geometry().topRight();
    QPoint globalPos = mapToGlobal(pos);
    QMenu menu(this);
    foreach(QAction *act, predefined_index_list_acts)
        menu.addAction(act);
    menu.exec(globalPos);

}
