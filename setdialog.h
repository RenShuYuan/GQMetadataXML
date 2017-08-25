#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QDialog>
#include <QList>

namespace Ui {
class setDialog;
}

class setDialog : public QDialog
{
    Q_OBJECT

public:
    explicit setDialog(QWidget *parent = 0);
    ~setDialog();

    QList<bool> getcks();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::setDialog *ui;

    QList<bool> iscks;
};

#endif // SETDIALOG_H
