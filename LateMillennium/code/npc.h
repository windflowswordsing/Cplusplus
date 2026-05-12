#ifndef NPC_H
#define NPC_H

#include <QRect>
#include <QString>
#include <QColor>

// NPC状态枚举
enum class NpcState {
    Normal,   // 正常状态（未对话）
    Talked    // 已对话状态
};

class Npc
{
public:
    Npc(QString id, int x, int y, int w, int h, QColor c, QString label,
        QString normalImage = "", QString talkedImage = "");

    QRect rect() const { return m_rect; }
    QString id() const { return m_id; }
    QColor color() const { return m_color; }
    QString label() const { return m_label; }

    // 状态管理
    NpcState state() const { return m_state; }
    void setState(NpcState state) { m_state = state; }
    bool talked() const { return m_state == NpcState::Talked; }
    void setTalked(bool v) { m_state = v ? NpcState::Talked : NpcState::Normal; }

    // 图片路径
    QString currentImagePath() const;
    QString normalImagePath() const { return m_normalImage; }
    QString talkedImagePath() const { return m_talkedImage; }

    // 交互标记
    bool canInteract() const { return m_canInteract; }
    void setCanInteract(bool v) { m_canInteract = v; }

private:
    QString m_id;
    QRect m_rect;
    QColor m_color;
    QString m_label;
    NpcState m_state;
    QString m_normalImage;  // 正常状态图片
    QString m_talkedImage;  // 已对话状态图片
    bool m_canInteract;
};

#endif // NPC_H
