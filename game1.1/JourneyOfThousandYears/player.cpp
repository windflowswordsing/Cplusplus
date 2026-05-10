#include "player.h"

Player::Player()
{
    m_rect = QRect(100, 300, 32, 48);
    vx = vy = 0;
    onGround = false;
    sceneId = 0;
}

void Player::update()
{
    vy += GRAVITY;
    if (vy > 15) vy = 15;
    m_rect.translate(vx, vy);

    if (m_rect.bottom() > 620) {
        m_rect.moveBottom(620);
        vy = 0;
        onGround = true;
    } else {
        onGround = false;
    }
}

void Player::keyPress(QKeyEvent *e)
{
    if (e->key() == Qt::Key_A || e->key() == Qt::Key_Left)
        vx = -SPEED;
    if (e->key() == Qt::Key_D || e->key() == Qt::Key_Right)
        vx = SPEED;
    if ((e->key() == Qt::Key_Space || e->key() == Qt::Key_W) && onGround) {
        vy = JUMP;
        onGround = false;
    }
}

void Player::keyRelease(QKeyEvent *e)
{
    if (e->key() == Qt::Key_A || e->key() == Qt::Key_Left) vx = 0;
    if (e->key() == Qt::Key_D || e->key() == Qt::Key_Right) vx = 0;
}