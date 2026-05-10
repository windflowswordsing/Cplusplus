#ifndef PLAYER_H
#define PLAYER_H

#include <QRect>
#include <QKeyEvent>
#include <QStringList>

class Player
{
public:
    Player();

    void update();
    void keyPress(QKeyEvent *e);
    void keyRelease(QKeyEvent *e);

    QRect rect() const { return m_rect; }
    void setPos(int x, int y) { m_rect.moveTo(x, y); }

    void addProp(const QString &p) { if (!m_props.contains(p)) m_props << p; }
    bool hasProp(const QString &p) const { return m_props.contains(p); }
    QStringList props() const { return m_props; }
    void clearProps() { m_props.clear(); }

    int sceneId = 0; // 0=教堂 1=地下城 2=金字塔 3=火山 4=图书馆 5=终局

    bool facingRight = true;

private:
    QRect m_rect;
    int vx, vy;
    bool onGround;
    QStringList m_props;

    const int GRAVITY = 2;
    const int JUMP = -16;
    const int SPEED = 5;
};

#endif // PLAYER_H
