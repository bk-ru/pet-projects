#include "config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

Config::Config(QObject *parent)
    : QObject(parent)
{
    load("client.json");
}

Config::~Config()
{
}

bool Config::load(const QString &filePath)
{
    QFile configFile(filePath);
    if (!configFile.exists()) {
        qWarning() << "Конфигурационный файл не найден!";
        return false;
    }

    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть файл конфигурации для чтения!";
        return false;
    }

    QByteArray data = configFile.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Ошибка при разборе JSON!";
        return false;
    }

    setSettings(doc);
    configFile.close();

    return true;
}

void Config::setSettings(const QJsonDocument &doc)
{
    QJsonObject jsonObject = doc.object();
    if (jsonObject.contains("host") && jsonObject["host"].isString())
        m_host = jsonObject["host"].toString();
    if (jsonObject.contains("port") && jsonObject["port"].isDouble())
        m_port = static_cast<quint16>(jsonObject["port"].toInt());
    if (jsonObject.contains("myId") && jsonObject["myId"].isString())
        m_myId = static_cast<QUuid>(jsonObject["myId"].toString());
}

const QString& Config::getHost() const
{
    return m_host;
}

quint16 Config::getPort() const
{
    return m_port;
}

const QUuid &Config::getId() const
{
    return m_myId;
}

void Config::setId(const QUuid &newId)
{
    m_myId = newId;
}
