#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSettings()
{
    ui->displayRes->clear();

    QObject::connect(ui->groupNums,
                     static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked),
                             this, &MainWindow::on_groupNumbers_clicked);
    QObject::connect(ui->groupStandOper,
                     static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked),
                             this, &MainWindow::on_groupOper_clicked);
}

void MainWindow::on_groupNumbers_clicked(QAbstractButton *button)
{
    QString displayStr = ui->displayRes->text();

    if (_operClicked) {
        displayStr.clear();
        _operClicked = false;
    }

    if (displayStr.length() >= _digitLimit) {
        qDebug() << "Character limit exceeded";
        return;
    }

    displayStr.append(button->text());
    ui->displayRes->setText(displayStr);
}

void MainWindow::on_groupOper_clicked(QAbstractButton *button)
{
    if (!_operClicked) {
        QString displayStr = ui->displayRes->text();
        if (!displayStr.isEmpty()) {
            _storedNum = displayStr.toDouble();
            _hasStoredNum = true;
        }
    }

    _operClicked = true;
    _lastOper = button->text().at(0);
}


void MainWindow::on_comma_clicked()
{
    QString displayStr = ui->displayRes->text();

    if (displayStr.contains('.') || displayStr.length() >= (_digitLimit - 1))
        return;

    displayStr.append(displayStr.isEmpty() ? "0." : ".");
    ui->displayRes->setText(displayStr);
}

void MainWindow::on_buttonDel_clicked()
{
    QString displayStr = ui->displayRes->text();

    if (displayStr.isEmpty())
        return;

    displayStr.chop(1);
    ui->displayRes->setText(displayStr);
}


void MainWindow::on_operEqual_clicked()
{
    QString displayStr = ui->displayRes->text();

    if (!_hasStoredNum || displayStr.isEmpty())
        return;

    calcResult();
    _hasStoredNum = false;
    _operClicked = false;
}


void MainWindow::on_buttonC_clicked()
{
    ui->displayRes->clear();
    _operClicked = false;
    _hasStoredNum = false;
}

void MainWindow::on_operPercent_clicked()
{
    QString displayStr = ui->displayRes->text();
    if (displayStr.isEmpty()) return;

    double percentage = displayStr.toDouble() * 0.01;
    displayStr = QString::number(percentage, 'f', _digitLimit);

    ui->displayRes->setText(displayStr);

    _operClicked = false;
    _hasStoredNum = false;
}


void MainWindow::on_operSign_clicked()
{
    QString displayStr = ui->displayRes->text();
    if (displayStr.isEmpty()) return;

    double value = displayStr.toDouble();
    value = -value;

    displayStr = QString::number(value, 'g', _digitLimit);
    ui->displayRes->setText(displayStr);

    _operClicked = false;
}

void MainWindow::calcResult()
{
    QString displayStr = ui->displayRes->text();
    if (displayStr.endsWith('.')) {
        displayStr.chop(1);
    }

    double operand = displayStr.toDouble();

    if (_lastOper == '+') {
        _storedNum += operand;
    }
    else if (_lastOper == '-') {
        _storedNum -= operand;
    }
    else if (_lastOper == 'x') {
        _storedNum *= operand;
    }
    else if (_lastOper == '/') {
        if (qFuzzyCompare(operand, 0.0)) {
            ui->displayRes->setText("Ошибка: деление на 0");
            return;
        }
        _storedNum /= operand;
    }

    displayStr = QString::number(_storedNum, 'g', _digitLimit);
    ui->displayRes->setText(displayStr);
}


void MainWindow::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
        //Numbers
        case Qt::Key_1:
            on_groupNumbers_clicked(ui->num1);
            break;
        case Qt::Key_2:
            on_groupNumbers_clicked(ui->num2);
            break;
        case Qt::Key_3:
            on_groupNumbers_clicked(ui->num3);
            break;
        case Qt::Key_4:
            on_groupNumbers_clicked(ui->num4);
            break;
        case Qt::Key_5:
            on_groupNumbers_clicked(ui->num5);
            break;
        case Qt::Key_6:
            on_groupNumbers_clicked(ui->num6);
            break;
        case Qt::Key_7:
            on_groupNumbers_clicked(ui->num7);
            break;
        case Qt::Key_8:
            on_groupNumbers_clicked(ui->num8);
            break;
        case Qt::Key_9:
            on_groupNumbers_clicked(ui->num9);
            break;
        case Qt::Key_0:
            on_groupNumbers_clicked(ui->num0);
            break;
        //Operators
        case Qt::Key_Plus:
            on_groupOper_clicked(ui->operPlus);
            break;
        case Qt::Key_Minus:
            on_groupOper_clicked(ui->operMinus);
            break;
        case Qt::Key_Asterisk:
            on_groupOper_clicked(ui->operMult);
            break;
        case Qt::Key_Slash:
            on_groupOper_clicked(ui->operDiv);
            break;
        //Comma
        case Qt::Key_Period:
            on_comma_clicked();
            break;
        //Return (enter)
        case Qt::Key_Enter:
        case Qt::Key_Return:
            on_operEqual_clicked();
            break;
        //Backspace and delete
        case Qt::Key_Backspace:
            on_buttonDel_clicked();
            break;
        case Qt::Key_Delete:
            on_buttonC_clicked();
            break;
        //Percentage
        case Qt::Key_Percent:
            on_operPercent_clicked();
            break;
    }
}

