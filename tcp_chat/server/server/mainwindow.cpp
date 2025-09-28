#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_config(new Config(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Chat (Server)");

    m_config->load("server.json");
    m_server = new Server(parent, m_debug, m_config->getMaxMessageSize());

    QObject::connect(m_server, &Server::signalReloadConStatus, this, &MainWindow::reloadConStatus);
    QObject::connect(m_server, &Server::signalsMessageToBrowser, this, &MainWindow::textToBrowser);
    QObject::connect(m_server, &Server::signalClientsListChanged, this, &MainWindow::updateClientsTable);
    m_server->start(m_config->getHost(), m_config->getPort());

    updateClientsTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateClientsTable()
{
    ui->ClientTable->setRowCount(0);
    QList<Server::ClientInfo> clients = m_server->getClientsInfo();
    for (const auto& client : clients) {
        int row = ui->ClientTable->rowCount();
        ui->ClientTable->insertRow(row);

        QTableWidgetItem* idItem = new QTableWidgetItem(client.id.toString(QUuid::WithoutBraces).left(8));
        idItem->setData(Qt::UserRole, client.id);
        ui->ClientTable->setItem(row, 0, idItem);
        ui->ClientTable->setItem(row, 1, new QTableWidgetItem(client.ip));
        ui->ClientTable->setItem(row, 2, new QTableWidgetItem(client.connectTime.toString("yyyy-MM-dd hh:mm:ss")));
    }

    ui->ClientTable->resizeColumnsToContents();
}
void MainWindow::textToBrowser(Server::MessageType type, const QString &message)
{
    switch (type) {
        case Server::MessageType::ServerInfo:
            ui->textBrowser->setTextColor(Qt::blue);
            break;
        case Server::MessageType::ClientInfo:
            ui->textBrowser->setTextColor(Qt::black);
            break;
        case Server::MessageType::ClientConn:
            ui->textBrowser->setTextColor(Qt::gray);
            break;
        case Server::MessageType::Warning:
            ui->textBrowser->setTextColor(Qt::yellow);
            break;
        case Server::MessageType::Error:
            ui->textBrowser->setTextColor(Qt::red);
            break;
        default:
            ui->textBrowser->setTextColor(Qt::black);
            break;
    }
    ui->textBrowser->append(message);
}

void MainWindow::reloadConStatus(const QString &status)
{
    ui->networkStatus->setText(status);
}

void MainWindow::on_network_clicked()
{
    if (!ensureDialogVisible(m_network, this)) {
        m_network->setHost(m_config->getHost());
        m_network->setPort(m_config->getPort());
        QObject::connect(m_network, &Network::signalConnected, m_server, &Server::start);
        QObject::connect(m_network, &Network::signalDisconnected, m_server, &Server::stop);
    }
}

template<typename DialogType>
bool MainWindow::ensureDialogVisible(QPointer<DialogType> &dialog, QWidget *parent)
{
    if (dialog && dialog->isVisible()) {
        dialog->raise();
        dialog->activateWindow();
        return true;
    } else {
        dialog = new DialogType(parent);
        dialog->show();
        return false;
    }
}


void MainWindow::on_clearChat_clicked()
{
    ui->textBrowser->clear();
}

