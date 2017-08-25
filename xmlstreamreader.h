#ifndef XMLSTREAMREADER_H
#define XMLSTREAMREADER_H

#include <QMap>
#include <QList>
#include <QXmlStreamReader>

class QFile;
class QChar;
class QString;
class QTreeWidget;
class QTreeWidgetItem;
class CollectionItems;
class QXmlStreamWriter;

class XmlStreamReader
{
//    Q_OBJECT

public:
    XmlStreamReader();
    XmlStreamReader(QTreeWidget *tree);

    //在tree中显示XML内容
    bool readFile(const QString &fileName);

    //将路径中XML相同内容显示在tree中
    bool readFiles(const QStringList &pathList);

    //根据MAP内容重新设置tree内容
    void setTree();

    //返回元数据中所有key列表
    static QStringList getKeyList(const QString &fileName);

    //返回对应的列表值
    QStringList getDifferentList(const QString &key);

    //返回所有key列表值
    QStringList getDifferentListAll();

    //比较元数据文件名称与内容是否一致
    bool isMetaDataFileName(const QString &name);

    //设置元数据路径，并打开元数据
    bool setFilePath(const QString &path);

    //关闭元数据
    void close();

    //返回指定项的内容
    QString getElementValue(const QString &strElement);

    //读取tree中的项生成新的元数据
    bool writeXml(const QString &fileName, QTreeWidget *tree, const QList<bool> iscks, const QStringList &jd_list);

    QStringList getCerrList();

private:
    void readMetadataindexElement();
    void readEntryElement(QTreeWidgetItem *parent);
    void skipUnknownElement();

    //创建一个对应于QTreeWidgetItem的<entry>元素并将其作为参数接受
    bool writeIndexEntry(QXmlStreamWriter *xmlWriter, QTreeWidgetItem *item, const QList<bool> iscks, const QStringList &jd_list);

    //保存当前元数据路径
    QString filePath;

    //保存精度报告路径
    QStringList jd_list;

    //保持错误信息
    QStringList cerrList;

    QTreeWidget *treeWidget;
    QXmlStreamReader reader;
    QMap<QString, CollectionItems> map;

    QFile* files;
};

class CollectionItems
{
public:
    CollectionItems();

    //添加内容
    void addVol(const QString &strKey, const QString &strVol);

    //比较存储所有内容是否一致
    bool isConsistency(const QString &str);

    //返回一个保存不一致内容的元数据名称列表
    QStringList getDifferent(const QString &key, const QString &str);

    //返回列表中第一个值
    QString getFistValue();

private:
    QStringList keyList;
    QStringList volList;
};

#endif // XMLSTREAMREADER_H
