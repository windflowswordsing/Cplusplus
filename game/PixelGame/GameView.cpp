#include "GameView.h"
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QBrush>

// ============== 玩家类 ==============
class Player : public QGraphicsRectItem {
public:
    Player() {
        setRect(0, 0, 32, 48);           // 像素风角色大小
        setBrush(Qt::red);               // 临时红色方块，可换成 QPixmap(":/assets/player.png")
        vx = vy = 0;
        onGround = false;
    }

    void update() {
        vx *= 0.85;                      // 摩擦
        vy += 0.8;                       // 重力
        if (vy > 15) vy = 15;

        setPos(x() + vx, y() + vy);

        // 简单地面碰撞
        if (y() > 480) {
            setPos(x(), 480);
            vy = 0;
            onGround = true;
        }
    }

    qreal vx, vy;
    bool onGround;
};

// ============== GameView ==============
GameView::GameView(QWidget *parent) : QGraphicsView(parent)
{
    scene = new QGraphicsScene(this);
    setScene(scene);
    setRenderHint(QPainter::Antialiasing, false);
    setRenderHint(QPainter::SmoothPixmapTransform, false);
    setBackgroundBrush(Qt::black);
    setCacheMode(QGraphicsView::CacheBackground);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setFocusPolicy(Qt::StrongFocus);

    // 游戏循环
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (player) player->update();
        scene->update();
    });
    timer->start(16);   // ~60FPS

    resetScene(0);      // 初始场景
}

void GameView::resetScene(int sceneType)
{
    scene->clear();

    QString bgPath, sceneName;
    switch(sceneType) {
    case 0: bgPath = ":/assets/church_bg.png"; sceneName = "破败教堂"; break;
    case 1: bgPath = ":/assets/dungeon_bg.png"; sceneName = "赛博地下城"; break;
    case 2: bgPath = ":/assets/pyramid_bg.png"; sceneName = "沙漠金字塔"; break;
    case 3: bgPath = ":/assets/volcano_bg.png"; sceneName = "末日火山"; break;
    case 4: bgPath = ":/assets/library_bg.png"; sceneName = "图书馆秘境"; break;
    case 5: bgPath = ":/assets/finalchurch_bg.png"; sceneName = "时空终局教堂"; break;
    default: bgPath = ":/assets/church_bg.png"; sceneName = "默认"; break;
    }

    QPixmap bgPix(bgPath);
    if (!bgPix.isNull()) {
        setBackgroundBrush(bgPix);
    } else {
        setBackgroundBrush(Qt::darkGray);
        qWarning() << "背景加载失败：" << bgPath;
    }

    // 创建玩家
    player = new Player();
    player->setPos(100, 300);
    scene->addItem(player);

    // 创建简单平台（示例）
    QGraphicsRectItem *plat1 = new QGraphicsRectItem(200, 400, 300, 20);
    plat1->setBrush(Qt::blue);
    scene->addItem(plat1);

    QGraphicsRectItem *plat2 = new QGraphicsRectItem(600, 300, 200, 20);
    plat2->setBrush(Qt::blue);
    scene->addItem(plat2);

    qDebug() << "✅ 切换到场景：" << sceneName;
}

void GameView::keyPressEvent(QKeyEvent *e)
{
    if (!player) return;
    switch(e->key()) {
    case Qt::Key_A: case Qt::Key_Left:  player->vx = -6; break;
    case Qt::Key_D: case Qt::Key_Right: player->vx = 6; break;
    case Qt::Key_Space:
        if (player->onGround) { player->vy = -16; player->onGround = false; }
        break;
    case Qt::Key_1: resetScene(0); break;
    case Qt::Key_2: resetScene(1); break;
    case Qt::Key_3: resetScene(2); break;
    case Qt::Key_4: resetScene(3); break;
    case Qt::Key_5: resetScene(4); break;
    case Qt::Key_6: resetScene(5); break;
    }
}

void GameView::keyReleaseEvent(QKeyEvent *e)
{
    if (!player) return;
    if (e->key() == Qt::Key_A || e->key() == Qt::Key_Left ||
        e->key() == Qt::Key_D || e->key() == Qt::Key_Right) {
        // player->vx = 0;  // 更平滑的话可保留摩擦
    }
}
