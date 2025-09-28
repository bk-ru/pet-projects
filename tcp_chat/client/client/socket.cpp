#include "socket.h"

#include <QDebug>
#include <QDataStream>
#include <QHostAddress>
#include <QByteArray>

Socket::Socket(QObject *parent, bool debug)
    : QObject(parent),
      m_socket(new QTcpSocket(this)), m_debug(debug)
{
    QObject::connect(m_socket, &QTcpSocket::connected, this, &Socket::onConnected);
    QObject::connect(m_socket, &QTcpSocket::disconnected, this, &Socket::onDisconnected);
    QObject::connect(m_socket, &QTcpSocket::readyRead, this, &Socket::onReadyRead);
    QObject::connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &Socket::onErrorOccurred);
    QObject::connect(m_socket, &QTcpSocket::stateChanged, this, &Socket::onStateChanged);
}

Socket *Socket::instance(QObject *parent, bool debug) {
    static Socket *instance = nullptr;
    if (instance == nullptr) {
        instance = new Socket(parent, debug);
    }
    return instance;
}

Socket::~Socket()
{
    if (m_socket->isOpen()) {
        m_socket->close();
    }
}

void Socket::connectToHost(const QString &host, quint16 port)
{
    if (m_socket->isOpen())
        m_socket->abort();

    m_socket->connectToHost(host, port);

    QTimer::singleShot(100, [this, host, port]() {
        if (isConnected()) {
            QString statusMessage{"Успешное соединение : " + host + ":" + QString::number(port)};
            emit signalReloadConStatus(statusMessage);
            m_socket->connectToHost(host, port);
        }
    });
}

void Socket::disconnectFromHost()
{
    if (m_socket->isOpen())
        m_socket->disconnectFromHost();
}

void Socket::onStateChanged(QAbstractSocket::SocketState socketState)
{
    QString statusMessage;

    switch (socketState) {
    case QAbstractSocket::UnconnectedState:
        statusMessage = "Нет соединения";
        break;
    case QAbstractSocket::HostLookupState:
        statusMessage = "Поиск хоста...";
        break;
    case QAbstractSocket::ConnectingState:
        statusMessage = "Подключение...";
        break;
    case QAbstractSocket::ConnectedState:
        statusMessage = "Подключено";
        break;
    case QAbstractSocket::BoundState:
        statusMessage = "Привязка...";
        break;
    case QAbstractSocket::ListeningState:
        statusMessage = "Прослушивание...";
        break;
    case QAbstractSocket::ClosingState:
        statusMessage = "Закрытие соединения...";
        break;
    default:
        statusMessage = "Неизвестное состояние";
        break;
    }

    if (socketState == QAbstractSocket::ConnectedState && m_socket->isOpen()) {
        statusMessage = QString("Подключено к %1:%2")
                       .arg(m_socket->peerAddress().toString())
                       .arg(m_socket->peerPort());
    }

    emit signalReloadConStatus(statusMessage);
}

void Socket::sendMessage(ClientMessageType type, const QUuid& userId, const QByteArray& data)
{
    if (!isConnected()) {
        emit signalErrorOccurred("Сокет не подключен");
        return;
    }

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);

    out << static_cast<quint16>(type);
    out << userId;
    out << static_cast<quint32>(data.size());
    out.writeRawData(data.constData(), data.size());
    m_socket->write(packet);
}

// Для текстовых сообщений
void Socket::sendTextMessage(const QString& message, const QUuid& userId)
{
    sendMessage(ClientMessageType::TEXT, userId, message.toUtf8());
}

// Для авторизации
void Socket::sendAuthMessage(const QUuid& userId)
{
    sendMessage(ClientMessageType::AUTH, userId);
}

// Для бинарных данных (файлы, изображения)
void Socket::sendBinaryData(ClientMessageType type, const QUuid& userId, const QByteArray& data)
{
    Q_ASSERT(type == ClientMessageType::FILE || type == ClientMessageType::IMAGE);
    sendMessage(type, userId, data);
}

void Socket::onReadyRead()
{
    QByteArray newData = m_socket->readAll();
    m_buffer.append(newData);
    while (true) {
        quint32 expectedHeaderSize = sizeof(quint16) + sizeof(QUuid) + sizeof(quint32);

        if (m_buffer.size() < expectedHeaderSize) {
            if (m_debug) {
                qDebug() << "Недостаточно данных для заголовка (" << m_buffer.size()
                         << " < " << expectedHeaderSize << "), ждем больше данных";
            }
            break;
        }

        // Создаем новый QDataStream для каждого чтения!
        QDataStream in(&m_buffer, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_15); // Добавьте эту строку
        quint16 messageType;
        QUuid userId;
        quint32 messageSize;

        in >> messageType >> userId >> messageSize;
        m_expectedMsg.messageType = messageType;
        m_expectedMsg.userId = userId;
        m_expectedMsg.messageSize = messageSize;
        quint32 fullMessageSize = expectedHeaderSize + m_expectedMsg.messageSize;

        if (m_buffer.size() < fullMessageSize) {
            if (m_debug) {
                qDebug() << "Недостаточно данных для полного сообщения (" << m_buffer.size()
                         << " < " << fullMessageSize << "), ждем больше данных";
            }
            break;
        }
        QByteArray messageData = m_buffer.mid(expectedHeaderSize, m_expectedMsg.messageSize);

        switch (m_expectedMsg.messageType) {
            case 0: // AUTH
                if (m_expectedMsg.userId.isNull()) {
                    if (m_debug)
                        qDebug() << "Не удалось авторизоваться";
                    emit signalErrorOccurred("Авторизация не удалась");
                } else {
                    if (m_debug)
                        qDebug() << "Успешная авторизация с ID:" << m_expectedMsg.userId.toString();
                    emit signalAuthSuccess(m_expectedMsg.userId);
                }
                break;

            case 1: // TEXT
                emit signalMessageReceived(m_expectedMsg.userId, QString::fromUtf8(messageData));
                break;

            default:
                qWarning() << "Неизвестный тип сообщения:" << m_expectedMsg.messageType;
                qWarning() << "Данные:" << messageData.toHex();
                break;
        }

        m_buffer = m_buffer.mid(fullMessageSize);
        m_expectedMsg.messageSize = 0;

        if (m_buffer.isEmpty() || m_buffer.size() < expectedHeaderSize)
            break;
    }
}

bool Socket::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void Socket::onConnected()
{
    if (m_debug)
        qDebug() << "Подключено к серверу";

    QString statusMessage = QString("Подключено к %1:%2")
                           .arg(m_socket->peerAddress().toString())
                           .arg(m_socket->peerPort());
    emit signalReloadConStatus(statusMessage);
    sendAuthMessage(m_myId);
    emit signalConnected();
}

void Socket::onDisconnected()
{
    if (m_debug)
        qDebug() << "Отключение от сервера";

    QString statusMessage{"Нет соединения"};
    emit signalReloadConStatus(statusMessage);
    emit signalDisconnected();
}

void Socket::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
}

void Socket::setUserId(const QUuid &id)
{
    m_myId = id;
}

void Socket::updateUserId(const QUuid &newId)
{
    QUuid oldId = m_myId;
    m_myId = newId;

    if (isConnected()) {
        sendAuthMessage(m_myId);
        if (m_debug)
            qDebug() << "ID обновлен с" << oldId.toString() << "на" << newId.toString();
    }
}
