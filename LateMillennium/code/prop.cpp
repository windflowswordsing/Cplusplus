#include "prop.h"

Prop::Prop(QString id, int x, int y, int w, int h, QColor c, QString label,
           QString normalImage, QString investigatedImage)
    : m_id(id), m_color(c), m_label(label),
      m_state(PropState::Normal),
      m_normalImage(normalImage),
      m_investigatedImage(investigatedImage),
      m_canInteract(true)
{
    m_rect = QRect(x, y, w, h);
}

QString Prop::currentImagePath() const
{
    if (m_state == PropState::Investigated && !m_investigatedImage.isEmpty()) {
        return m_investigatedImage;
    }
    return m_normalImage;
}
