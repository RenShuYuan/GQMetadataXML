#ifndef MYOGR_H
#define MYOGR_H
#include <QList>

#include <gdal_priv.h>
#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"

class QString;
class QPointF;
class QStringList;
class OGRSpatialReference;

class myOGR
{
public:
    myOGR(const QString &fileName);
    ~myOGR();

    bool open(const QString &fileName);
    bool isOpen();

    //返回指定影像的角点坐标
    QStringList getXY();

    //返回指定影像的投影信息
    QString getProjection();

    //返回影像像元大小
    QString getPixelSize();

    //返回影像位深
    int getDataType();

    //根据地理坐标系定义创建投影坐标系列表
    void definitionGcs(const QString gcsEPSG);

    //返回指定的地理坐标系名称
    const QString getGCSDatum(const QString gcsEPSG);

    //返回对应的投影坐标系参数
    QStringList getIndexProhParm(const int index);

    //返回地理坐标系对应的投影坐标系名称列表
    QStringList getAllPCSProjcs();

    //输入中央经线，返回ESRI格式的CGCS2000高斯投影字符串
    QString getEsriCgcs2000Gcs(const int cm);

    //输入中央经线，返回ERDAS格式的CGCS2000高斯投影字符串
    QString getErdasCgcs2000Gcs(const int cm);

private:
    GDALDataset* poDataset;
    QList<OGRSpatialReference> list_pcs;
};

#endif // MYOGR_H
