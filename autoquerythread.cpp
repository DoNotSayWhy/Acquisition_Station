#include "autoquerythread.h"

autoquerythread::autoquerythread()
{

}
void autoquerythread::run(){

    while (true) {
        emit queryLockStatus();
        QThread::msleep(1000);
    }

}
