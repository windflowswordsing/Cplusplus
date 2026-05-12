#include "player.h"

Player::Player() { reset(); }

void Player::reset() {
    sceneId = 0;
    facingRight = true;
    m_rect = QRect(100, 300, 32, 48);
    m_vx = 0; m_vy = 0;
    m_onGround = false;
    m_keyLeft = false; m_keyRight = false;
    m_props.clear();
}

void Player::update() {
    m_vy += 0.5f;
    if (m_vy > 12) m_vy = 12;
    if (m_keyLeft) { m_vx = -4; facingRight = false; }
    else if (m_keyRight) { m_vx = 4; facingRight = true; }
    else m_vx = 0;
    m_rect.moveLeft(m_rect.left() + (int)m_vx);
    m_rect.moveTop(m_rect.top() + (int)m_vy);
    if (m_rect.left() < 0) m_rect.moveLeft(0);
    if (m_rect.right() > 1280) m_rect.moveRight(1280);
    m_onGround = false;
}

void Player::land() { m_vy = 0; m_onGround = true; }

void Player::keyPress(QKeyEvent *e) {
    int k = e->key();
    if (k == Qt::Key_A || k == Qt::Key_Left) m_keyLeft = true;
    if (k == Qt::Key_D || k == Qt::Key_Right) m_keyRight = true;
    if ((k == Qt::Key_Space || k == Qt::Key_W || k == Qt::Key_Up) && m_onGround) {
        m_vy = -12;
        m_onGround = false;
    }
}

void Player::keyRelease(QKeyEvent *e) {
    int k = e->key();
    if (k == Qt::Key_A || k == Qt::Key_Left) m_keyLeft = false;
    if (k == Qt::Key_D || k == Qt::Key_Right) m_keyRight = false;
}

void Player::setPos(int x, int y) {
    m_rect.moveTo(x, y);
    m_vx = 0; m_vy = 0;
}

void Player::addProp(const QString &p) {
    if (!m_props.contains(p)) m_props.append(p);
}

bool Player::hasProp(const QString &p) const {
    return m_props.contains(p);
}
