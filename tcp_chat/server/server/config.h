#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QHash>
#include <QObject>

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
    quint16 getPort();
    quint32 getMaxMessageSize();

private:
    QString m_filePath;
    QString m_host{"127.0.0.1"};
    quint16 m_port{12345};
    quint32 m_maxMesssageSize{10*1024*1024}; // 10 MB
};

#endif // CONFIG_H
