#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QUuid>
#include <QTime>
#include <QDateTime>
#include <QDataStream>

#include "clienthandler.h"


class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr,
                    bool debug = false, quint32 maxMessageSize = 10 * 1024 * 1024);
    ~Server() override;

public:
    enum class MessageType {
            ServerInfo,
            ClientInfo,
            ClientConn,
            Warning,
            Error
        };
        Q_ENUM(MessageType)

public slots:
    bool start(const QString &host, quint16 port);
    bool stop();

public:
    struct ClientInfo {
        QUuid id;
        QString ip;
        QDateTime connectTime;
    };

    QList<Server::ClientInfo> getClientsInfo() const;

public:
    bool isRunning();

signals:
    void signalReloadConStatus(const QString& status);

signals:
    void signalClientsListChanged();

signals:
    void signalsMessageToBrowser(Server::MessageType type, const QString &message);

protected:
    void incomingConnection(qintptr handle) override;

private slots:
    void onClientDisconnected(const QUuid& clientId);
    void onClientMessageReceived(const QUuid& clientId, const QString &message);
    void onClientAuthoriz(const QUuid& currentId, const QUuid& newId);

private:
    void sendBrowserMessage(MessageType type, const QString &message);
    void sendBrowserMessage(MessageType type, const QUuid& id, const QString &message);

private:
    QString getCurrentTime() const;

private:
    void broadcastMessage(const QString& message, const QUuid& clientId);

private:
    QHash<QUuid, ClientHandler*> m_clients;
    bool m_isRunning{false};
    bool m_debug{true};
    quint32 m_maxMessageSize;
};

#endif // SERVER_H
