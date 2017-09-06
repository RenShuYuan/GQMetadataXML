#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "setdialog.h"
#include "myogr.h"
#include "core.h"

#include <QPropertyAnimation>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QInputDialog>
#include <QProgressDialog>
#include <QDirIterator>
#include <QFileDialog>
#include <QStringList>
#include <QFileInfo>
#include <QPalette>
#include <QString>
#include <QDebug>
#include <QSize>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //界面动画，改变透明度的方式出现0 - 1渐变
    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(1000);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();

    isVb = false;
    ui->label->setVisible(false);
    ui->label_2->setVisible(false);
    ui->menu_2->setEnabled(false);
    ui->menu_3->setEnabled(false);

    //设置标签颜色
    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::darkCyan);
    ui->label->setPalette(pe);
    pe.setColor(QPalette::WindowText, Qt::red);
    ui->label_2->setPalette(pe);

    ui->listWidget->setVisible(false);
    ui->pushButton_3->setVisible(false);
    int  width = this->size().width()/2;
    this->resize(width, this->size().height());

    //设置treeWidget表头
    QStringList strList;
    strList << "序号" << "错误信息";
    ui->treeWidget->setHeaderLabels(strList);

    //创建状态栏
    createStatusBar();

    xmlnew = NULL;

    myDialog = new setDialog(this);
    iscks=myDialog->getcks();
}

MainWindow::~MainWindow()
{
    delete ui;

    if (xmlnew != nullptr)
    {
        delete xmlnew;
        xmlnew = 0;
    }
}

void MainWindow::createStatusBar()
{
    //添加邮箱地址
    QLabel *mailLabel = new QLabel(this);
    mailLabel->setText("<a href=\"mailto:y15496439@gmail.com\">"
                          "bug反馈及建议:  y15496439@gmail.com</a>");
    mailLabel->setTextFormat(Qt::RichText);
    mailLabel->setOpenExternalLinks(true);
    statusBar()->addPermanentWidget(mailLabel);
}

bool MainWindow::startEdit(const QString &text)
{
    QFile file(text);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return false;

    bufferList.clear();
    QTextStream in(&file);
    in.setCodec("UTF-8");
    while (!in.atEnd())
    {
        bufferList.append(in.readLine());
    }
    file.close();
    return true;
}

bool MainWindow::endEdit(const QString &text)
{
    QFile file(text);
    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
        return false;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    foreach (QString str, bufferList)
    {
        out << str << "\n";
    }
    file.close();
    return true;
}

bool MainWindow::replaceVol(const QString &key, const QString &vol)
{
    QString skey = "<" + key + ">";
    QString ekey = "</" + key + ">";

    for (int i=0; i<bufferList.size(); ++i)
    {
        QString str = bufferList.at(i);
        if (str.contains(skey))
        {
            int istatr = str.indexOf(skey);
            int iend = str.indexOf(ekey);
            if (istatr==-1 || iend==-1)
                return false;
            else
            {
                QString leftStr = str.left(istatr + skey.size());
                QString rightStr = str.mid(iend, str.size());
                bufferList.value(0, leftStr + vol + rightStr);
                bufferList[i] = leftStr + vol + rightStr;
                return true;
            }
        }
    }
    return false;
}

void MainWindow::printInfo(const int countMax, const int errSize, QStringList &errList)
{
    //输出检查个数、错误信息
    ui->label->setText(QString("检查完成: %1景影像正确.").arg(countMax - errSize));
    ui->label_2->setText(QString("              %1景出现错误.").arg(errSize));
    ui->label_2->setVisible(true);

    ui->treeWidget->clear();

    //设置treeWidget表头
    QStringList strList;
    strList << "序号" << "错误信息";
    ui->treeWidget->setHeaderLabels(strList);

    long count = 0;
    foreach (QString str, errList)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString::number(++count));
        item->setText(1, str);
    }

    //将错误列表清空
    errList.clear();
}

void MainWindow::on_action_triggered()
{
    /*打开单个元数据*/

    ui->treeWidget->clear();

    //设置treeWidget表头
    QStringList strList;
    strList << "字段" << "内容";
    ui->treeWidget->setHeaderLabels(strList);

    //浏览元数据
    QString fileName = QFileDialog::getOpenFileName(this, "打开元数据", ".", "XML (*.xml)");
    if (!fileName.isEmpty())
    {
        XmlStreamReader xml(ui->treeWidget);
        if (!xml.readFile(fileName))
        {
            QMessageBox::warning(this, "错误", "XML文件无法读取，已终止运行。");
            ui->menu_3->setEnabled(false);
            return;
        }
        else
        {
            //将所有tree节点展开
            ui->treeWidget->expandAll();
            ui->menu_3->setEnabled(true);
            str_template = fileName;
        }
    }
}

void MainWindow::on_action_1_triggered()
{
    /*设置元数据路径*/

    //设置元数据路径
    filePath = QFileDialog::getExistingDirectory(this, "设置元数据路径", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (filePath.isEmpty())
        return;

    //判断路径是否存在
    QDir dir(filePath);
    if(!dir.exists())
    {
        QMessageBox::warning(this, "错误", "路径不存在，检查后重新设置。");
        ui->menu_2->setEnabled(false);
        return;
    }

    //获取所选文件类型过滤器
    QStringList filters;
    filters << QString("*Y.xml");

    //进度条
    QProgressDialog dialog("搜索元数据...", "取消", 0, 0, this);
    dialog.setWindowTitle("搜索元数据");                      //设置窗口标题
    dialog.setWindowModality(Qt::WindowModal);      //将对话框设置为模态
    dialog.show();

    //定义迭代器并设置过滤器
    xml_list.clear();
    QDirIterator dir_iterator(filePath,filters,QDir::Files | QDir::NoSymLinks,QDirIterator::Subdirectories);
    while(dir_iterator.hasNext())
    {
        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString absolute_file_path = file_info.absoluteFilePath();
        xml_list.append(absolute_file_path);

        //在界面上更新找到的元数据
        dialog.setLabelText(file_info.fileName());

        //避免界面冻结
        QApplication::processEvents();

        //当按下取消按钮则中断
        if (dialog.wasCanceled())
        {
            xml_list.clear();
            ui->menu_2->setEnabled(false);
            ui->menu_3->setEnabled(false);
            return;
        }
    }

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        ui->label->setVisible(false);
        ui->menu_2->setEnabled(false);
        ui->menu_3->setEnabled(false);
        return;
    }
    else
    {
        ui->menu_2->setEnabled(true);
        ui->menu_3->setEnabled(true);
        ui->label->setVisible(true);
        ui->label->setText("找到: " + QString::number(xml_list.size(), 10) + "个影像元数据");
    }

    if (xmlnew != NULL)
    {
        delete xmlnew;
        xmlnew = 0;
    }
    xmlnew = new XmlStreamReader(ui->treeWidget);
}

void MainWindow::on_action_2_triggered()
{
    /*元数据名称一致性检查*/

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        return;
    }

    QProgressDialog prDialog("检查元数据名称与影像名称一致性...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;
        QFileInfo file_info(str);

        //检查元数据名称与数据内容一致性
        XmlStreamReader xml;
        if (!xml.setFilePath(str))
        {
            cerr_list.append("文件名一致性检查: " + file_info.baseName() +
                             "元数据无法正确打开。");
            if (!isCount) { isCount = true; ++errCount; }
            continue;
        }

        if (!xml.isMetaDataFileName(file_info.fileName()))
        {
            cerr_list.append("文件名一致性检查: " + file_info.baseName() +
                             "元数据名称与[MetaDataFileName]字段内容不一致。");
            if (!isCount) { isCount = true; ++errCount; }
        }

        QFileInfo img_info(str.replace(str.size()-5, 5, "F.IMG"));
        if (!img_info.exists())
        {
            cerr_list.append("文件名一致性检查: " + file_info.baseName() +
                             "元数据名称与融合影像（F）名称不一致或没找到对应数据。");
            isCount = true;
            ++errCount;
        }
        img_info.setFile(str.replace(str.size()-5, 5, "P.IMG"));
        if (!img_info.exists())
        {
            cerr_list.append("文件名一致性检查: " + file_info.baseName() +
                             "元数据名称与全色影像（P）名称不一致或没找到对应数据。");
            isCount = true;
            ++errCount;
        }
        img_info.setFile(str.replace(str.size()-5, 5, "M.IMG"));
        if (!img_info.exists())
        {
            cerr_list.append("文件名一致性检查: " + file_info.baseName() +
                             "元数据名称与多光谱影像（P）名称不一致或没找到对应数据。");
            isCount = true;
            ++errCount;
        }

        //检查元数据名称与上级文件夹名称一致性
        QString path = file_info.path();
        QString dirName = path.right(path.size()-path.lastIndexOf("/")-1);

        QString strTemp = file_info.baseName();
        strTemp = strTemp.left(strTemp.size()-1);
        strTemp = strTemp.insert(3, "-");
        strTemp = strTemp.insert(strTemp.size()-8, "-");

        if (!(dirName == strTemp))
        {
            cerr_list.append("文件名一致性检查: " + file_info.baseName() +
                             "元数据名称与上级文件夹名称不一致。");
            if (!isCount) { isCount = true; ++errCount; }
        }

        //检查元数据名称与投影文件名称一致性
        QFileInfo img_info_t(str.replace(str.size()-5, 5, "T.XML"));
        if (!img_info_t.exists())
        {
            cerr_list.append("文件名一致性检查: " + img_info_t.fileName() +
                             "元数据名称与投影文件名称不一致或没找到对应数据。");
            if (!isCount) { isCount = true; ++errCount; }
        }

        xml.close();

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_action_3_triggered()
{
    /*坐标检查*/

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        return;
    }

    QProgressDialog prDialog("检查元数据影像坐标一致性...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    //检查元数据名称与影像名称一致性
    foreach (QString str, xml_list)
    {
        bool isCount = false;

        QFileInfo file_info(str);
        QString strOgr = str;
        myOGR ogr(strOgr.replace(strOgr.size()-5, 5, "P.IMG"));
        if (!ogr.isOpen())
        {
            cerr_list.append("坐标检查: " + file_info.baseName() + "读取影像数据失败。");
            isCount = true;
            ++errCount;
        }
        else
        {
            XmlStreamReader xml;
            if (!xml.setFilePath(str))
            {
                cerr_list.append("坐标检查: " + file_info.baseName() + "元数据无法正确打开。");
                if (!isCount) { isCount = true; ++errCount; }
                continue;
            }

            QStringList xy_list = ogr.getXY();
            QString strSouthWestAbs = xml.getElementValue("SouthWestAbs");
            QString strSouthWestOrd = xml.getElementValue("SouthWestOrd");
            QString strNorthWestAbs = xml.getElementValue("NorthWestAbs");
            QString strNorthWestOrd = xml.getElementValue("NorthWestOrd");
            QString strNorthEastAbs = xml.getElementValue("NorthEastAbs");
            QString strNorthEastOrd = xml.getElementValue("NorthEastOrd");
            QString strSouthEastAbs = xml.getElementValue("SouthEastAbs");
            QString strSouthEastOrd = xml.getElementValue("SouthEastOrd");
            if (!(strSouthWestAbs==xy_list.at(0) && strNorthWestAbs==xy_list.at(0)
                    && strSouthWestOrd==xy_list.at(3) && strSouthEastOrd==xy_list.at(3)
                    && strNorthWestOrd==xy_list.at(1) && strNorthEastOrd==xy_list.at(1)
                    && strNorthEastAbs==xy_list.at(2) && strSouthEastAbs==xy_list.at(2)))
            {
                cerr_list.append("坐标检查: " + file_info.baseName() + "元数据坐标与影像坐标不一致。");
                cerr_list.append("\tSouthWestAbs填写坐标为:" + strSouthWestAbs + ", 程序读取为:" + xy_list.at(0));
                cerr_list.append("\tNorthWestAbs填写坐标为:" + strNorthWestAbs + ", 程序读取为:" + xy_list.at(0));
                cerr_list.append("\tSouthWestOrd填写坐标为:" + strSouthWestOrd + ", 程序读取为:" + xy_list.at(3));
                cerr_list.append("\tSouthEastOrd填写坐标为:" + strSouthEastOrd + ", 程序读取为:" + xy_list.at(3));
                cerr_list.append("\tNorthWestOrd填写坐标为:" + strNorthWestOrd + ", 程序读取为:" + xy_list.at(1));
                cerr_list.append("\tNorthEastOrd填写坐标为:" + strNorthEastOrd + ", 程序读取为:" + xy_list.at(1));
                cerr_list.append("\tNorthEastAbs填写坐标为:" + strNorthEastAbs + ", 程序读取为:" + xy_list.at(2));
                cerr_list.append("\tSouthEastAbs填写坐标为:" + strSouthEastAbs + ", 程序读取为:" + xy_list.at(2));
                if (!isCount) { isCount = true; ++errCount; }
            }
            xml.close();
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount,cerr_list);
}

void MainWindow::on_action_4_triggered()
{
    /*筛选相同项内容*/

    ui->treeWidget->clear();

    //设置treeWidget表头
    QStringList strList;
    strList << "字段" << "内容";
    ui->treeWidget->setHeaderLabels(strList);

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        return;
    }

    if (!xmlnew->readFiles(xml_list))
    {
        QMessageBox::warning(this, "错误", "一个或多个XML文件无法读取，已终止运行。");
        return;
    }
    else
    {
        //将所有tree节点展开
        ui->treeWidget->expandAll();
    }
}

void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (!ui->listWidget->isVisible())
    {
        isVb = true;
        setListVisible(isVb);
    }

    ui->listWidget->clear();
    ui->listWidget->addItems(xmlnew->getDifferentList(item->text(0)));
}

void MainWindow::setListVisible(bool bl)
{
    ui->listWidget->setVisible(bl);
    ui->pushButton_3->setVisible(bl);

    if (bl)
    {
        int  width = this->size().width()*2;
        this->resize(width, this->size().height());
        ui->pushButton->setText("<<");
    }
    else
    {
        int  width = this->size().width()/2;
        this->resize(width, this->size().height());
        ui->pushButton->setText(">>");
    }
}

void MainWindow::on_action_5_triggered()
{
    /*中误差检查*/

    bool isok;

    double value1 = QInputDialog::getDouble(this, "输入", "平面中误差:", 0, 0, 50, 2, &isok);
    if (!isok)
        return;
    double value2 = QInputDialog::getDouble(this, "输入", "最大中误差:", 0, 0, 50, 2, &isok);
    if (!isok)
        return;

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        return;
    }

    QProgressDialog prDialog("检查元数据中误差填写是否超限...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;
        XmlStreamReader xml;

        QFileInfo file_info(str);

        if (!xml.setFilePath(str))
        {
            cerr_list.append("中误差检查: " + file_info.baseName() + "元数据无法正确打开。");
            isCount = true;
            ++errCount;
            continue;
        }

        QString checkrms = xml.getElementValue("CheckRMS");
        QString checkmaxerr = xml.getElementValue("CheckMAXErr");
        xml.close();

        if (checkrms.isEmpty() || checkmaxerr.isEmpty())
        {
            cerr_list.append("中误差检查: " + file_info.baseName() + "CheckRMS及CheckMAXErr至少一项为空，或字段名称不正确。");
            if (!isCount) { isCount = true; ++errCount; }
            continue;
        }

        double checkrmsf, checkmaxerrf;
        checkrmsf = checkmaxerrf = 0.00;
        checkrmsf = checkrms.toDouble();
        checkmaxerrf = checkmaxerr.toDouble();
        if (!(checkrmsf >=0 && checkrmsf <=value1))
        {
            cerr_list.append("中误差检查: " + file_info.baseName() + "，CheckRMS=" + checkrms + "，填写超限。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!(checkmaxerrf >=0 && checkmaxerrf <=value2))
        {
            cerr_list.append("中误差检查: " + file_info.baseName() + "，CheckMAXErr=" + checkmaxerr + "，填写超限。");
            if (!isCount) { isCount = true; ++errCount; }
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_pushButton_2_clicked()
{
    /*导出错误*/

    QString saveName = QFileDialog::getSaveFileName(this, "导出错误信息", "untitled.txt", "文本文件(*txt)");
    if (saveName.isEmpty())
        return;

    QFile file(saveName);
    if (!file.open(QFile::WriteOnly))
        return;

    QTextStream out(&file);

    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it)
    {
        out << (*it)->text(0) << ", " << (*it)->text(1) << "\n";
        ++it;
    }

    file.close();
    QMessageBox::about(this, "信息", "保存完成.");
}

void MainWindow::on_action_6_triggered()
{
    /**轨道号、影像时间检查*/

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        return;
    }

    QProgressDialog prDialog("检查元数据中轨道号、影像时间填写是否超限...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;

        //保存轨道号、影像时间
        QString orbitCode;
        QString bandDate;
        QString orbitCodeXml;
        QString bandDateXml;

        QFileInfo file_info(str);
        QString fileName = file_info.fileName();
        QString value = fileName.left(3);

        if (value == "GF1" || value=="GF2")
        {
            orbitCode = fileName.mid(3, 7);
            bandDate = fileName.mid(10, 8);
        }
        else if (value == "BJ2")
        {
            orbitCode = fileName.mid(3, 9);
            bandDate = fileName.mid(12, 8);
        }
        else if (value == "ZY3" || value == "ZY1" || value == "TH1")
        {
            orbitCode = fileName.mid(3, 12);
            bandDate = fileName.mid(15, 8);
        }

        XmlStreamReader xml;
        if (!xml.setFilePath(str))
        {
            cerr_list.append("轨道号、影像时间检查: " + fileName + "元数据无法正确打开。");
            isCount = true;
            ++errCount;
            continue;
        }

        orbitCodeXml = xml.getElementValue("PbandOrbitCode");
        bandDateXml = xml.getElementValue("PbandDate");
        if (orbitCodeXml != orbitCode)
        {
            cerr_list.append("轨道号检查    : " + fileName + "，PbandOrbitCode=" + orbitCodeXml + "，应为" + orbitCode);
            if (!isCount) { isCount = true; ++errCount; }
        }
        if ( bandDateXml != bandDate)
        {
            cerr_list.append("影像时间检查: " + fileName + "，PbandDate=" + bandDateXml + "，应为" + bandDate);
            if (!isCount) { isCount = true; ++errCount; }
        }

        orbitCodeXml.clear();   bandDateXml.clear();
        orbitCodeXml = xml.getElementValue("MultiBandOrbitCode");
        bandDateXml = xml.getElementValue("MultiBandDate");
        if (orbitCodeXml != orbitCode)
        {
            cerr_list.append("轨道号检查    : " + fileName + "，MultiBandOrbitCode=" + orbitCodeXml + "，应为" + orbitCode);
            if (!isCount) { isCount = true; ++errCount; }
        }
        if ( bandDateXml != bandDate)
        {
            cerr_list.append("影像时间检查: " + fileName + "，MultiBandDate=" + bandDateXml + "，应为" + bandDate);
            if (!isCount) { isCount = true; ++errCount; }
        }

        xml.close();

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_action_7_triggered()
{
    /*投影检查*/

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        return;
    }

    QProgressDialog prDialog("检查投影一致性...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;

        QString fileName = QFileInfo(str).fileName();

        //保存元数据的中央经线与带号
        QString str_centralMederianXml;
        QString str_zoneNoXml;

        XmlStreamReader xml;
        if (!xml.setFilePath(str))
        {
            cerr_list.append("投影检查: " + fileName + "元数据无法正确打开。");
            isCount = true;
            ++errCount;
            continue;
        }
        str_centralMederianXml = xml.getElementValue("CentralMederian");
        str_zoneNoXml = xml.getElementValue("GaussKrugerZoneNo");
        xml.close();

        //检查元数据中中央经线与带号关系是否正确
        if ((str_zoneNoXml.toInt()*6-3) != str_centralMederianXml.toInt())
        {
            cerr_list.append("投影检查: " + fileName + "， [CentralMederian]与[GaussKrugerZoneNo]关系不正确。");
            if (!isCount) { isCount = true; ++errCount; }
        }

        //检查中央经线与投影文件内容是否正确
        //创建一个用于检查投影文件的字符串
        QString tXml = "PROJCS[\"CGCS_2000_GK_CM_" + str_centralMederianXml +
                                "E\",GEOGCS[\"GCS_China_Geodetic_Coordinate_System_2000\",DATUM[\"D_China_2000\","
                                "SPHEROID[\"CGCS2000\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],"
                                "UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Gauss_Kruger\"],PARAMETER[\"False_Easting\","
                                "500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\"," +
                                str_centralMederianXml + ".0],PARAMETER[\"Scale_Factor\",1.0],"
                                "PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";

        //检查T投影文件内容是否正确
        QFileInfo img_info(str.replace(str.size()-5, 5, "T.XML"));
        if (!xml.setFilePath(img_info.filePath()))
        {
            cerr_list.append("投影检查: " + fileName + "投影文件无法正确打开。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        else
        {
            QString tFile = xml.getElementValue("SRS");
            xml.close();

            //元数据中投影信息与T投影文件是否一致
            if (tXml != tFile)
            {
                cerr_list.append("投影检查: " + fileName + "， 元数据中投影信息与投影文件不符，或投影文件格式不正确。");
                if (!isCount) { isCount = true; ++errCount; }
            }
        }

        //检查元数据中央经线与影像文件投影是否一致
        myOGR org_f(str.replace(str.size()-5, 5, "F.IMG"));
        myOGR org_p(str.replace(str.size()-5, 5, "P.IMG"));
        myOGR org_m(str.replace(str.size()-5, 5, "M.IMG"));
        QString strProjection_f = org_f.getProjection();
        QString strProjection_p = org_p.getProjection();
        QString strProjection_m = org_m.getProjection();
        QString strCgcs2000 = org_f.getEsriCgcs2000Gcs(str_centralMederianXml.toInt());

        if (strCgcs2000 != strProjection_f)
        {
            cerr_list.append("投影检查: " + fileName + "， 融合影像文件定义的投影信息不正确，或影像文件与元数据中央经线不符。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (strCgcs2000 != strProjection_p)
        {
            cerr_list.append("投影检查: " + fileName + "， 全色影像文件定义的投影信息不正确，或影像文件与元数据中央经线不符。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (strCgcs2000 != strProjection_m)
        {
            cerr_list.append("投影检查: " + fileName + "， 多光谱影像文件定义的投影信息不正确，或影像文件与元数据中央经线不符。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        cerr_list.append("投影检查: " + fileName + "， 中央经度为" + str_centralMederianXml);

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_pushButton_clicked()
{
    if (isVb)
    {
        isVb = false;
        setListVisible(isVb);
    }
    else
    {
        isVb = true;
        setListVisible(isVb);
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    /*导出所有筛选结果*/

    QString saveName = QFileDialog::getSaveFileName(this, "导出所有筛选结果", "untitled.txt", "文本文件(*txt)");
    if (saveName.isEmpty())
        return;

    QFile file(saveName);
    if (!file.open(QFile::WriteOnly))
        return;

    QTextStream out(&file);

       QStringList crrList = xmlnew->getDifferentListAll();
       foreach (QString str, crrList)
           out << str << "\n";

    file.close();
    QMessageBox::about(this, "信息", "保存完成.");
}

void MainWindow::on_action_8_triggered()
{
    /*数据量检查*/

    //检查该路径下是否有XML文件
    if (xml_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到XML文件。");
        return;
    }

    QProgressDialog prDialog("检查数据量填写是否正确...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;
        QFileInfo fileinfo(str);
        QString str_ImgSize;

        //保存元数据中大小
        XmlStreamReader xml;
        if (!xml.setFilePath(str))
        {
            cerr_list.append("数据量检查: " + fileinfo.fileName() + "元数据无法正确打开。");
            isCount = true;
            ++errCount;
            continue;
        }
        str_ImgSize = xml.getElementValue("ImgSize");
        xml.close();

        //img影像文件大小
        QString str_fileSize = core::dataAmount(str);

        if (str_ImgSize != str_fileSize)
        {
            cerr_list.append("数据量检查: " + fileinfo.fileName() + "，程序计算大小为:" + str_fileSize + "，元数据填写为:" + str_ImgSize);
            if (!isCount) { isCount = true; ++errCount; }
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_action_9_triggered()
{
    /*元数据生产*/

    //设置影像路径
    filePath = QFileDialog::getExistingDirectory(this, "设置影像路径", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (filePath.isEmpty())
        return;

    //判断路径是否存在
    QDir dir(filePath);
    if(!dir.exists())
    {
        QMessageBox::warning(this, "错误", "路径不存在，检查后重新设置。");
        ui->menu_3->setEnabled(false);
        return;
    }

    yx_list.clear();

    //获取所选文件类型过滤器
    QStringList filters;
    filters << QString("*.img");

    //进度条
    QProgressDialog dialog("搜索影像...", "取消", 0, 0, this);
    dialog.setWindowTitle("搜索影像");                      //设置窗口标题
    dialog.setWindowModality(Qt::WindowModal);      //将对话框设置为模态
    dialog.show();

    //定义迭代器并设置过滤器
    QDirIterator dir_iterator(filePath,filters,
                              QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                              QDirIterator::Subdirectories);
    while(dir_iterator.hasNext())
    {
        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString absolute_file_path = file_info.absoluteFilePath();
        yx_list.append(absolute_file_path);

        //在界面上更新找到的影像
        dialog.setLabelText(file_info.fileName());

        //避免界面冻结
        QApplication::processEvents();

        //当按下取消按钮则中断
        if (dialog.wasCanceled())
        {
            ui->menu_3->setEnabled(false);
            return;
        }
    }

    //检查该路径下是否有影像文件
    if (yx_list.empty())
    {
        QMessageBox::warning(this, "错误", "在指定路径中没找到影像数据。");
        ui->label->setVisible(false);
        ui->menu_3->setEnabled(false);
        return;
    }
    else
    {
        ui->menu_3->setEnabled(true);
        ui->label->setVisible(true);
        ui->label->setText("找到: " + QString::number(yx_list.size(), 10) + "景影像数据");
    }

    //保存精度报告路径
    if (myDialog->getcks().at(9))
    {
        //设置影像路径
        QString jdFilePath = QFileDialog::getExistingDirectory(this, "设置精度报告路径", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!jdFilePath.isEmpty())
        {
            //判断路径是否存在
            QDir dir(jdFilePath);
            if(!dir.exists())
            {
                QMessageBox::warning(this, "错误", "路径不存在，检查后重新设置。");
                return;
            }

            //获取所选文件类型过滤器
            QStringList filters;
            filters << QString("*.txt");

            QDirIterator dir_iterator(jdFilePath,filters,QDir::Files | QDir::NoSymLinks,QDirIterator::Subdirectories);
            while(dir_iterator.hasNext())
            {
                dir_iterator.next();
                QFileInfo file_info = dir_iterator.fileInfo();
                QString absolute_file_path = file_info.absoluteFilePath();
                jd_list.append(absolute_file_path);
            }
        }
    }

    QProgressDialog prDialog("批生产元数据...", "取消", 0, yx_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, yx_list)
    {     
        if (!str.endsWith("F.IMG"))
            continue;

        str = str.mid(0, str.size()-5) + "Y.XML";
        XmlStreamReader xml;
        bool bl = xml.writeXml(str, ui->treeWidget, iscks, jd_list);

        cerr_list.append(xml.getCerrList());
        QFileInfo fileInfo(str);
        if (bl)
            cerr_list.append(fileInfo.fileName()+"------->成功");
        else
        {
            cerr_list.append(fileInfo.fileName()+"------->失败");
            ++errCount;
        }

        //避免界面冻结
        QApplication::processEvents();

        //当按下取消按钮则中断
        prDialog.setValue(++prCount);
        if (prDialog.wasCanceled())
        {
            ui->label->setVisible(false);
            ui->menu_3->setEnabled(false);
            return;
        }
    }

    //输出检查个数、错误信息
    ui->label->setText(QString("批量制作完成: %1景影像正确.").arg(yx_list.size() - errCount));
    ui->label_2->setText(QString("　　　　　　  %1景出现错误.").arg(errCount));
    ui->label_2->setVisible(true);

    ui->treeWidget->clear();

    //设置treeWidget表头
    QStringList strList;
    strList << "序号" << "错误信息";
    ui->treeWidget->setHeaderLabels(strList);

    long count = 0;
    foreach (QString str, cerr_list)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString::number(++count));
        item->setText(1, str);
    }

    //将错误列表清空
    cerr_list.clear();
}

void MainWindow::on_action_10_triggered()
{
    /*元数据制作设置*/
    myDialog->exec();
    iscks=myDialog->getcks();
}

void MainWindow::on_action_11_triggered()
{
    /*更新数据量*/

    QProgressDialog prDialog("更新影像数据量...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;

        // img影像文件大小
        QString str_ImgSize = core::dataAmount(str);

        // 写入数据量
        if (!startEdit(str))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据无法正确打开。");
            isCount = true;
            ++errCount;
            continue;
        }
        if (!replaceVol("ImgSize", str_ImgSize))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到ImgSize字段。");
            if (!isCount) { isCount = true; ++errCount; }
            continue;
        }
        if (!endEdit(str))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "重新写入元数据内容失败。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        else
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + " -->成功");
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_action_13_triggered()
{
    /*批量修改字段内容*/

    bool ok1;
    bool ok2;
    QString fieldName = QInputDialog::getText(this, "批量修改字段内容",
                                         "字段名称:", QLineEdit::Normal,"", &ok1);
    if (!ok1 || fieldName.isEmpty())
        return;
    QString fieldValue = QInputDialog::getText(this, "批量修改字段内容",
                                         "字段内容:", QLineEdit::Normal,"", &ok2);
    if (!ok2 || fieldValue.isEmpty())
        return;

    QProgressDialog prDialog(QString("批量修改%1字段内容...").arg(fieldName), "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;

        // 写入修改内容
        if (!startEdit(str))
        {
            cerr_list.append("批量修改字段内容: " + QFileInfo(str).fileName() + "元数据无法正确打开。");
            isCount = true;
            ++errCount;
            continue;
        }
        if (!replaceVol(fieldName, fieldValue))
        {
            cerr_list.append("批量修改字段内容: " + QFileInfo(str).fileName() + QString("元数据中未找到%1字段。").arg(fieldName));
            if (!isCount) { isCount = true; ++errCount; }
            continue;
        }
        if (!endEdit(str))
        {
            cerr_list.append("批量修改字段内容: " + QFileInfo(str).fileName() + "重新写入元数据内容失败。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        else
        {
            cerr_list.append("批量修改字段内容: " + QFileInfo(str).fileName() + QString(" %1-->成功").arg(fieldName));
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_action_12_triggered()
{
    /*更新影像角点坐标*/

    QProgressDialog prDialog("更新影像角点坐标...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;

        QFileInfo file_info(str);
        QString strOgr = str;
        myOGR ogr(strOgr.replace(strOgr.size()-5, 5, "P.IMG"));
        if (!ogr.isOpen())
        {
            cerr_list.append("坐标检查: " + file_info.baseName() + "读取影像数据失败。");
            isCount = true;
            ++errCount;
            continue;
        }

        if (!startEdit(str))
        {
            cerr_list.append("更新影像角点坐标: " + QFileInfo(str).fileName() + "元数据无法正确打开。");
            if (!isCount) { isCount = true; ++errCount; }
            continue;
        }

        QStringList xy_list = ogr.getXY();

        if (!replaceVol("SouthWestAbs", xy_list.at(0)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到SouthWestAbs字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!replaceVol("SouthWestOrd", xy_list.at(3)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到SouthWestOrd字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!replaceVol("NorthWestAbs", xy_list.at(0)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到NorthWestAbs字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!replaceVol("NorthWestOrd", xy_list.at(1)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到NorthWestOrd字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!replaceVol("NorthEastAbs", xy_list.at(2)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到NorthEastAbs字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!replaceVol("NorthEastOrd", xy_list.at(1)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到NorthEastOrd字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!replaceVol("SouthEastAbs", xy_list.at(2)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到SouthEastAbs字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        if (!replaceVol("SouthEastOrd", xy_list.at(3)))
        {
            cerr_list.append("数据量更新: " + QFileInfo(str).fileName() + "元数据中未找到SouthEastOrd字段。");
            if (!isCount) { isCount = true; ++errCount; }
        }

        if (!endEdit(str))
        {
            cerr_list.append("更新影像角点坐标: " + QFileInfo(str).fileName() + "重新写入元数据内容失败。");
            if (!isCount) { isCount = true; ++errCount; }
        }
        else
        {
            cerr_list.append("更新影像角点坐标: " + QFileInfo(str).fileName() + " -->成功");
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}

void MainWindow::on_action_17_triggered()
{
    /*成果影像分辨率更新*/

    ui->treeWidget->clear();
    cerr_list.clear();

    //设置treeWidget表头
    QStringList strList;
    strList << "字段" << "内容";
    ui->treeWidget->setHeaderLabels(strList);

    QProgressDialog prDialog("更新成果影像分辨率...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;

    foreach (QString str, xml_list)
    {
        // 计算像素位数
        myOGR *ogrF = 0;
        ogrF = new myOGR(str.left(str.size()-5) + "F.IMG");
        if (!ogrF->isOpen())
        {
            cerr_list.append(QFileInfo(str).fileName() + ",对应融合影像数据无法打开，已跳过元数据该项更新。");
            continue;
        }
        QString strPixelSize = ogrF->getPixelSize();
        delete ogrF; ogrF = 0;

        if (strPixelSize.isEmpty())
        {
            cerr_list.append("更新成果影像分辨率: " + QFileInfo(str).fileName() + "融合影像分辨率长、宽不一致，该项填写失败。");
            continue;
        }

        // 写入像素位数
        if (!startEdit(str))
        {
            cerr_list.append("更新成果影像分辨率: " + QFileInfo(str).fileName() + "元数据无法正确打开。");
            continue;
        }
        if (!replaceVol("GroundResolution", strPixelSize))
        {
            cerr_list.append("更新成果影像分辨率: " + QFileInfo(str).fileName() + "元数据中未找到GroundResolution字段。");
            continue;
        }
        if (!endEdit(str))
        {
            cerr_list.append("更新成果影像分辨率: " + QFileInfo(str).fileName() + "重新写入元数据内容失败。");
        }
        else
        {
            cerr_list.append("更新成果影像分辨率: " + QFileInfo(str).fileName() + " -->成功");
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出错误信息
    long count = 0;
    foreach (QString str, cerr_list)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString::number(++count));
        item->setText(1, str);
    }
}

void MainWindow::on_action_18_triggered()
{
    /*元数据结构检查*/

    // 选择元数据模板文件
    QString templateName = QFileDialog::getOpenFileName(this, "选择元数据模板", ".", "XML (*.xml)");
    if ( templateName.isEmpty() )
    {
        return;
    }

    // 获取模板结构
    QStringList templateList = xmlnew->getKeyList(templateName);
    if (templateList.isEmpty())
    {
        QMessageBox::warning(this, "错误", "从元数据模板文件中读取结构失败，\n请重新检查模板文件，已终止运行。");
        return;
    }

    QProgressDialog prDialog("元数据结构检查...", "取消", 0, xml_list.size(), this);
    prDialog.setWindowTitle("处理进度");			//设置窗口标题
    prDialog.setWindowModality(Qt::WindowModal);	//将对话框设置为模态
    prDialog.show();
    int prCount = 0;
    int errCount = 0;

    foreach (QString str, xml_list)
    {
        bool isCount = false;
        QString xmlName = QFileInfo(str).fileName();

        QStringList strList = xmlnew->getKeyList(str);
        if ( strList.isEmpty() )
        {
            cerr_list.append("元数据结构检查: " + xmlName + "读取失败。");
            isCount = true;
            ++errCount;
            continue;
        }

        for (int i = 0; i < templateList.size(); ++i)
        {
            if (!strList.contains(templateList.at(i)))
            {
                cerr_list.append("元数据结构检查: " + xmlName + QString("元数据中缺少节点：[%1]。").arg(templateList.at(i)));
                if (!isCount) { isCount = true; ++errCount; }
            }
        }
        for (int i = 0; i < strList.size(); ++i)
        {
            if (!templateList.contains(strList.at(i)))
            {
                cerr_list.append("元数据结构检查: " + xmlName + QString("元数据多余节点：[%1]。").arg(strList.at(i)));
                if (!isCount) { isCount = true; ++errCount; }
            }
        }
        if (!isCount)
        {
            for (int i = 0; i < templateList.size(); ++i)
            {
                if ((templateList.at(i)).compare(strList.at(i)) != 0)
                {
                    cerr_list.append("元数据结构检查: " + xmlName + QString("[%1]节点顺序与模板不一致。").arg(strList.at(i)));
                    if (!isCount) { isCount = true; ++errCount; }
                    break;
                }
            }
        }

        prDialog.setValue(++prCount);
        QApplication::processEvents();				//避免界面冻结
        if (prDialog.wasCanceled())					//当按下取消按钮则中断
        {
            cerr_list.clear();
            return;
        }
    }

    //输出检查个数、错误信息
    printInfo(xml_list.size(), errCount, cerr_list);
}


























