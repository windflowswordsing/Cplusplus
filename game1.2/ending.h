#ifndef ENDING_H
#define ENDING_H

#include <QString>
#include <QColor>

struct EndingData {
    QString name;
    QString desc;
    QColor bgColor;
    QColor textColor;
    QString effectHint; // 像素特效描述
};

class Ending
{
public:
    static EndingData getEnding(int type);
};

#endif // ENDING_H
