#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QDateTime>
#include <QMouseEvent>
#include <QPixmap>
#include <QSet>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "player.h"
#include "scene.h"
#include "dialogue.h"
#include "ending.h"

// 游戏状态
enum class GameState {
    Intro,       // 开场画面（诗句）- 保留声明，实际未使用
    Title,       // 标题画面
    Playing,     // 正常游戏
    Dialog,      // 对话中
    Flashback,   // 回溯动画 - 保留声明，实际未使用
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

    // === 重置游戏 ===
    void resetGame();

    // === 对话系统 ===
    void startDialog(const QString &key);
    void advanceDialog();
    void handleDialogEnd();
    void syncDialogBg();  // 根据对话类型同步背景（现实/回溯）

    // === 回溯系统（保留声明，实际由对话系统处理） ===
    void startFlashback(const QString &trigger);
    void startLibraryFlashback();
    void updateFlashback();

    // === 结局 ===
    void triggerEnding(int type);

    // === 渲染 ===
    void drawIntro(QPainter &p);       // 保留声明，实际未使用
    void drawTitle(QPainter &p);
    void drawGame(QPainter &p);
    void drawPlayer(QPainter &p);
    void drawProps(QPainter &p);
    void drawNpcs(QPainter &p);
    void drawDialog(QPainter &p);
    void drawFlashback(QPainter &p);   // 保留声明，实际未使用
    void drawEnding(QPainter &p);
    void drawHUD(QPainter &p);
    void drawPixelRect(QPainter &p, QRect r, QColor c);
    void drawPixelGround(QPainter &p);
    void drawScrollingBg(QPainter &p);

    // === 图片处理 ===
    static QPixmap removeBackground(const QPixmap &pixmap, QColor bgColor, int threshold = 30);

    // === 资源加载 ===
    QString findResDir() const;
    QPixmap loadTransparent(const QString &name, QColor bg, int thr);

    // === 音乐播放 ===
    void playRandomBgm();
    void stopBgm();

    // === 交互检测 ===
    bool isNearInteractable() const;

    // === 核心对象 ===
    Player player;
    Scene scene;
    Dialogue *dialogue;          // 指针类型，在构造函数中 new，resetGame 中 delete+new

    // === 游戏状态 ===
    GameState state;
    int camX;

    // === 对话系统 ===
    DialogData currentDialog;
    QString currentDialogKey;
    bool dialogHasChoices;

    // === 回溯动画 ===
    int flashbackTimer;
    static const int FLASHBACK_DURATION = 180;
    QColor flashbackBgColor;
    QString flashbackText;

    // === 结局 ===
    int endingType;
    int endingTimer;

    // === 标题画面 ===
    int titleStarCount;

    // === 标记 ===
    bool flashbackUsed;
    bool showingFlashbackBg;
    QSet<int> flashbackUsedScenes;
    bool churchStartShown;
    bool statueTalked;                // 女神像第一次互动标记
    bool circleTalked;                // 法阵第一次互动标记
    bool candleTalked;                // 烛台第一次互动标记
    bool loopShockShown;
    bool swordPicked;
    bool seedPicked;
    bool lobsterSaved;
    bool lobsterFreed;
    bool wellCleared;
    bool dreamMachineInteracted;     // 保留（兼容旧场景数据）
    bool dreamMachineUnlocked;       // 新代码使用
    bool coffinUnlocked;
    bool pendingTeleport;            // 保留（兼容旧逻辑）
    int pendingTeleportScene;        // 保留（兼容旧逻辑）
    bool libraryFlashbackShown;
    bool seedDiscovered;
    bool manuscriptRevealed;         // 保留（兼容旧逻辑）
    bool returnedFromScene6;         // 保留（兼容旧逻辑）
    bool returnedFromLibrary;        // 新代码使用
    bool swordDropped;
    bool ghostTalked;                // 新代码使用
    bool bookshelfSearched;          // 场景5书架已搜索

    // === 提示按钮 ===
    bool showHintButton;
    QRect hintButtonRect;
    QString hintText;
    bool showHintPanel;

    // === 操作提示（首次进入游戏） ===
    bool controlHintShown;
    int controlHintTimer;

    // === 资源目录 ===
    QString resDir;

    // === 背景图片 ===
    QPixmap bgChurch;
    QList<QPixmap> sceneBg;
    QList<QPixmap> sceneBgFlashback;

    // === 玩家精灵图 ===
    QPixmap playerIdle;
    QList<QPixmap> playerWalkR;
    QList<QPixmap> playerWalkL;
    int animFrame;
    int animTimer;
    bool spritesLoaded;

    // === 背景音乐 ===
    QMediaPlayer *bgmPlayer;
    QAudioOutput *audioOutput;
    QStringList bgmFiles;
    int currentBgmIndex;
};

#endif // GAMEWIDGET_H
