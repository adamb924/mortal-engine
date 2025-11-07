#include "mortal-engine_global.h"

#include <QString>
#include <QRegularExpression>

QString toOneLine(QString string)
{
    static QRegularExpression REGEX("\\s+", QRegularExpression::UseUnicodePropertiesOption);
    return string.replace(REGEX," ").trimmed();
}
