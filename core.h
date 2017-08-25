#ifndef CORE_H
#define CORE_H

#include <QObject>

class core : public QObject
{
    Q_OBJECT
public:
    explicit core(QObject *parent = 0);

    // 计算影像数据量
    static QString dataAmount(const QString &xmlPath);
signals:

public slots:
};

#endif // CORE_H
