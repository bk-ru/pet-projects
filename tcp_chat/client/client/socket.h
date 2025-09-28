#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QObject>
#include <QTimer>
#include <QUuid>

enum class ClientMessageType : quint16 {
    AUTH = 0,
    TEXT = 1,
    FILE = 2,
    IMAGE = 3
};

#pragma pack(push, 1)
struct NetworkMessage {
    quint16 messageType;  // Тип сообщения
    QUuid userId;         // Уникальный идентификатор пользователя
    quint32 messageSize;  // Размер сообщения (без заголовка)
};
#pragma pack(pop)

class Socket : public QObject
{
    Q_OBJECT
public:
    static Socket* instance(QObject *parent = nullptr, bool debug = false);
    ~Socket() override;

private:
    explicit Socket(QObject *parent = nullptr, bool debug = false);
    Q_DISABLE_COPY(Socket)

public:
    void setUserId(const QUuid& id);
    void updateUserId(const QUuid& newId);

public:
    void connectToHost(const QString &host, quint16 port);
    void disconnectFromHost();

public:
    void sendBinaryData(ClientMessageType type, const QUuid &userId, const QByteArray &data);
    void sendAuthMessage(const QUuid &userId);
    void sendTextMessage(const QString &message, const QUuid &userId);
    void sendMessage(ClientMessageType type, const QUuid &userId, const QByteArray &data = QByteArray());

public:
    bool isConnected() const;

signals:
    void signalAuthSuccess(const QUuid& userId);
    void signalConnected();
    void signalDisconnected();
    void signalErrorOccurred(const QString &error);
    void signalMessageReceived(const QUuid& userId, const QString &message);

signals:
    void signalReloadConStatus(const QString& status);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onStateChanged(QAbstractSocket::SocketState socketState);

private:
    QTcpSocket *m_socket{nullptr};
    QUuid m_myId{QUuid::createUuid()};
    QByteArray m_buffer{QByteArray()};
    NetworkMessage m_expectedMsg;
    bool m_debug{false};
    int HEADER_SIZE = sizeof(NetworkMessage);
};

#endif // SOCKET_H
