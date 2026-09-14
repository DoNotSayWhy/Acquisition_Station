#ifndef WSSETFORM_H
#define WSSETFORM_H

#include <QWidget>
#include <QList>
#include <QPair>
#include "config.h"

namespace Ui {
class WsSetForm;
}

class WsSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit WsSetForm(QWidget *parent = 0);
    ~WsSetForm();

public:
    void saveConfig();
    QStringList getMediaMountPoints();


private:
    Config *pConfig;
    void initForms();
    QList<QPair<QString, QString>> getMountPoints();
    std::vector<int> getAvailableCameras();

public slots:
    void pushButtonPairDeviceClick();
    void comboBoxCurrentIndexChanged(int index);

private:
    Ui::WsSetForm *ui;
};

#endif // WSSETFORM_H
