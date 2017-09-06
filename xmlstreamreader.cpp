#include "xmlstreamreader.h"
#include "myogr.h"
#include "core.h"

#include <QXmlStreamWriter>
#include <QTreeWidget>
#include <QFileInfo>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QList>
#include <QChar>
#include <QHash>

XmlStreamReader::XmlStreamReader()
{

}

XmlStreamReader::XmlStreamReader(QTreeWidget *tree)
{
    treeWidget = tree;
}

bool XmlStreamReader::readFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "不能读取元数据.";
        return false;
    }

    reader.setDevice(&file);
    readMetadataindexElement();

    file.close();
    if (reader.hasError())
    {
        QString str_err;
        str_err = "Error: Failed to parse file " + fileName + ": " + reader.errorString();
        qDebug() << str_err;
        return false;
    }
    else if (file.error() != QFile::NoError)
    {
        qDebug() << "不能读取元数据.";
        return false;
    }
    return true;
}

bool XmlStreamReader::readFiles(const QStringList &pathList)
{
    if (!readFile(pathList.first()))
        return false;

    foreach (QString fileName, pathList)
    {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            qDebug() << "不能读取元数据.";
            return false;
        }

        QFileInfo file_info(fileName);
        QString subfileName = file_info.fileName();

        reader.setDevice(&file);
        reader.readNext();
        while (!reader.atEnd())
        {
            if (reader.isStartElement())
            {
                QString name = reader.name().toString();
                if (!(name == "Metadatafile" || name == "BasicDataContent" || name == "ImgRange" ||
                      name == "MathFoundation" || name == "ImgSource" || name == "PanBand" ||
                      name == "MultiBand" || name == "ProduceInfomation" || name == "ImgOrientation" ||
                      name == "MosaicInfo" || name == "QualityCheckInfo"))
                {
                    if (map.contains(name))
                    {
                        QMap<QString,CollectionItems>::iterator it = map.find(name);
                        CollectionItems ct = it.value();
                        ct.addVol(subfileName, reader.readElementText());
                        map[name] = ct;
                    }
                    else
                    {
                        CollectionItems ct;
                        ct.addVol(subfileName, reader.readElementText());
                        map[name] = ct;
                    }
                }
                else
                    reader.readNext();
            }
            else
                reader.readNext();
        }
        file.close();
    }
    setTree();

    return true;
}

void XmlStreamReader::setTree()
{
    QMapIterator<QString, CollectionItems> it(map);
    while (it.hasNext())
    {
        it.next();
        QString itemName = it.key();
        QList<QTreeWidgetItem*> itemVec = treeWidget->findItems(itemName, Qt::MatchCaseSensitive | Qt::MatchRecursive);
        if (itemVec.size() != 1)
            continue;

        CollectionItems ct = it.value();
        const QString fistVol = ct.getFistValue();
        if (!(ct.isConsistency(fistVol)))
            itemVec.at(0)->setText(1, "");
    }
}

QStringList XmlStreamReader::getKeyList(const QString &fileName)
{
    QStringList keyList;
    QXmlStreamReader mReader;

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "不能读取元数据.";
        return keyList;
    }

    mReader.setDevice(&file);
    mReader.readNext();
    while (!mReader.atEnd())
    {
        if (mReader.isStartDocument())
        {
            QStringRef code = mReader.documentEncoding();
            QStringRef version = mReader.documentVersion();
            if ((!code.isEmpty()) || (!version.isEmpty()))
            {
                keyList.append("<?xml version=\"1.0\"?>");
            }
        }
        else if (mReader.isStartElement())
        {
            QString keyName = mReader.name().toString();
            keyList.append(keyName);
        }
        else if (mReader.isEndElement())
        {
            QString keyName = mReader.name().toString();
            keyList.append("/"+keyName);
        }
        mReader.readNext();
    }
    file.close();
    return keyList;
}

QStringList XmlStreamReader::getDifferentList(const QString &key)
{
    if (map.contains(key))
    {
        CollectionItems ct = map[key];
        const QString fistVol = ct.getFistValue();
        return map[key].getDifferent(key, fistVol);
    }
    else
    {
        QStringList list;
        return list;
    }
}

QStringList XmlStreamReader::getDifferentListAll()
{
    QStringList list;
    QMapIterator<QString, CollectionItems> i(map);
    while (i.hasNext())
    {
        i.next();
        CollectionItems ct = i.value();
        const QString fistVol = ct.getFistValue();
        list += ct.getDifferent(i.key(), fistVol);
        list.append("\n");
    }
    return list;
}

bool XmlStreamReader::isMetaDataFileName(const QString &name)
{
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isStartElement())
        {
            QString str = reader.name().toString();
            if (str == "MetaDataFileName")
            {
                if (reader.readElementText() == name)
                {
                    return true;
                }
                else
                    return false;
            }
            else
                reader.readNext();
        }
        else
            reader.readNext();
    }
    return false;
}

bool XmlStreamReader::setFilePath(const QString &path)
{
    files = new QFile(path);
    if (!files->open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "不能读取元数据.";
        delete files; files = 0;
        return false;
    }

    reader.setDevice(files);
    return true;
}

void XmlStreamReader::close()
{
    files->close();
}

QString XmlStreamReader::getElementValue(const QString &strElement)
{
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isStartElement())
        {
            QString str = reader.name().toString();
            if (str == strElement)
                return reader.readElementText();
            else
                reader.readNext();
        }
        else
            reader.readNext();
    }
    return "";
}

bool XmlStreamReader::writeXml(const QString &fileName, QTreeWidget *tree, const QList<bool> iscks, const QStringList &jd_list)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return false;

    filePath = fileName;
    QXmlStreamWriter xmlWrite(&file);
    xmlWrite.setAutoFormatting(true);
    xmlWrite.writeStartDocument();
    bool bl=false;
    for (int i=0; i<tree->topLevelItemCount(); ++i)
        bl=writeIndexEntry(&xmlWrite, tree->topLevelItem(i), iscks, jd_list);
    xmlWrite.writeEndDocument();
    file.close();
    return bl;
}

QStringList XmlStreamReader::getCerrList()
{
    return cerrList;
}

void XmlStreamReader::readMetadataindexElement()
{
    reader.readNext();
    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }
        if (reader.isStartElement())
            readEntryElement(treeWidget->invisibleRootItem());
        else
            reader.readNext();
    }
}

void XmlStreamReader::readEntryElement(QTreeWidgetItem *parent)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    QString name = reader.name().toString();
    item->setText(0, name);
    if (!(name == "Metadatafile" || name == "BasicDataContent" || name == "ImgRange" ||
          name == "MathFoundation" || name == "ImgSource" || name == "PanBand" ||
          name == "MultiBand" || name == "ProduceInfomation" || name == "ImgOrientation" ||
          name == "MosaicInfo" || name == "QualityCheckInfo"))
    {
        QString vol = reader.readElementText();
        item->setText(1, vol);
    }
    else
        reader.readNext();

    while (!reader.atEnd())
    {
        if (reader.isEndElement())
        {
            reader.readNext();
            break;
        }
        if (reader.isStartElement())
            readEntryElement(item);
        else
            reader.readNext();
    }
}

bool XmlStreamReader::writeIndexEntry(QXmlStreamWriter *xmlWriter, QTreeWidgetItem *item, const QList<bool> iscks, const QStringList &jd_list)
{

    //元数据名称,不包含路径
    QFileInfo fileInfo(filePath);
    QString xmlName = fileInfo.fileName();

    myOGR *ogrP=0;
    myOGR *ogrM=0;
    myOGR *ogrF=0;

    ogrP = new myOGR(filePath.left(filePath.size()-5) + "P.IMG");
    if (!ogrP->isOpen())
    {
        cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,全色影像数据无法打开，该元数据制作失败。");
        return false;
    }
    ogrM = new myOGR(filePath.left(filePath.size()-5) + "M.IMG");
    if (!ogrM->isOpen())
    {
        cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,多光谱影像数据无法打开，该元数据制作失败。");
        delete ogrP; ogrP = 0;
        return false;
    }
    ogrF = new myOGR(filePath.left(filePath.size()-5) + "F.IMG");
    if (!ogrF->isOpen())
    {
        cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,融合影像数据无法打开，该元数据制作失败。");
        delete ogrP; ogrP = 0;
        delete ogrM; ogrM = 0;
        return false;
    }


    QString key   = item->text(0);
    QString value = item->text(1);
    if (value.isEmpty())
        xmlWriter->writeStartElement(key);
    else
    {        
        if (key=="MetaDataFileName")
        {   //文件名
            if (iscks.at(0))
                xmlWriter->writeTextElement(key, xmlName);
        }
        else if (key=="GroundResolution")
        {   //地面分辨率
            QString pixelSize = ogrF->getPixelSize();
            if (pixelSize.isEmpty())
            {
                cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,融合影像分辨率长、宽不一致，该项填写失败。");
            }
            else
            {
                xmlWriter->writeTextElement(key, pixelSize);
            }
        }
        else if (key=="SateResolution" || key=="MultiBandResolution")
        {   //地面分辨率
            if (iscks.at(1))
            {
                QString sizeP;
                QString sizeM;

                // 根据不同影像源设置对应的原始分辨率
                if (xmlName.left(3) == "BJ2" || xmlName.left(3) == "GF2")
                {
                    sizeP = "0.8";
                    sizeM = "3.2";
                }
                else if (xmlName.left(3) == "ZY1" || xmlName.left(3) == "TH1")
                {
                    sizeP = "2.0";
                    sizeM = "10.0";
                }
                else if (xmlName.left(3) == "ZY3")
                {
                    sizeP = "2.1";
                    sizeM = "5.8";
                }
                else if (xmlName.left(3) == "GF1")
                {
                    sizeP = "2.0";
                    sizeM = "7.7";
                }

                // 检查是否支持该影像源
                if (sizeP.isEmpty() || sizeM.isEmpty())
                {
                    cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,程序暂不支持该影像源，请联系项目负责人增加，未输出正确内容。");
                }
                else
                {
                    if (key=="GroundResolution" || key=="SateResolution")   // 全是分辨率
                    {
                        xmlWriter->writeTextElement(key, sizeP);
                    }
                    else if (key=="MultiBandResolution")    // 多光谱分辨率
                    {
                        xmlWriter->writeTextElement(key, sizeM);
                    }
                }

            }
        }
        else if (key=="PixelBits")
        {
            if (iscks.at(2))    //影像位深
            {
                xmlWriter->writeTextElement(key, QString::number(ogrM->getDataType()*4));
            }
        }
        else if (key=="ImgSize")
        {
            if (iscks.at(3))    //数据量
            {
                //img影像文件大小
                QString str_fileSize = core::dataAmount(filePath);

                xmlWriter->writeTextElement(key, str_fileSize);
            }
        }
        else if (key=="SouthWestAbs" || key=="SouthWestOrd" || key=="NorthWestAbs" || key=="NorthWestOrd"
                 || key=="NorthEastAbs" || key=="NorthEastOrd" || key=="SouthEastAbs" || key=="SouthEastOrd")
        {
            if (iscks.at(4))    //坐标
            {
                QStringList xyList;
                xyList = ogrP->getXY();

                if (key == "SouthWestAbs" || key == "NorthWestAbs")
                    xmlWriter->writeTextElement(key, xyList.at(0));
                else if (key == "SouthWestOrd" || key == "SouthEastOrd")
                    xmlWriter->writeTextElement(key, xyList.at(3));
                else if (key == "NorthWestOrd" || key == "NorthEastOrd")
                    xmlWriter->writeTextElement(key, xyList.at(1));
                else if (key == "NorthEastAbs" || key == "SouthEastAbs")
                    xmlWriter->writeTextElement(key, xyList.at(2));
            }
        }
        else if (key=="CentralMederian" || key=="GaussKrugerZoneNo")
        {
            if (iscks.at(5))    //投影信息
            {
                OGRSpatialReference oSRS;
                QString projection;
                projection = ogrF->getProjection();

                if (projection.isEmpty())
                    cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,影像数据未定义投影，投影信息填写失败。");
                else
                {
                    QByteArray ba = projection.toLocal8Bit();
                    oSRS.SetFromUserInput(ba.data());
                    double d_centralMeridian = oSRS.GetProjParm(SRS_PP_CENTRAL_MERIDIAN);
                    if (key=="CentralMederian")
                        xmlWriter->writeTextElement(key, QString::number(d_centralMeridian));
                    else if (key=="GaussKrugerZoneNo")
                        xmlWriter->writeTextElement(key, QString::number((int)d_centralMeridian/6 +1));
                }
            }
        }
        else if (key=="SateName")
        {
            if (iscks.at(6))    //影像源代码
            {
                QString sateName = xmlName.left(3);
                sateName.insert(2, '-');
                xmlWriter->writeTextElement(key, sateName);
            }
        }
        else if (key=="PbandOrbitCode" || key=="MultiBandOrbitCode")
        {
            if (iscks.at(7))
            {
                QString orbitCode;
                if ( xmlName.left(3)== "GF1" || xmlName.left(3)=="GF2")
                {
                    orbitCode = xmlName.mid(3, 7);
                    xmlWriter->writeTextElement(key, orbitCode);
                }
                else if (xmlName.left(3)== "BJ2")
                {
                    orbitCode = xmlName.mid(3, 9);
                    xmlWriter->writeTextElement(key, orbitCode);
                }
                else if (xmlName.left(3) == "ZY3" || xmlName.left(3) == "ZY1" ||xmlName.left(3) == "TH1")
                {
                    orbitCode = xmlName.mid(3, 12);
                    xmlWriter->writeTextElement(key, orbitCode);
                }
            }
        }
        else if (key=="PbandDate" || key=="MultiBandDate")
        {
            if (iscks.at(8))    //影像时间
            {
                if (key=="PbandDate" || key=="MultiBandDate")
                {
                    QString bandDate;
                    if ( xmlName.left(3)== "GF1" || xmlName.left(3)=="GF2")
                    {
                        bandDate = xmlName.mid(10, 8);
                        xmlWriter->writeTextElement(key, bandDate);
                    }
                    else if (xmlName.left(3)== "BJ2")
                    {
                        bandDate = xmlName.mid(12, 8);
                        xmlWriter->writeTextElement(key, bandDate);
                    }
                    else if (xmlName.left(3) == "ZY3" || xmlName.left(3) == "ZY1" ||xmlName.left(3) == "TH1")
                    {
                        bandDate = xmlName.mid(15, 8);
                        xmlWriter->writeTextElement(key, bandDate);
                    }
                }
            }
        }
        else if (key=="CheckPointNum" || key=="CheckRMS" || key=="CheckMAXErr")
        {
            if (iscks.at(9))    //影像精度
            {
                QFileInfo fileInfo(filePath);
                QString baseName = fileInfo.baseName();
                baseName = baseName.left(baseName.size()-1);
                QString jdName;
                foreach (QString str, jd_list)
                {
                    QFileInfo file_info(str);
                    if (file_info.fileName().startsWith(baseName, Qt::CaseInsensitive))
                    {
                        jdName = str;
                        break;
                    }
                }

                QFile file(jdName);
                if (!file.open(QFile::ReadOnly | QFile::Text))
                    cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,未找到对应的精度报告或不能打开。");
                else
                {
                    QTextStream in(&file);
                    while (!in.atEnd())
                    {
                        QString line = in.readLine();
                        if (key=="CheckPointNum")
                        {
                            if (line.contains("总点数="))
                            {
                                QStringList list = line.split('=');
                                if (list.size() < 2)
                                    cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,读取精度报告中检查点个数失败,检查格式是否正确。");
                                else
                                {
                                    QStringList list1 = list.at(1).split(' ');
                                    xmlWriter->writeTextElement(key, list1.at(0));
                                    break;
                                }
                            }
                        }
                        else if (key=="CheckRMS")
                        {
                            if (line.contains("Mx="))
                            {
                                QStringList list = line.split('=');
                                if (list.size() < 2)
                                    cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,读取精度报告中平均中误差失败,检查格式是否正确。");
                                else
                                {
                                    QStringList list1 = list.at(1).split(' ');
                                    QString str = list1.at(0);
                                    str = str.mid(1, str.size());
                                    double mx = str.toDouble();
                                    xmlWriter->writeTextElement(key, QString::number(mx, 'f', 2));
                                    break;
                                }
                            }
                        }

                        else if (key=="CheckMAXErr")
                        {
                            if (line.contains("最大误差为："))
                            {
                                QStringList list = line.split("：");
                                if (list.size() < 2)
                                    cerrList.append(xmlName.left(xmlName.size()-3) + "IMG,读取精度报告中最大中误差失败,检查格式是否正确。");
                                else
                                {
                                    QStringList list1 = list.at(1).split(' ');
                                    double mx = list1.at(0).toDouble();
                                    xmlWriter->writeTextElement(key, QString::number(mx, 'f', 2));
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
            xmlWriter->writeTextElement(key, value);

        delete ogrP; ogrP = 0;
        delete ogrM; ogrM = 0;
        delete ogrF; ogrF = 0;

        return true;
    }

    delete ogrP; ogrP = 0;
    delete ogrM; ogrM = 0;
    delete ogrF; ogrF = 0;

    for (int i=0; i<item->childCount(); ++i)
        writeIndexEntry(xmlWriter, item->child(i), iscks, jd_list);

    xmlWriter->writeEndElement();
    return true;
}

CollectionItems::CollectionItems()
{

}

void CollectionItems::addVol(const QString &strKey, const QString &strVol)
{
    keyList.append(strKey);
    volList.append(strVol);
}

bool CollectionItems::isConsistency(const QString &str)
{
    foreach (QString tmp, volList)
    {
        if (str != tmp)
            return false;
    }
    return true;
}

QStringList CollectionItems::getDifferent(const QString &key, const QString &str)
{
    QStringList differentList;

    for (int i=0; i<volList.size(); ++i)
    {
        QString tmp = volList.at(i);
        if (str != tmp)
            differentList.append(key + ": " + tmp + "   ---->" + keyList.at(i));
    }
    return differentList;
}

QString CollectionItems::getFistValue()
{
    return volList.first();
}
