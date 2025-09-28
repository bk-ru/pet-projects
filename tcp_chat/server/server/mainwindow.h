#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>

#include "network.h"
#include "server.h"
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    template<typename DialogType>
    bool ensureDialogVisible(QPointer<DialogType> &dialog, QWidget* parent);

private slots:
    void reloadConStatus(const QString& status);

private slots:
    void on_network_clicked();

private slots:
    void updateClientsTable();

private slots:
    void textToBrowser(Server::MessageType type, const QString& message);

    void on_clearChat_clicked();

private:
    Ui::MainWindow *ui;
    Server *m_server{nullptr};
    Config *m_config{nullptr};
    QPointer<Network> m_network;
    bool m_debug{true};
};
#endif // MAINWINDOW_H
