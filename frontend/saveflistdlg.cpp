#include "saveflistdlg.h"
#include "ui_saveflistdlg.h"

SaveFListDlg::SaveFListDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveFListDlg)
{
    ui->setupUi(this);

    ui->buttonBox->buttons().at(1)->setText("Отмена");

    setWindowIcon(QIcon(":/icons/256_colors.png"));

    ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    setWindowFlags( Qt::Dialog | Qt::WindowTitleHint );

    ui->labelInfo->setText("Файлы будут сохранены в рабочий каталог проекта\n"
                           "Имена файлов соответствуют наименованию в списке\n"
                           "Недопустимые символы  ( \ / : * ? "" < > | )\n"
                           "будут заменены знаком 'нижнее подчеркивание'");
}

SaveFListDlg::~SaveFListDlg()
{
    delete ui;
}
