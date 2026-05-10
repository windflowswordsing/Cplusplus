#ifndef NPC_H
#define NPC_H

#include <QRect>
#include <QString>
#include <QColor>

class Npc
{
public:
    Npc(QString id, int x, int y, int w = 40, int h = 60, QColor c = Qt::yellow, QString label = "");

    QRect rect() const { return m_rect; }
    QString id() const { return m_id; }
    QColor color() const { return m_color; }
    QString label() const { return m_label; }
    bool talked() const { return m_talked; }
    void setTalked(bool v) { m_talked = v; }

private:
    QString m_id;
    QRect m_rect;
    QColor m_color;
    QString m_label;
    bool m_talked;
};

#endif // NPC_H
