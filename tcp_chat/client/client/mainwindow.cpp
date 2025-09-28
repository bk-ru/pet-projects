#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_config(new Config(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Chat (Client)");

    ui->myID->setText(m_config->getId().toString(QUuid::WithoutBraces));

    m_socket = Socket::instance(parent, m_debug);
    m_socket->setUserId(m_config->getId());

    QObject::connect(m_socket, &Socket::signalReloadConStatus, this, &MainWindow::onReloadConStatus);
    QObject::connect(m_socket, &Socket::signalMessageReceived, this, &MainWindow::onMessageReceived);
    QObject::connect(m_socket, &Socket::signalAuthSuccess, this, &MainWindow::onAuthSuccess);
//    m_socket->connectToHost("127.0.0.1", 12345);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onReloadConStatus(const QString &status)
{
    ui->networkStatus->setText(status);
}

void MainWindow::onAuthSuccess(const QUuid& userId)
{
    if (m_debug)
        qDebug() << "Успешная авторизация с ID:" << userId.toString();
    ui->myID->setText(userId.toString(QUuid::WithoutBraces));
}

void MainWindow::onMessageReceived(const QUuid& userId, const QString& message)
{
    QColor oldColor = ui->textBrowser->textColor();
    ui->textBrowser->setTextColor(Qt::black);
    QString senderId = userId.toString(QUuid::WithoutBraces).left(8);
    QString displayMessage = QString("[%1]: %2").arg(senderId).arg(message);
    ui->textBrowser->append(displayMessage);
    ui->textBrowser->setTextColor(oldColor);
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(cursor);
}

void MainWindow::on_network_clicked()
{
    ensureDialogVisible(m_network, this);
    if (m_network) {
        m_network->setHost(m_config->getHost());
        m_network->setPort(m_config->getPort());
        QObject::connect(m_network, &Network::signalConnected, m_socket, &Socket::connectToHost);
        QObject::connect(m_network, &Network::signalDisconnected, m_socket, &Socket::disconnectFromHost);
    }
}


void MainWindow::on_send_clicked()
{
    QString message = ui->textSend->toPlainText();
    if (!message.isEmpty()) {
        m_socket->sendTextMessage(message, m_config->getId());
        QColor oldColor = ui->textBrowser->textColor();
        ui->textBrowser->setTextColor(Qt::darkGray);
        QString displayMessage = QString("[Я]: %1").arg(message);
        ui->textBrowser->append(displayMessage);
        ui->textBrowser->setTextColor(oldColor);
        ui->textSend->clear();
    }
}

void MainWindow::on_newID_clicked()
{
    QUuid oldId = m_config->getId();
    QUuid newId = QUuid::createUuid();
    m_config->setId(newId);
    m_socket->updateUserId(newId);
    QColor oldColor = ui->textBrowser->textColor();
    ui->textBrowser->setTextColor(Qt::black);
    QString displayMessage = QString("[Система]: ID обновлен с %1 на %2")
                                .arg(oldId.toString(QUuid::WithoutBraces).left(8))
                                .arg(newId.toString(QUuid::WithoutBraces).left(8));
    ui->textBrowser->append(displayMessage);
    ui->textBrowser->setTextColor(oldColor);
    ui->myID->setText(newId.toString(QUuid::WithoutBraces));
}

void MainWindow::on_history_clicked()
{

}

template<typename DialogType>
void MainWindow::ensureDialogVisible(QPointer<DialogType> &dialog, QWidget *parent)
{
    if (dialog && dialog->isVisible()) {
        dialog->raise();
        dialog->activateWindow();
    } else {
        dialog = new DialogType(parent);
        dialog->show();
    }
}

void MainWindow::on_clearChat_clicked()
{
    ui->textBrowser->clear();
}

