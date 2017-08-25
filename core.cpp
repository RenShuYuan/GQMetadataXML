#include "core.h"

#include <QString>
#include <QFileInfo>
#include <QtGlobal>

QString core::dataAmount(const QString &xmlPath)
{
    //保存影像文件大小
    qint64 qint = 0;
    QString str_fileSize;
    double f_fileSize = 0.0;

    //全色
    QString imgNamep = xmlPath.left(xmlPath.size()-5) + "P.IMG";
    QString igeNamep = xmlPath.left(xmlPath.size()-5) + "P.IGE";
    QFileInfo imginfop(imgNamep);
    QFileInfo igeinfop(igeNamep);
    if (imginfop.exists())
        qint = imginfop.size();
    if (igeinfop.exists())
        qint += igeinfop.size();

    //多光谱
    QString imgNamem = xmlPath.left(xmlPath.size()-5) + "M.IMG";
    QString igeNamem = xmlPath.left(xmlPath.size()-5) + "M.IGE";
    QFileInfo imginfom(imgNamem);
    QFileInfo igeinfom(igeNamem);
    if (imginfom.exists())
        qint += imginfom.size();
    if (igeinfom.exists())
        qint += igeinfom.size();

    //融合
    QString imgNamef = xmlPath.left(xmlPath.size()-5) + "F.IMG";
    QString igeNamef = xmlPath.left(xmlPath.size()-5) + "F.IGE";
    QFileInfo imginfof(imgNamef);
    QFileInfo igeinfof(igeNamef);
    if (imginfof.exists())
        qint += imginfof.size();
    if (igeinfof.exists())
        qint += igeinfof.size();

    if (qint > 1073741824 || qint < 104857600)
    {
        f_fileSize = (double)qint / 1024 / 1024;
        if (qint > 1073741824)
            str_fileSize = QString::number(f_fileSize, 'f', 1);
        else
        {
            str_fileSize = QString::number(f_fileSize, 'f', 6);
            str_fileSize = str_fileSize.left(str_fileSize.size()-5);
        }
    }
    else
    {
        f_fileSize = qint / 1024 / 1024;
        str_fileSize = QString::number(f_fileSize, 'f', 1);
    }

    return str_fileSize;
}
