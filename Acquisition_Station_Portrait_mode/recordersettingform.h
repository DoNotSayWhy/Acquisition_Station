#ifndef RECORDERSETTINGFORM_H
#define RECORDERSETTINGFORM_H

#include <QWidget>
#include <string>
#include <vector>
#include <unistd.h>
#include <set>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

using namespace std;

namespace Ui {
class RecorderSettingForm;
}

class RecorderSettingForm : public QWidget
{
    Q_OBJECT

public:
    explicit RecorderSettingForm(QWidget* pWidget);
    ~RecorderSettingForm();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int32_t myexec(const char *cmd, vector<string> &resvec);
    QStringList getUsbPaths();

public slots:
    void pushButtonClickConfirmModify();
    void pushButtonClickReadParam();



private:
    Ui::RecorderSettingForm *ui;
};


#endif // RECORDERSETTINGFORM_H
