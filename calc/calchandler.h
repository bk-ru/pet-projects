#ifndef CALCHANDLER_H
#define CALCHANDLER_H

#include <QObject>
#include <QString>

class CalcHandler : public QObject
{
    Q_OBJECT
public:
    explicit CalcHandler(QObject *parent = nullptr);
};

#endif // CALCHANDLER_H
