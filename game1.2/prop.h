#ifndef PROP_H
#define PROP_H

#include <QRect>
#include <QColor>
#include <QString>

class Prop
{
public:
    Prop(QString id, int x, int y, int w, int h, QColor c = Qt::yellow, QString label = "");

    QRect rect() const { return m_rect; }
    QString id() const { return m_id; }
    QColor color() const { return m_color; }
    bool picked() const { return m_picked; }
    void pick() { m_picked = true; }
    void reset() { m_picked = false; }
    QString label() const { return m_label; }

private:
    QString m_id;
    QRect m_rect;
    QColor m_color;
    bool m_picked;
    QString m_label;
};

#endif // PROP_H
