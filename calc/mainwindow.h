#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAbstractButton>
#include <QKeyEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#include <QMainWindow>
#include <QPushButton>

#include "calchandler.h"

class CalcHandler;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void setSettings();

private slots:
    void on_groupNumbers_clicked(QAbstractButton* button);
    void on_groupOper_clicked(QAbstractButton* button);

private slots:
    void on_comma_clicked();

private slots:
    void on_buttonDel_clicked();
    void on_buttonC_clicked();

private slots:
    void on_operPercent_clicked();
    void on_operSign_clicked();
    void on_operEqual_clicked();

private:
    void calcResult();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    Ui::MainWindow *ui;
    CalcHandler *calcHandler;

    const int _digitLimit = 10;
    bool _operClicked = false;
    QChar _lastOper = QChar();
    bool _hasStoredNum = false;
    double _storedNum = double();
};
#endif // MAINWINDOW_H
