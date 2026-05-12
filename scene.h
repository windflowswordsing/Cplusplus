#ifndef SCENE_H
#define SCENE_H

#include <QList>
#include <QRect>
#include <QColor>
#include <QString>
#include "prop.h"
#include "npc.h"

// 场景出口定义
struct SceneExit {
    QRect rect;       // 出口触发区域
    int targetScene;  // 目标场景ID
    int spawnX;       // 目标出生X
    int spawnY;       // 目标出生Y
    QString requireProp; // 需要的道具（空=无条件）
};

class Scene
{
public:
    Scene();

    void load(int sceneId);
    QList<QRect> platforms() const { return m_platforms; }
    QList<Prop>& props() { return m_props; }
    const QList<Prop>& props() const { return m_props; }
    QList<Npc>& npcs() { return m_npcs; }
    const QList<Npc>& npcs() const { return m_npcs; }
    QList<SceneExit> exits() const { return m_exits; }

    // 场景视觉属性
    QColor bgColor() const { return m_bgColor; }
    QColor groundColor() const { return m_groundColor; }
    QString name() const { return m_name; }
    QString hint() const { return m_hint; }

    // 回溯相关
    QString flashbackTrigger() const { return m_flashbackTrigger; } // 触发回溯的道具/NPC id
    QColor flashbackColor() const { return m_flashbackColor; }     // 回溯暖色调

private:
    QList<QRect> m_platforms;
    QList<Prop> m_props;
    QList<Npc> m_npcs;
    QList<SceneExit> m_exits;
    QColor m_bgColor;
    QColor m_groundColor;
    QString m_name;
    QString m_hint;
    QString m_flashbackTrigger;
    QColor m_flashbackColor;
};

#endif // SCENE_H
