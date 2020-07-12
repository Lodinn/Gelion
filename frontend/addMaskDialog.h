#ifndef ADDMASKDIALOG_H
#define ADDMASKDIALOG_H

#include <QDialog>

namespace Ui {
class addMaskDialog;
}

class addMaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit addMaskDialog(QWidget *parent = nullptr);
    ~addMaskDialog();
    void setData(QString title, QString formula);
    void getData(QString &title);

private:
    Ui::addMaskDialog *ui;
};

#endif // ADDMASKDIALOG_H
