#include "npc.h"

Npc::Npc(QString id, int x, int y, int w, int h, QColor c, QString label,
         QString normalImage, QString talkedImage)
    : m_id(id), m_color(c), m_label(label),
      m_state(NpcState::Normal),
      m_normalImage(normalImage),
      m_talkedImage(talkedImage),
      m_canInteract(true)
{
    m_rect = QRect(x, y, w, h);
}

QString Npc::currentImagePath() const
{
    if (m_state == NpcState::Talked && !m_talkedImage.isEmpty()) {
        return m_talkedImage;
    }
    return m_normalImage;
}
