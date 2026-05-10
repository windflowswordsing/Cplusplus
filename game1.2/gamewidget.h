#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QDateTime>
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
    void drawPixelRect(QPainter &p, QRect r, QColor c); // 像素风格矩形

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
    bool flashbackUsed;     // 回溯能力是否已耗尽（场景5后永久耗尽）
    bool churchStartShown;  // 教堂初始独白是否已显示
    bool loopShockShown;    // 终局震惊独白是否已显示
    bool swordPicked;       // 断剑是否已拾取
    bool seedPicked;        // 种子是否已拾取
    bool lobsterSaved;      // 龙虾是否已救
};

#endif // GAMEWIDGET_H
