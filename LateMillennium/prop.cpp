#include "prop.h"

Prop::Prop(QString id, int x, int y, int w, int h, QColor c, QString label)
    : m_id(id), m_color(c), m_picked(false), m_label(label)
{
    m_rect = QRect(x, y, w, h);
}
