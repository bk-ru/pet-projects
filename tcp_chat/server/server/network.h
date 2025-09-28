#ifndef NETWORK_H
#define NETWORK_H

#include <QDialog>

#include "server.h"

namespace Ui {
class Network;
}

class Network : public QDialog
{
    Q_OBJECT

public:
    explicit Network(QWidget *parent = nullptr);
    ~Network();

public:
    void setHost(const QString &host);
    void setPort(quint16 port);

signals:
    void signalConnected(const QString& host, quint16 port);
    void signalDisconnected();

private slots:
    void on_connect_clicked();
    void on_disconnect_clicked();

private:
    Ui::Network *ui;
    Server *m_server{nullptr};
    Network *m_network{nullptr};
};

#endif // NETWORK_H
