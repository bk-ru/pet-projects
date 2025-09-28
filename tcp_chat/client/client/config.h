#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QHash>
#include <QObject>
#include <QUuid>

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = nullptr);
    ~Config();

public:
    bool load(const QString &filePath);
    void setSettings(const QJsonDocument& doc);

public:
    const QString& getHost() const;
    quint16 getPort() const;
    const QUuid& getId() const;

public:
    void setId(const QUuid &newId);

private:
    QString m_filePath{"client.json"};
    QString m_host{"127.0.0.1"};
    quint16 m_port{12345};
    QUuid m_myId{"32513efb-11e7-488d-90ea-9ca6c157b76e"};
};

#endif // CONFIG_H
