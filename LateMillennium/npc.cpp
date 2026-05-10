#include "npc.h"

Npc::Npc(QString id, int x, int y, int w, int h, QColor c, QString label)
    : m_id(id), m_color(c), m_label(label), m_talked(false)
{
    m_rect = QRect(x, y, w, h);
}
