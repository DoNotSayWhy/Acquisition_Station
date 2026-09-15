#include "uiprofile.h"

#include "config.h"

#include <QFile>
#include <QFont>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QSizePolicy>
#include <QStringList>
#include <QWidget>
#include <QXmlStreamReader>
#include <QDebug>

namespace {

struct UiValue
{
    enum Kind { Invalid, Number, Text, Rect, Size, Font, SizePolicyValue } kind = Invalid;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int number = 0;
    int horizontalStretch = 0;
    int verticalStretch = 0;
    QString text;
    QString horizontalPolicy;
    QString verticalPolicy;
    QFont font;
};

int readIntegerElement(QXmlStreamReader &xml)
{
    return xml.readElementText().trimmed().toInt();
}

UiValue readPropertyValue(QXmlStreamReader &xml)
{
    UiValue value;
    if (!xml.readNextStartElement()) return value;

    const QString type = xml.name().toString();
    if (type == QLatin1String("rect")) {
        value.kind = UiValue::Rect;
        while (xml.readNextStartElement()) {
            const QString name = xml.name().toString();
            const int item = readIntegerElement(xml);
            if (name == QLatin1String("x")) value.x = item;
            else if (name == QLatin1String("y")) value.y = item;
            else if (name == QLatin1String("width")) value.width = item;
            else if (name == QLatin1String("height")) value.height = item;
        }
    } else if (type == QLatin1String("size")) {
        value.kind = UiValue::Size;
        while (xml.readNextStartElement()) {
            const QString name = xml.name().toString();
            const int item = readIntegerElement(xml);
            if (name == QLatin1String("width")) value.width = item;
            else if (name == QLatin1String("height")) value.height = item;
        }
    } else if (type == QLatin1String("font")) {
        value.kind = UiValue::Font;
        QFont font;
        while (xml.readNextStartElement()) {
            const QString name = xml.name().toString();
            const QString item = xml.readElementText().trimmed();
            if (name == QLatin1String("family")) font.setFamily(item);
            else if (name == QLatin1String("pointsize")) font.setPointSize(item.toInt());
            else if (name == QLatin1String("weight")) font.setWeight(item.toInt());
            else if (name == QLatin1String("bold")) font.setBold(item == QLatin1String("true"));
            else if (name == QLatin1String("italic")) font.setItalic(item == QLatin1String("true"));
            else if (name == QLatin1String("underline")) font.setUnderline(item == QLatin1String("true"));
            else if (name == QLatin1String("strikeout")) font.setStrikeOut(item == QLatin1String("true"));
        }
        value.font = font;
    } else if (type == QLatin1String("sizepolicy")) {
        value.kind = UiValue::SizePolicyValue;
        value.horizontalPolicy = xml.attributes().value(QLatin1String("hsizetype")).toString();
        value.verticalPolicy = xml.attributes().value(QLatin1String("vsizetype")).toString();
        while (xml.readNextStartElement()) {
            const QString name = xml.name().toString();
            const int item = readIntegerElement(xml);
            if (name == QLatin1String("horstretch")) value.horizontalStretch = item;
            else if (name == QLatin1String("verstretch")) value.verticalStretch = item;
        }
    } else if (type == QLatin1String("number")) {
        value.kind = UiValue::Number;
        value.number = xml.readElementText().trimmed().toInt();
    } else if (type == QLatin1String("string") || type == QLatin1String("enum") ||
               type == QLatin1String("set") || type == QLatin1String("bool")) {
        value.kind = UiValue::Text;
        value.text = xml.readElementText();
    } else {
        xml.skipCurrentElement();
    }

    while (xml.readNextStartElement()) xml.skipCurrentElement();
    return value;
}

QSizePolicy::Policy policyFromName(const QString &name)
{
    if (name == QLatin1String("Fixed")) return QSizePolicy::Fixed;
    if (name == QLatin1String("Minimum")) return QSizePolicy::Minimum;
    if (name == QLatin1String("Maximum")) return QSizePolicy::Maximum;
    if (name == QLatin1String("Preferred")) return QSizePolicy::Preferred;
    if (name == QLatin1String("MinimumExpanding")) return QSizePolicy::MinimumExpanding;
    if (name == QLatin1String("Expanding")) return QSizePolicy::Expanding;
    if (name == QLatin1String("Ignored")) return QSizePolicy::Ignored;
    return QSizePolicy::Preferred;
}

QString compactResourceStyle(QString style)
{
    const QStringList compactImages = {
        QLatin1String("Dataquery8303.png"), QLatin1String("asm6232.png"),
        QLatin1String("asm8315.png"), QLatin1String("backdrop.png")
    };
    for (const QString &name : compactImages) {
        style.replace(QLatin1String(":/image/") + name,
                      QLatin1String(":/ui_profiles/1280x1024/image/") + name);
    }
    return style;
}

Qt::Alignment alignmentFromName(const QString &name)
{
    Qt::Alignment alignment;
    const QStringList parts = name.split(QLatin1Char('|'), QString::SkipEmptyParts);
    for (const QString &partValue : parts) {
        const QString part = partValue.trimmed();
        if (part == QLatin1String("Qt::AlignLeft")) alignment |= Qt::AlignLeft;
        else if (part == QLatin1String("Qt::AlignRight")) alignment |= Qt::AlignRight;
        else if (part == QLatin1String("Qt::AlignHCenter")) alignment |= Qt::AlignHCenter;
        else if (part == QLatin1String("Qt::AlignJustify")) alignment |= Qt::AlignJustify;
        else if (part == QLatin1String("Qt::AlignTop")) alignment |= Qt::AlignTop;
        else if (part == QLatin1String("Qt::AlignBottom")) alignment |= Qt::AlignBottom;
        else if (part == QLatin1String("Qt::AlignVCenter")) alignment |= Qt::AlignVCenter;
        else if (part == QLatin1String("Qt::AlignCenter")) alignment |= Qt::AlignCenter;
        else if (part == QLatin1String("Qt::AlignLeading")) alignment |= Qt::AlignLeading;
        else if (part == QLatin1String("Qt::AlignTrailing")) alignment |= Qt::AlignTrailing;
    }
    return alignment;
}

void applyWidgetProperty(QWidget *widget, const QString &propertyName,
                         const UiValue &value, QWidget *root, const QString &formName)
{
    if (!widget) return;

    if (propertyName == QLatin1String("geometry") && value.kind == UiValue::Rect) {
        QRect geometry(value.x, value.y, value.width, value.height);
        if (widget == root && formName == QLatin1String("mainform")) geometry.setSize(QSize(1280, 1024));
        widget->setGeometry(geometry);
    } else if (propertyName == QLatin1String("minimumSize") && value.kind == UiValue::Size) {
        widget->setMinimumSize(value.width, value.height);
    } else if (propertyName == QLatin1String("maximumSize") && value.kind == UiValue::Size) {
        widget->setMaximumSize(value.width, value.height);
    } else if (propertyName == QLatin1String("font") && value.kind == UiValue::Font) {
        widget->setFont(value.font);
    } else if (propertyName == QLatin1String("styleSheet") && value.kind == UiValue::Text) {
        widget->setStyleSheet(compactResourceStyle(value.text));
    } else if (propertyName == QLatin1String("sizePolicy") && value.kind == UiValue::SizePolicyValue) {
        QSizePolicy policy(policyFromName(value.horizontalPolicy), policyFromName(value.verticalPolicy));
        policy.setHorizontalStretch(value.horizontalStretch);
        policy.setVerticalStretch(value.verticalStretch);
        widget->setSizePolicy(policy);
    } else if (propertyName == QLatin1String("iconSize") && value.kind == UiValue::Size) {
        widget->setProperty("iconSize", QSize(value.width, value.height));
    } else if (propertyName == QLatin1String("alignment") && value.kind == UiValue::Text) {
        if (QLabel *label = qobject_cast<QLabel *>(widget)) {
            label->setAlignment(alignmentFromName(value.text));
        }
    }
}

void applyLayoutProperty(QLayout *layout, const QString &propertyName, const UiValue &value)
{
    if (!layout || value.kind != UiValue::Number) return;

    int left = 0, top = 0, right = 0, bottom = 0;
    layout->getContentsMargins(&left, &top, &right, &bottom);
    if (propertyName == QLatin1String("leftMargin")) layout->setContentsMargins(value.number, top, right, bottom);
    else if (propertyName == QLatin1String("topMargin")) layout->setContentsMargins(left, value.number, right, bottom);
    else if (propertyName == QLatin1String("rightMargin")) layout->setContentsMargins(left, top, value.number, bottom);
    else if (propertyName == QLatin1String("bottomMargin")) layout->setContentsMargins(left, top, right, value.number);
    else if (propertyName == QLatin1String("spacing")) layout->setSpacing(value.number);
    else if (propertyName == QLatin1String("horizontalSpacing")) {
        if (QGridLayout *grid = qobject_cast<QGridLayout *>(layout)) grid->setHorizontalSpacing(value.number);
    } else if (propertyName == QLatin1String("verticalSpacing")) {
        if (QGridLayout *grid = qobject_cast<QGridLayout *>(layout)) grid->setVerticalSpacing(value.number);
    }
}

QObject *profileObject(QWidget *root, const QString &objectName, bool layout)
{
    if (!root) return nullptr;
    if (!layout && root->objectName() == objectName) return root;
    return layout ? static_cast<QObject *>(root->findChild<QLayout *>(objectName))
                  : static_cast<QObject *>(root->findChild<QWidget *>(objectName));
}

void parseElement(QXmlStreamReader &xml, QWidget *root, const QString &formName, QObject *currentObject)
{
    const QString elementName = xml.name().toString();
    QObject *object = currentObject;
    if (elementName == QLatin1String("widget") || elementName == QLatin1String("layout")) {
        object = profileObject(root, xml.attributes().value(QLatin1String("name")).toString(),
                               elementName == QLatin1String("layout"));
    }

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("property") && object) {
            const QString propertyName = xml.attributes().value(QLatin1String("name")).toString();
            const UiValue value = readPropertyValue(xml);
            if (QWidget *widget = qobject_cast<QWidget *>(object)) {
                applyWidgetProperty(widget, propertyName, value, root, formName);
            } else if (QLayout *layout = qobject_cast<QLayout *>(object)) {
                applyLayoutProperty(layout, propertyName, value);
            }
        } else {
            parseElement(xml, root, formName, object);
        }
    }
}

} // namespace

QString UiProfile::resolution1920()
{
    return QLatin1String("1920 * 1080");
}

QString UiProfile::resolution1280()
{
    return QLatin1String("1280 * 1024");
}

QString UiProfile::currentResolution()
{
    const QString configured = Config::getInstance()->Get("wsConfig", "reslution").toString().simplified();
    QString normalized = configured;
    normalized.remove(QLatin1Char(' '));
    normalized.replace(QLatin1Char('x'), QLatin1Char('*'), Qt::CaseInsensitive);
    if (normalized == QLatin1String("1280*1024")) return resolution1280();
    return resolution1920();
}

bool UiProfile::isCompact1280()
{
    return currentResolution() == resolution1280();
}

void UiProfile::apply(QWidget *root, const QString &formName)
{
    if (!root || !isCompact1280()) return;

    QFile file(QLatin1String(":/ui_profiles/1280x1024/forms/") + formName + QLatin1String(".ui"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "1280x1024 UI profile is unavailable for" << formName;
        return;
    }

    QXmlStreamReader xml(&file);
    if (xml.readNextStartElement()) parseElement(xml, root, formName, nullptr);
    if (xml.hasError()) qWarning() << "Failed to read 1280x1024 UI profile" << formName << xml.errorString();
}
