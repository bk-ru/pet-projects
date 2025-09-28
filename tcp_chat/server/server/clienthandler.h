#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QUuid>
#include <QHostAddress>

struct NetworkMessage {
    quint16 messageType;  // Тип сообщения
    QUuid userId;         // Уникальный идентификатор пользователя
    quint32 messageSize{0};  // Размер сообщения (без заголовка)
};

enum class ClientMessageType : quint16 {
    AUTH = 0,
    TEXT = 1,
    FILE = 2,
    IMAGE = 3
};

class ClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClientHandler(QTcpSocket *socket, QObject *parent = nullptr,
                           bool debug = false, quint32 maxMessageSize = 10 * 1024 * 1024);
    ~ClientHandler();

public:
    void disconnectFromHost();

public:
    const QUuid& getId() const;
    const QString getPeerAddress() const;

public:
    bool isValid() const;
    void sendData(const QByteArray &data);
    void sendMessage(const QString& message);

public:
    bool isConnected();

signals:
    void signalConnected();
    void signalDisconnected(const QUuid& clientId);
    void signalStrMessageReceived(const QUuid& userId, const QString &message);
    void signalAuthorize(const QUuid& currentId, const QUuid& userId);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void processMessage(const NetworkMessage &header, const QByteArray &data);

public:
    void sendDontAuthorize();

private:
    NetworkMessage m_expectedMsg;
    QByteArray m_buffer{QByteArray()};
    QTcpSocket *m_socket{nullptr};
    QUuid m_id{QUuid::createUuid()};
    bool m_debug{false};

    int m_headerSize{sizeof(NetworkMessage)};
    quint32 m_maxMessageSize;
};

#endif // CLIENTHANDLER_H
