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
    void getData(QString &title, QString &formula, bool &inverse);

private:
    Ui::addMaskDialog *ui;
    QString m_title, m_formula;
    QString getFormula();
    QString getTitle();
private slots:
    void changeInversion(int);
};

#endif // ADDMASKDIALOG_H
