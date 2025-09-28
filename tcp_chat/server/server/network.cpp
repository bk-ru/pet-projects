#include "network.h"
#include "ui_network.h"

#include <QMessageBox>
#include <QDebug>

Network::Network(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Network)
{
    ui->setupUi(this);
    this->setWindowTitle("Network");
}

Network::~Network()
{
    delete ui;
}

void Network::setHost(const QString &host)
{
    ui->host->setText(host);
}

void Network::setPort(quint16 port)
{
    ui->port->setText(QString::number(port));
}

void Network::on_connect_clicked()
{
    QString host = ui->host->text();
    if (host.isEmpty()) {
        QMessageBox::warning(this, "Инфо", "Поле host должно быть заполненно, пример: 127.0.0.1");
        return;
    }
    bool ok;
    quint16 port = ui->port->text().toUShort(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Инфо", "Порт должен быть числом от 0 до 65535");
        return;
    }

    emit signalConnected(host, port);
}

void Network::on_disconnect_clicked()
{
    emit signalDisconnected();
}
