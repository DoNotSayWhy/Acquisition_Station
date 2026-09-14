#ifndef WORKSTATIONSETFORM_H
#define WORKSTATIONSETFORM_H

#include <QWidget>



namespace Ui {
class WorkStationSetForm;
}

class WorkStationSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit WorkStationSetForm(QWidget *parent = 0);
    ~WorkStationSetForm();



signals:
    void signalStopMainWindowDisk();

private:
    void initForms();
    void initConnectSignals();

public slots:
    void pushButtonWsSet();
    void pushButtonWsRunMode();
    void pushButtonWsInterface();
    void pushButtonWsSaveConfig();
    void pushButtonWsReboot();


private slots:
    void on_pushButton_w_reboot_clicked();

private:
    Ui::WorkStationSetForm *ui;
};

#endif // WORKSTATIONSETFORM_H
