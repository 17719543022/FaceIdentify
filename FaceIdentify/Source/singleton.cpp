#include "Include/singleton.h"
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QXmlStreamReader>
#include <QString>
#include <QMap>
#include <QDomDocument>

Singleton::Singleton()
{
    configIni = new QSettings("config.ini", QSettings::IniFormat);
    configQtIni = new QSettings("config_qt.ini", QSettings::IniFormat);

    QString systemParam = "SystemParam.xml";

    try {
        QString path = QCoreApplication::applicationDirPath();
        QDomDocument doc;
        QFile file(path + "/" + systemParam);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << QString("%1 QFile open failure").arg(systemParam);
            return;
        }
        if (!doc.setContent(&file)) {
            qCritical() << doc.setContent(&file);
            qCritical() << QString("%1 QDomDocument setContent failure").arg(systemParam);
            file.close();
            return;
        }
        file.close();

        QDomElement docElem = doc.documentElement();
        if (docElem.toElement().tagName() == "Message") {
            QDomNode node = docElem.firstChild();
            while (!node.isNull()) {
                if (node.isElement()) {
                    QDomElement elem = node.toElement();
                    QDomNodeList list = elem.childNodes();
                    for (int i = 0; i < list.count(); i++) {
                        QDomNode node1 = list.at(i);
                        if (node1.isElement()) {
                            xml.insert(node1.toElement().tagName(), node1.toElement().text());
                        }
                    }
                }
                node = node.nextSibling();
            }
        }
    } catch (std::exception &ex) {
        qCritical() << ex.what();
        return;
    }
}

Singleton::~Singleton()
{
    delete configIni;
    delete configQtIni;
}

Singleton & Singleton::operator=(const Singleton &)
{
    return *this;
}

Singleton & Singleton::getInstance()
{
    static Singleton singleton;
    return singleton;
}

void Singleton::dump()
{
    qDebug() << "/////SystemParam.xml/////";
    QMap<QString, QString>::Iterator it;
    for (it = xml.begin(); it != xml.end(); ++it) {
        qDebug("%s: %s", qPrintable(it.key()), qPrintable(it.value()));
    }

    qDebug() << "/////MaxDetectFace/////";
    qDebug() << "MaxDetectFace: " << getMaxDetectFace();
}

QMap<QString, QString> Singleton::getSystemParamXml()
{
    return xml;
}

QSettings * Singleton::getConfigIni()
{
    return configIni;
}

QSettings * Singleton::getConfigQtIni()
{
    return configQtIni;
}

int Singleton::getMaxDetectFace()
{
    return 1;
}
