#ifndef WSPAIRDEVICEFORM_H
#define WSPAIRDEVICEFORM_H

#include <QWidget>
#include <QVector>
#include <QPainter>
#include "wspairdevicethread.h"
#include "deviceportform.h"
#include "config.h"

namespace Ui {
class WsPairDeviceForm;
}

class WsPairDeviceForm : public QWidget
{
    Q_OBJECT

public:
    explicit WsPairDeviceForm(int count = 20,QWidget *parent = 0);
    ~WsPairDeviceForm();
    int deviceCount;
    void setPairDeviceCount(int count){this->deviceCount = count;}
private:
    Config *pConfig;
    QVector<DevicePortForm *> deviceForm;
    QString currentPairPort;
    WsPairDeviceThread *deviceThread;
    void initForms();
    void initConnects();
    void checkConfigPortInfo();
    void updatePortraitLayout();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void pushButtonClickBack();
    void toInsertRecorder(QString);
    void toDeleteRecorder(QString);
    void pushButtonPairDevice(QString portNum);

private:
    Ui::WsPairDeviceForm *ui;
};

#endif // WSPAIRDEVICEFORM_H
