#ifndef WSINTERFACESETFORM_H
#define WSINTERFACESETFORM_H

#include <QWidget>
#include "config.h"

namespace Ui {
class WsInterfaceSetForm;
}

class WsInterfaceSetForm : public QWidget
{
    Q_OBJECT

public:
    explicit WsInterfaceSetForm(QWidget *parent = 0);
    ~WsInterfaceSetForm();

public:
    bool saveConfig(bool showSuccessMessage = true);

private:
    Config *pConfig;
    void initForms();


private:
    Ui::WsInterfaceSetForm *ui;
};

#endif // WSINTERFACESETFORM_H
