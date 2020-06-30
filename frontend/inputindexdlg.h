#ifndef INPUTINDEXDLG_H
#define INPUTINDEXDLG_H

#include "SpectralImage.h"

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
    QString get_index_title();
    QString get_formula();
    Ui::inputIndexDlg *ui;
    void setDefaultIndexList(QVector<J09::indexType> indexList);
    QStringList namesList;
private slots:
    void predefined_index_menu();
    void on_pushButtonIndexes_clicked();

private:
    QVector<QAction *> predefined_index_list_acts;
};

#endif // INPUTINDEXDLG_H
