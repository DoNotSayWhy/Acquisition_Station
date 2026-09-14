#include "hotplugthread.h"


HotPlugThread::HotPlugThread(DASBuddy **buddy)
{

}


void HotPlugThread::run()
{
    while (true) {

        QApplication::processEvents(QEventLoop::AllEvents);


        if(is_mainwindow_exited){
            break;
        }

        emit insertZFY();
        QThread::msleep(4000);

    }

}




void HotPlugThread::hswork()
{
    while (true) {
        emit insertZFY();
        QThread::msleep(1000);
    }

}
