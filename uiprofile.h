#ifndef UIPROFILE_H
#define UIPROFILE_H

#include <QString>

class QWidget;

class UiProfile
{
public:
    static QString resolution1920();
    static QString resolution1280();
    static QString currentResolution();
    static bool isCompact1280();
    static void apply(QWidget *root, const QString &formName);
};

#endif // UIPROFILE_H
