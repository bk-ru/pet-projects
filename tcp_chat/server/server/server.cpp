#include "server.h"

#include <QHostAddress>
#include <QDebug>
#include <memory>

Server::Server(QObject *parent, bool debug, quint32 maxMessageSize)
    : QTcpServer(parent), m_debug(debug), m_maxMessageSize(maxMessageSize)
{
}

Server::~Server()
{
    stop();
}

bool Server::start(const QString &host, quint16 port)
{
    if (m_isRunning)
        return false;

    if (!listen(QHostAddress(host), port)) {
        sendBrowserMessage(MessageType::Error, "Не удалось запустить сервер");
        return false;
    }

    sendBrowserMessage(MessageType::ServerInfo,
                       QString("Сервер запущен на - %1:%2").arg(host).arg(port));

    QString statusMessage{"Сервер слушает на : " + host + ":" + QString::number(port)};
    emit signalReloadConStatus(statusMessage);
    m_isRunning = true;
    return true;
}

bool Server::stop()
{
    if (!m_isRunning)
       return false;

    if (this->isListening())
        close();

    m_clients.clear();
    m_isRunning = false;

    sendBrowserMessage(MessageType::ServerInfo, "Сервер остановлен");

    QString statusMessage{"Сервер остановлен"};
    emit signalReloadConStatus(statusMessage);

    return true;
}

QList<Server::ClientInfo> Server::getClientsInfo() const
{
    QList<ClientInfo> clients;
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        ClientInfo info;
        info.id = it.key();
        info.ip = it.value()->getPeerAddress();
        info.connectTime = QDateTime::currentDateTime();
        clients.append(info);
    }
    return clients;
}

void Server::incomingConnection(qintptr handle)
{
    QTcpSocket *socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(handle)) {
        if (m_debug)
            qDebug() << "Ошибка создания сокета:" << socket->errorString();
        delete socket;
        return;
    }

    sendBrowserMessage(MessageType::ClientConn,
                       QString("Клиент по адресу %1 подключился")
                       .arg(socket->peerAddress().toString()));

    ClientHandler *handler = new ClientHandler(socket, this, m_debug, m_maxMessageSize);

    QObject::connect(handler, &ClientHandler::signalDisconnected, this, &Server::onClientDisconnected);
    QObject::connect(handler, &ClientHandler::signalStrMessageReceived, this, &Server::onClientMessageReceived);
    QObject::connect(handler, &ClientHandler::signalAuthorize, this, &Server::onClientAuthoriz);
}

void Server::onClientDisconnected(const QUuid& clientId)
{
    sendBrowserMessage(MessageType::ClientConn,
                       QString("Клиент %1 отключился")
                       .arg(clientId.toString()));

    if (m_clients.contains(clientId)) {
        m_clients.remove(clientId);
    }

    for (auto it = m_clients.begin(); it != m_clients.end();) {
        if (it.value() == sender()) {
            it = m_clients.erase(it);
        } else {
            ++it;
        }
    }

    emit signalClientsListChanged();
}

void Server::onClientAuthoriz(const QUuid& currentId, const QUuid &newId)
{
    ClientHandler* client = qobject_cast<ClientHandler*>(sender());
    if (!client) {
        if (m_debug)
            qDebug() << "Client = nullptr";
        return;
    }

    if (m_clients.contains(newId) && m_clients[newId] != client) {
        client->sendDontAuthorize();
        sendBrowserMessage(MessageType::ClientConn,
                           QString("Клиент по адресу %1 попытался использовать занятый ID %2")
                           .arg(client->getPeerAddress())
                           .arg(newId.toString()));
        return;
    }

    if (currentId != newId) {
        if (m_clients.contains(currentId)) {
            m_clients.remove(currentId);
        }
    }

    m_clients[newId] = client;
    sendBrowserMessage(MessageType::ClientConn,
                       QString("Клиент по адресу %1 авторизовался под ID %2")
                       .arg(client->getPeerAddress())
                       .arg(newId.toString()));

    emit signalClientsListChanged();
}

void Server::onClientMessageReceived(const QUuid& clientId, const QString &message)
{
    QString fullMessage = QString("%2").arg(message);

    sendBrowserMessage(MessageType::ClientInfo, clientId,  fullMessage);
    broadcastMessage(fullMessage, clientId);
}

void Server::sendBrowserMessage(MessageType type, const QString &message)
{
    QString text = QString("%1 | %2")
            .arg(getCurrentTime())
            .arg(message);
    emit signalsMessageToBrowser(type, text);
}

void Server::sendBrowserMessage(MessageType type, const QUuid &id, const QString &message)
{
    QString text = QString("%1: %2")
            .arg(id.toString(QUuid::WithoutBraces).left(8))
            .arg(message);
    sendBrowserMessage(type, text);
}

QString Server::getCurrentTime() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void Server::broadcastMessage(const QString &message, const QUuid &clientId)
{
    QByteArray data = message.toUtf8();

    if (m_debug) {
        qDebug() << "=== Server::broadcastMessage ===";
        qDebug() << "Сообщение для рассылки:" << message;
        qDebug() << "Количество клиентов:" << m_clients.size();
    }

    // Используйте тот же подход, что и в ClientHandler::sendMessage()
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        qDebug() << clientId;
        if (it.value()->isConnected() && it.key() != clientId) {
            qDebug() << "1";
            it.value()->sendMessage(message);

            if (m_debug) {
                qDebug() << "Отправка клиенту" << it.key().toString(QUuid::WithoutBraces).left(8);
            }
        }
    }

    if (m_debug) {
        qDebug() << "=== Конец broadcastMessage ===";
    }
}

bool Server::isRunning()
{
    return m_isRunning;
}
