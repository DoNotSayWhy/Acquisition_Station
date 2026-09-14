#ifndef DEVICEPORTFORM_H
#define DEVICEPORTFORM_H

#include <QWidget>

namespace Ui {
class DevicePortForm;
}

class DevicePortForm : public QWidget
{
    Q_OBJECT

public:
    explicit DevicePortForm(QWidget *parent = 0);
    ~DevicePortForm();
    void setPortNum(QString txt);
    void setPairStatus(int status);
    int getPairStatus();
private:
    QString currentPortNum;
    //0:not pair. 1: paired.
    int pairStatus;

public slots:
    void pushButtonPairDeviceClick();

signals:
    void pushButtonPairDevice(QString portNum);



private:
    Ui::DevicePortForm *ui;
};

#endif // DEVICEPORTFORM_H
