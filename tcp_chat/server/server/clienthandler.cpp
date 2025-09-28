#include "clienthandler.h"
#include <QDataStream>

ClientHandler::ClientHandler(QTcpSocket *socket, QObject *parent, bool debug, quint32 maxMessageSize)
    : QObject(parent), m_socket(socket), m_debug(debug), m_maxMessageSize(maxMessageSize)
{
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);
}

ClientHandler::~ClientHandler()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void ClientHandler::disconnectFromHost()
{
    m_socket->disconnectFromHost();
}

const QUuid& ClientHandler::getId() const
{
    return m_id;
}

const QString ClientHandler::getPeerAddress() const
{
    return m_socket->peerAddress().toString();
}

bool ClientHandler::isValid() const
{
    return m_socket && m_socket->state() == QTcpSocket::ConnectedState;
}

void ClientHandler::sendData(const QByteArray &data)
{
    if (isValid()) {
        m_socket->write(data);
        if (m_debug)
            qDebug() << "Отправлено клиенту" << m_id << ":" << data;
    }
}

void ClientHandler::sendMessage(const QString& message)
{
    if (!isConnected())
        return;

    QByteArray data = message.toUtf8();

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15); // Добавьте эту строку

    out << static_cast<quint16>(ClientMessageType::TEXT);
    out << m_id;
    out << static_cast<quint32>(data.size());
    out.writeRawData(data.constData(), data.size());

    m_socket->write(packet);

    if (m_debug)
        qDebug() << "Сообщение отправлено клиенту" << m_id << ":" << message;
}

void ClientHandler::sendDontAuthorize()
{
    if (!isConnected())
        return;

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15); // Добавьте эту строку

    out << static_cast<quint16>(ClientMessageType::AUTH);
    out << QUuid();  // Пустой ID как индикатор ошибки
    out << static_cast<quint32>(0);  // Нет данных

    m_socket->write(packet);

    if (m_debug)
        qDebug() << "Отправлен ответ об ошибке авторизации клиенту" << m_id;
}

void ClientHandler::onReadyRead()
{
    m_buffer.append(m_socket->readAll());

    while (true) {
        quint32 expectedHeaderSize = sizeof(quint16) + sizeof(QUuid) + sizeof(quint32);

        if (m_buffer.size() < expectedHeaderSize)
            return;

        QDataStream in(&m_buffer, QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_5_15); // Добавьте эту строку
        quint16 messageType;
        QUuid userId;
        quint32 messageSize;

        in >> messageType >> userId >> messageSize;

        if (messageSize > m_maxMessageSize) {
            qCritical() << "Превышен максимальный размер сообщения:" << messageSize;
            m_socket->close();
            return;
        }

        quint32 fullMessageSize = expectedHeaderSize + messageSize;
        if (m_buffer.size() < fullMessageSize)
            return;

        QByteArray messageData = m_buffer.mid(expectedHeaderSize, messageSize);

        NetworkMessage header;
        header.messageType = messageType;
        header.userId = userId;
        header.messageSize = messageSize;

        processMessage(header, messageData);

        m_buffer = m_buffer.mid(fullMessageSize);
    }
}

void ClientHandler::processMessage(const NetworkMessage& header, const QByteArray& data)
{
    if (m_debug) {
        qDebug() << "=== ClientHandler::processMessage ===";
        qDebug() << "Тип сообщения:" << header.messageType;
        qDebug() << "ID пользователя:" << header.userId.toString();
        qDebug() << "Размер данных:" << header.messageSize;
        if (!data.isEmpty()) {
            qDebug() << "Данные:" << QString::fromUtf8(data);
        }
    }

    switch (static_cast<ClientMessageType>(header.messageType)) {
        case ClientMessageType::AUTH:
            if (m_debug)
                qDebug() << "Обработка AUTH от" << header.userId.toString();
            emit signalAuthorize(m_id, header.userId);
            m_id = header.userId;
            break;
        case ClientMessageType::TEXT:
            if (m_debug)
                qDebug() << "Обработка TEXT от" << header.userId.toString();
            emit signalStrMessageReceived(header.userId, QString::fromUtf8(data));
            break;
        default:
            qWarning() << "Неизвестный тип сообщения:" << header.messageType;
            break;
    }
}

bool ClientHandler::isConnected()
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void ClientHandler::onDisconnected()
{
    emit signalDisconnected(m_id);
}
