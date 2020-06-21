#ifndef ZGRAPHPARAMDLG_H
#define ZGRAPHPARAMDLG_H

#include <QDialog>

namespace Ui {
class zgraphParamDlg;
}

class zgraphParamDlg : public QDialog
{
    Q_OBJECT

public:
    explicit zgraphParamDlg(QWidget *parent = 0);
    ~zgraphParamDlg();
public:
    Ui::zgraphParamDlg *ui;
};

#endif // ZGRAPHPARAMDLG_H
