#include "setdialog.h"
#include "ui_setdialog.h"

#include <QDebug>

setDialog::setDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::setDialog)
{
    ui->setupUi(this);
    iscks << true << true << true << true << true << true << true << true << true << true;
}

setDialog::~setDialog()
{
    delete ui;
}

QList<bool> setDialog::getcks()
{
    return iscks;
}

void setDialog::on_buttonBox_accepted()
{
    if (ui->checkBox->isChecked())  iscks[0] = true;
    else iscks[0] = false;

    if (ui->checkBox->isChecked())  iscks[1] = true;
    else iscks[1] = false;

    if (ui->checkBox->isChecked())  iscks[2] = true;
    else iscks[2] = false;

    if (ui->checkBox->isChecked())  iscks[3] = true;
    else iscks[3] = false;

    if (ui->checkBox->isChecked())  iscks[4] = true;
    else iscks[4] = false;

    if (ui->checkBox->isChecked())  iscks[5] = true;
    else iscks[5] = false;

    if (ui->checkBox->isChecked())  iscks[6] = true;
    else iscks[6] = false;

    if (ui->checkBox->isChecked())  iscks[7] = true;
    else iscks[7] = false;

    if (ui->checkBox->isChecked())  iscks[8] = true;
    else iscks[8] = false;

    if (ui->checkBox->isChecked())  iscks[9] = true;
    else iscks[9] = false;
}
