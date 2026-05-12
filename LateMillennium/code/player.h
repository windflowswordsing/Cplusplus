#ifndef PLAYER_H
#define PLAYER_H

#include <QRect>
#include <QStringList>
#include <QKeyEvent>

class Player {
public:
    Player();
    void reset();
    void update();
    void land();
    void keyPress(QKeyEvent *e);
    void keyRelease(QKeyEvent *e);

    QRect& rect() { return m_rect; }
    const QRect& rect() const { return m_rect; }
    void setPos(int x, int y);

    float getVx() const { return m_vx; }
    float getVy() const { return m_vy; }

    QStringList props() const { return m_props; }
    void addProp(const QString &p);
    bool hasProp(const QString &p) const;
    void clearProps() { m_props.clear(); }

    int sceneId;
    bool facingRight;

private:
    QRect m_rect;
    float m_vx, m_vy;
    bool m_onGround;
    bool m_keyLeft, m_keyRight;
    QStringList m_props;
};

#endif
