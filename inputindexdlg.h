#ifndef INPUTINDEXDLG_H
#define INPUTINDEXDLG_H

#include <QDialog>

namespace Ui {
class inputIndexDlg;
}

class inputIndexDlg : public QDialog
{
    Q_OBJECT

public:
    explicit inputIndexDlg(QWidget *parent = nullptr);
    ~inputIndexDlg();
    void setSpectralRange(QVector<double> &wls);
    QString get_formula();
private:
    Ui::inputIndexDlg *ui;
};

#endif // INPUTINDEXDLG_H
