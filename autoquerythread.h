#ifndef AUTOQUERYTHREAD_H
#define AUTOQUERYTHREAD_H

#include <QObject>
#include <QThread>
class autoquerythread : public QThread
{
    Q_OBJECT
public:
     autoquerythread();

protected:
    void run();

public slots:


signals:
    void queryLockStatus();
};

#endif // AUTOQUERYTHREAD_H
