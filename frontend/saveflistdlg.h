#ifndef SAVEFLISTDLG_H
#define SAVEFLISTDLG_H

#include <QDialog>

namespace Ui {
class SaveFListDlg;
}

class SaveFListDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SaveFListDlg(QWidget *parent = nullptr);
    ~SaveFListDlg();

public:
    Ui::SaveFListDlg *ui;
};

#endif // SAVEFLISTDLG_H
