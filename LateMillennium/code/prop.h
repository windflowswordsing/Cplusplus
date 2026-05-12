#ifndef PROP_H
#define PROP_H

#include <QRect>
#include <QString>
#include <QColor>

// 道具状态枚举
enum class PropState {
    Normal,      // 正常状态（未调查）
    Investigated // 已调查状态
};

class Prop
{
public:
    Prop(QString id, int x, int y, int w, int h, QColor c, QString label,
         QString normalImage = "", QString investigatedImage = "");

    QRect rect() const { return m_rect; }
    QString id() const { return m_id; }
    QColor color() const { return m_color; }
    QString label() const { return m_label; }

    // 状态管理
    PropState state() const { return m_state; }
    void setState(PropState state) { m_state = state; }
    bool isInvestigated() const { return m_state == PropState::Investigated; }
    void setInvestigated(bool v) { m_state = v ? PropState::Investigated : PropState::Normal; }

    // 图片路径
    QString currentImagePath() const;
    QString normalImagePath() const { return m_normalImage; }
    QString investigatedImagePath() const { return m_investigatedImage; }

    // 交互标记
    bool canInteract() const { return m_canInteract; }
    void setCanInteract(bool v) { m_canInteract = v; }

private:
    QString m_id;
    QRect m_rect;
    QColor m_color;
    QString m_label;
    PropState m_state;
    QString m_normalImage;       // 正常状态图片
    QString m_investigatedImage; // 已调查状态图片
    bool m_canInteract;
};

#endif // PROP_H
