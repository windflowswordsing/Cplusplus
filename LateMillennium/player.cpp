#include "player.h"

Player::Player()
{
    m_rect = QRect(100, 300, 24, 40);
    vx = vy = 0;
    onGround = false;
    sceneId = 0;
}

void Player::update()
{
    vy += GRAVITY;
    if (vy > 15) vy = 15;
    m_rect.translate(vx, vy);

    // 地面限制
    if (m_rect.bottom() > 620) {
        m_rect.moveBottom(620);
        vy = 0;
        onGround = true;
    } else {
        onGround = false;
    }

    // 画面边缘限制（1280x720）
    if (m_rect.left() < 0) m_rect.moveLeft(0);
    if (m_rect.right() > 1280) m_rect.moveRight(1280);
}

void Player::keyPress(QKeyEvent *e)
{
    if (e->key() == Qt::Key_A || e->key() == Qt::Key_Left) {
        vx = -SPEED;
        facingRight = false;
    }
    if (e->key() == Qt::Key_D || e->key() == Qt::Key_Right) {
        vx = SPEED;
        facingRight = true;
    }
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
