#include "myogr.h"

#include <string>
#include <QString>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QDirIterator>
#include <QDebug>
#include <QPointF>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QtGlobal>

using namespace std;

myOGR::myOGR(const QString &fileName)
{
    //    添加环境变量
    CPLSetConfigOption("GDAL_DATA", "C:\\gdal201\\data");

    //支持中文路径
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

    GDALAllRegister();  //注册所有影像格式
    QByteArray ba = fileName.toLocal8Bit();
    const char* pszFile= ba.data();

    //已只读方式打开影像文件
    poDataset=(GDALDataset*)GDALOpen(pszFile, GA_ReadOnly);
}

myOGR::~myOGR()
{
    if (poDataset!=NULL)
        GDALClose((GDALDatasetH)poDataset);
}

bool myOGR::open(const QString &fileName)
{
    GDALAllRegister();  //注册所有影像格式
    QByteArray ba = fileName.toLocal8Bit();
    const char* pszFile= ba.data();

    //已只读方式打开影像文件
    poDataset=(GDALDataset*)GDALOpen(pszFile, GA_ReadOnly);
    if (poDataset==NULL)
        return false;
    else
        return true;
}

bool myOGR::isOpen()
{
    if (poDataset==NULL)
        return false;
    else
        return true;
}

QStringList myOGR::getXY()
{
    QStringList xyList;
    if (poDataset==NULL)
        return xyList;

    //获得影像像素大小
    GDALRasterBand *poBand_1 = poDataset->GetRasterBand(1);
    int xSize = poBand_1->GetXSize();
    int ySize = poBand_1->GetYSize();

    double pro[6];
    poDataset->GetGeoTransform(pro);

    double xy[4];
    double midPixel = pro[1] / 2;
    xy[0] = pro[0] + midPixel;
    xy[1] = pro[3] - midPixel;
    xy[2] = pro[0] + pro[1] * xSize - midPixel;
    xy[3] = pro[3] + pro[5] * ySize + midPixel;

    QString str_out_xy[4];
    str_out_xy[0] = QString::number(xy[0], 10, 2);
    str_out_xy[1] = QString::number(xy[1], 10, 2);
    str_out_xy[2] = QString::number(xy[2], 10, 2);
    str_out_xy[3] = QString::number(xy[3], 10, 2);

    xyList <<  str_out_xy[0] <<  str_out_xy[1] <<  str_out_xy[2] <<  str_out_xy[3];

    return xyList;
}

QString myOGR::getProjection()
{
    if (poDataset==NULL)
        return "";

    QString projectionName = poDataset->GetProjectionRef();
    return projectionName;
}

QString myOGR::getPixelSize()
{
    double pro[6];
    poDataset->GetGeoTransform(pro);

    QString sizeX = QString::number(pro[1], 'f', 6);
    QString sizeY = QString::number(pro[5], 'f', 6);

    if (sizeX.at(0)=='-')
        sizeX = sizeX.mid(1, sizeX.length());
    if (sizeY.at(0)=='-')
        sizeY = sizeY.mid(1, sizeY.length());

    if (sizeX == sizeY)
        return sizeX.left(3);
    else
        return "";
}

int myOGR::getDataType()
{
    if (poDataset->GetRasterCount() == 0)
        return 0;
    GDALRasterBand *poBand_1 = poDataset->GetRasterBand(1);
    GDALDataType datatype= poBand_1->GetRasterDataType();

    if (datatype==::GDT_Byte)   return 8;
    else if (datatype==::GDT_UInt16 )   return 16;
    else if (datatype==::GDT_Int16 )   return 16;
    else if (datatype==::GDT_UInt32 )   return 32;
    else if (datatype==::GDT_Int32 )   return 32;
    else    return 0;
}

void myOGR::definitionGcs(const QString gcsEPSG)
{
    list_pcs.clear();

    if (gcsEPSG == "EPSG:4490")
    {
        for (int i=75; i<136; i+=3)
        {
            OGRSpatialReference oSRS;
            QByteArray ba = (getEsriCgcs2000Gcs(i)).toLocal8Bit();
            oSRS.SetFromUserInput(ba.data());
            list_pcs << oSRS;
        }
        for (int i=25; i<46; ++i)
        {
            OGRSpatialReference oSRS;
            QByteArray ba = (getEsriCgcs2000Gcs(i)).toLocal8Bit();
            oSRS.SetFromUserInput(ba.data());
            list_pcs << oSRS;
        }
        for (int i=13; i<24; ++i)
        {
            OGRSpatialReference oSRS;
            QByteArray ba = (getEsriCgcs2000Gcs(i)).toLocal8Bit();
            oSRS.SetFromUserInput(ba.data());
            list_pcs << oSRS;
        }
    }
}

const QString myOGR::getGCSDatum(const QString gcsEPSG)
{

    OGRSpatialReference oSRS;

    QByteArray ba = gcsEPSG.toLocal8Bit();
    oSRS.SetWellKnownGeogCS(ba.data());

    const QString gcsDatum = oSRS.GetAttrValue("DATUM");

    return gcsDatum;
}

QStringList myOGR::getIndexProhParm(const int index)
{
    QStringList list_ProjParm;
    OGRSpatialReference oSRS;
    oSRS = list_pcs.at(index);

    double d_falseEasting = oSRS.GetProjParm(SRS_PP_FALSE_EASTING);
    double d_falseNorthing = oSRS.GetProjParm(SRS_PP_FALSE_NORTHING);
    double d_centralMeridian = oSRS.GetProjParm(SRS_PP_CENTRAL_MERIDIAN);
    double d_scaleFactor = oSRS.GetProjParm(SRS_PP_SCALE_FACTOR);

    list_ProjParm << QString::number(d_falseEasting, 10, 1);
    list_ProjParm << QString::number(d_falseNorthing, 10, 1);
    list_ProjParm << QString::number(d_centralMeridian, 10, 1);
    list_ProjParm << QString::number(d_scaleFactor, 10, 1);

    return list_ProjParm;
}

QStringList myOGR::getAllPCSProjcs()
{
    QStringList list;

    foreach (OGRSpatialReference oSRS, list_pcs)
        list << oSRS.GetAttrValue("PROJCS");

    return list;
}

QString myOGR::getEsriCgcs2000Gcs(const int cm)
{
//    QString pszCGCS_2000_1;
//    QString pszCGCS_2000_2;
//    QString pszCGCS_2000_3;
//    QString pszCGCS_2000_4 = "PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"meters\",1]]";

//    //3度分带，不加带号
//    if (cm>74 && cm<136)
//    {
//        pszCGCS_2000_1 = QString("PROJCS[\"CGCS_2000_GK_CM_%1E\",").arg(cm);
//        pszCGCS_2000_2 = "GEOGCS[\"GCS_China_Geodetic_Coordinate_System_2000\",DATUM[\"China_2000\",SPHEROID[\"CGCS2000\",6378137.0,298.257222101],"
//                         "PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],";
//        pszCGCS_2000_3 = QString("PARAMETER[\"central_meridian\",%1],PARAMETER[\"scale_factor\",1],").arg(cm);
//    }

//    //3度分带，加带号
//    if (cm>24 && cm<46)
//    {
//        pszCGCS_2000_1 = QString("ESRI::PROJCS[\"CGCS_2000_GK_CM_zone_%1E\",").arg(cm);
//        pszCGCS_2000_2 = QString("GEOGCS[\"GCS_CGCS_2000\",DATUM[\"D_CGCS_2000\",SPHEROID[\"CGCS2000\",6378137.0,298.257222101]],"
//                         "PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Gauss_Kruger\"],"
//                         "PARAMETER[\"False_Easting\",%1.0],PARAMETER[\"False_Northing\",0.0],").arg(cm*1000000+500000);
//        pszCGCS_2000_3 = QString("PARAMETER[\"Central_Meridian\",%1.0],PARAMETER[\"Scale_Factor\",1.0],").arg(cm*3);
//    }

//    //6度分带，加带号
//    if (cm>12 && cm<24)
//    {
//        pszCGCS_2000_1 = QString("ESRI::PROJCS[\"CGCS_2000_GK_CM_zone_%1E\",").arg(cm);
//        pszCGCS_2000_2 = QString("GEOGCS[\"GCS_CGCS_2000\",DATUM[\"D_CGCS_2000\",SPHEROID[\"CGCS2000\",6378137.0,298.257222101]],"
//                         "PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Gauss_Kruger\"],"
//                         "PARAMETER[\"False_Easting\",%1.0],PARAMETER[\"False_Northing\",0.0],").arg(cm*1000000+500000);
//        pszCGCS_2000_3 = QString("PARAMETER[\"Central_Meridian\",%1.0],PARAMETER[\"Scale_Factor\",1.0],").arg(cm*6);
//    }

//    return pszCGCS_2000_1 + pszCGCS_2000_2 + pszCGCS_2000_3 + pszCGCS_2000_4;

    QString pszCGCS = QString("PROJCS[\"CGCS_2000_GK_CM_%1E\",GEOGCS[\"GCS_China_Geodetic_Coordinate_System_2000\","
                              "DATUM[\"China_2000\",SPHEROID[\"CGCS2000\",6378137.0,298.257222101]],PRIMEM[\"Greenwich\",0.0],"
                              "UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],"
                              "PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",%1.0],PARAMETER[\"Scale_Factor\",1.0],"
                              "PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]").arg(cm).arg(cm);
    return pszCGCS;
}

QString myOGR::getErdasCgcs2000Gcs(const int cm)
{
    QString pszCGCS_2000;

    //3度分带，不加带号
    if (cm>74 && cm<136)
    {
        pszCGCS_2000 = QString("PROJCS[\"Gauss Kruger\",GEOGCS[\"D_China_2000\",DATUM[\"D_China_2000\","
                         "SPHEROID[\"CGCS2000\",6378137,298.2572221010042],TOWGS84[0,0,0,0,0,0,0]],"
                         "PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],"
                         "PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],"
                         "PARAMETER[\"central_meridian\",%1],PARAMETER[\"scale_factor\",1],"
                         "PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"meters\",1]]").arg(cm);
    }

    //3度分带，加带号
    if (cm>24 && cm<46)
    {
        pszCGCS_2000 = QString("PROJCS[\"Gauss Kruger\",GEOGCS[\"D_China_2000\",DATUM[\"D_China_2000\","
                         "SPHEROID[\"CGCS2000\",6378137,298.2572221010042],TOWGS84[0,0,0,0,0,0,0]],"
                         "PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],"
                         "PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],"
                         "PARAMETER[\"central_meridian\",%1],PARAMETER[\"scale_factor\",1],"
                         "PARAMETER[\"false_easting\",%2],PARAMETER[\"false_northing\",0],UNIT[\"meters\",1]]").arg(cm*3).arg(cm*1000000+500000);
    }

    //6度分带，加带号
    if (cm>12 && cm<24)
    {
        pszCGCS_2000 = QString("PROJCS[\"Gauss Kruger\",GEOGCS[\"D_China_2000\",DATUM[\"D_China_2000\","
                         "SPHEROID[\"CGCS2000\",6378137,298.2572221010042],TOWGS84[0,0,0,0,0,0,0]],"
                         "PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],"
                         "PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],"
                         "PARAMETER[\"central_meridian\",%1],PARAMETER[\"scale_factor\",1],"
                         "PARAMETER[\"false_easting\",%2],PARAMETER[\"false_northing\",0],UNIT[\"meters\",1]]").arg(cm*6).arg(cm*1000000+500000);
    }

    return pszCGCS_2000;
}
