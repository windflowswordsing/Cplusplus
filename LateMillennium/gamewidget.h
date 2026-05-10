#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QDateTime>
#include <QMouseEvent>
#include <QPixmap>
#include <QSet>
#include "player.h"
#include "scene.h"
#include "dialogue.h"
#include "ending.h"

// 游戏状态
enum class GameState {
    Title,       // 标题画面
    Playing,     // 正常游戏
    Dialog,      // 对话中
    Flashback,   // 回溯动画
    Choice,      // 选择分支
    Ending       // 结局展示
};

class GameWidget : public QWidget
{
    Q_OBJECT
public:
    GameWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void timerEvent(QTimerEvent *) override;

private:
    // === 游戏逻辑 ===
    void updateGame();
    void checkCollisions();
    void handleInteraction(const QString &action);

    // === 场景切换 ===
    void changeScene(int targetScene, int spawnX, int spawnY);

    // === 回溯系统 ===
    void startFlashback(const QString &trigger);
    void updateFlashback();

    // === 结局 ===
    void triggerEnding(int type);

    // === 渲染 ===
    void drawTitle(QPainter &p);
    void drawGame(QPainter &p);
    void drawPlayer(QPainter &p);
    void drawProps(QPainter &p);
    void drawNpcs(QPainter &p);
    void drawDialog(QPainter &p);
    void drawFlashback(QPainter &p);
    void drawEnding(QPainter &p);
    void drawHUD(QPainter &p);
    void drawHintButton(QPainter &p);
    void drawPixelRect(QPainter &p, QRect r, QColor c);
    void drawPixelGround(QPainter &p);
    void drawPixelPlatform(QPainter &p, QRect plat, int sceneId);
    void drawScrollingBg(QPainter &p);  // 卷轴背景（视差滚动）

    // === 图片处理 ===
    static QPixmap removeBackground(const QPixmap &pixmap, QColor bgColor, int threshold = 30); // 自动去底色

    // === 交互检测 ===
    bool isNearInteractable() const; // 检测玩家是否靠近可交互对象

    // === 核心对象 ===
    Player player;
    Scene scene;
    Dialogue dialogue;

    // === 游戏状态 ===
    GameState state;
    int camX;

    // === 对话系统 ===
    DialogData currentDialog;
    QString currentDialogKey;
    bool dialogHasChoices;

    // === 回溯动画 ===
    int flashbackTimer;     // 回溯动画计时（帧数）
    static const int FLASHBACK_DURATION = 180; // 3秒@60fps
    QColor flashbackBgColor;
    QString flashbackText;

    // === 结局 ===
    int endingType;
    int endingTimer;

    // === 标题画面 ===
    int titleStarCount;

    // === 标记 ===
    bool flashbackUsed;
    bool showingFlashbackBg;  // 是否正在显示回溯盛景背景
    QSet<int> flashbackUsedScenes;  // 已使用过回溯的场景ID集合
    bool churchStartShown;  // 教堂初始独白是否已显示
    bool loopShockShown;    // 终局震惊独白是否已显示
    bool swordPicked;       // 断剑是否已拾取
    bool seedPicked;        // 种子是否已拾取
    bool lobsterSaved;      // 龙虾是否已救（触发对话后）
    bool lobsterFreed;      // 龙虾是否已释放（第一次互动，切换图片）
    bool wellCleared;       // 古井是否已疏通（场景0）

    // === 提示按钮 ===
    bool showHintButton;    // 是否显示提示按钮
    QRect hintButtonRect;   // 提示按钮区域
    QString hintText;       // 提示内容
    bool showHintPanel;     // 是否显示提示面板

    // === 操作提示（首次进入游戏） ===
    bool controlHintShown;  // 操作提示是否已显示过
    int controlHintTimer;   // 操作提示显示计时（帧数）

    // === 背景图片 ===
    QPixmap bgChurch;               // 教堂背景图（废墟）
    QList<QPixmap> sceneBg;         // 6个场景的废墟背景 [0~5]
    QList<QPixmap> sceneBgFlashback;// 5个场景的回溯盛景背景 [0~4]

    // === 玩家精灵图 ===
    QPixmap playerIdle;         // 静止帧
    QList<QPixmap> playerWalkR; // 向右走3帧
    QList<QPixmap> playerWalkL; // 向左走3帧
    int animFrame;              // 当前动画帧
    int animTimer;              // 动画计时器
    bool spritesLoaded;         // 精灵图是否加载成功
};

#endif // GAMEWIDGET_H
