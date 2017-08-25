#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include "xmlstreamreader.h"

class setDialog;
class QString;
class QTreeWidgetItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void createStatusBar();

    bool startEdit(const QString &text);
    bool endEdit(const QString &text);
    bool replaceVol(const QString &key, const QString &vol);

    void printInfo(const int countMax, const int errSize, QStringList &errList);

private slots:
    void on_action_triggered();

    void on_action_1_triggered();

    void on_action_2_triggered();

    void on_action_3_triggered();

    void on_action_4_triggered();

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void setListVisible(bool bl);

    void on_action_5_triggered();

    void on_pushButton_2_clicked();

    void on_action_6_triggered();

    void on_action_7_triggered();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_action_8_triggered();

    void on_action_9_triggered();

    void on_action_10_triggered();

    void on_action_11_triggered();

    void on_action_13_triggered();

    void on_action_12_triggered();

    void on_action_17_triggered();

    void on_action_18_triggered();

private:
    Ui::MainWindow *ui;

    bool isVb;

    //保存设置选项
    QList<bool> iscks;

    //保存元数据模板名称
    QString str_template;

    //保存元数据路径
    QString filePath;

    setDialog* myDialog;

    QStringList jd_list;
    QStringList xml_list;
    QStringList yx_list;
    QStringList cerr_list;
    XmlStreamReader *xmlnew;

    QStringList bufferList;
};

#endif // MAINWINDOW_H
