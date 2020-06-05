#ifndef SINGLETON_H
#define SINGLETON_H

#include <QMap>
#include <QString>
#include <QSettings>

class Singleton {
public:
    static Singleton & getInstance();
    QMap<QString, QString> getSystemParamXml();
    QSettings * getConfigIni();
    QSettings * getConfigQtIni();
    int getMaxDetectFace();
    void dump();

private:
    Singleton();
    ~Singleton();
    Singleton(const Singleton &);
    Singleton & operator=(const Singleton &);

private:
    QMap<QString, QString> xml;
    QSettings *configQtIni;
    QSettings *configIni;
};

#endif // SINGLETON_H
