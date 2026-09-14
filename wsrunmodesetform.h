#ifndef WSRUNMODESETFORM_H
#define WSRUNMODESETFORM_H

#include <QWidget>
#include "config.h"

namespace Ui {
class WsRunModeSetForm;
}

class WsRunModeSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit WsRunModeSetForm(QWidget *parent = 0);
    ~WsRunModeSetForm();

public:
    void saveConfig();

private:
    Config *pConfig;
    void initForms();
    QString getLocalIP();


private:
    Ui::WsRunModeSetForm *ui;
};

#endif // WSRUNMODESETFORM_H
