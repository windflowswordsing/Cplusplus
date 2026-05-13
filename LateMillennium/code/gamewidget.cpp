#include "gamewidget.h"
#include <QKeyEvent>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QLinearGradient>
#include <QRandomGenerator>
#include <QUrl>
#include <cmath>

// =================== 构造 ===================
GameWidget::GameWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    state = GameState::Title;
    camX = 0;
    flashbackTimer = 0;
    endingType = 0;
    endingTimer = 0;
    flashbackUsed = false;
    showingFlashbackBg = false;
    flashbackBgColor = QColor(0,0,0);
    flashbackText = "";
    churchStartShown = false;
    statueTalked = false;
    circleTalked = false;
    candleTalked = false;
    loopShockShown = false;
    libraryFlashbackShown = false;
    ghostTalked = false;
    bookshelfSearched = false;
    swordDropped = false;
    swordPicked = false;
    seedDiscovered = false;
    seedPicked = false;
    lobsterFreed = false;
    lobsterSaved = false;
    wellCleared = false;
    dreamMachineUnlocked = false;
    dreamMachineInteracted = false;
    coffinUnlocked = false;
    returnedFromLibrary = false;
    returnedFromScene6 = false;
    manuscriptRevealed = false;
    pendingTeleport = false;
    pendingTeleportScene = -1;

    controlHintShown = false;
    controlHintTimer = 0;
    hintText = "";

    titleStarCount = 60;
    showHintButton = false;
    hintButtonRect = QRect(width() - 100, 10, 80, 30);
    showHintPanel = false;

    dialogue = new Dialogue();
    dialogHasChoices = false;

    resDir = findResDir();
    qDebug() << "Resource directory:" << resDir;

    QStringList bgNames = {"bg/bg_church.png","bg/bg_dungeon.png","bg/bg_pyramid.png",
                            "bg/bg_volcano.jpg","bg/bg_library.jpg","bg/bg_church.png"};
    for (const QString &n : bgNames) sceneBg << QPixmap(resDir + "/" + n);

    QStringList fbNames = {"bg/bg_church_fb.png","bg/bg_dungeon_fb.jpg","bg/bg_pyramid_fb.png",
                            "bg/bg_volcano_fb.jpg","bg/bg_library_fb.jpg"};
    for (const QString &n : fbNames) {
        QPixmap pm(resDir + "/" + n);
        sceneBgFlashback << pm;
        qDebug() << "Loading flashback BG:" << n << "Null:" << pm.isNull();
    }

    QColor whiteBg(255,255,255);
    playerIdle = loadTransparent("player/player_idle.png", whiteBg, 60);
    playerWalkR << loadTransparent("player/player_walk_r1.png", whiteBg, 60)
                << loadTransparent("player/player_walk_r2.png", whiteBg, 60)
                << loadTransparent("player/player_walk_r3.png", whiteBg, 60);
    playerWalkL << loadTransparent("player/player_walk_l1.png", whiteBg, 60)
                << loadTransparent("player/player_walk_l2.png", whiteBg, 60)
                << loadTransparent("player/player_walk_l3.png", whiteBg, 60);
    spritesLoaded = !playerIdle.isNull();

    animFrame = 0;
    animTimer = 0;

    // 初始化音乐播放器
    bgmPlayer = new QMediaPlayer(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    audioOutput = new QAudioOutput(this);
    bgmPlayer->setAudioOutput(audioOutput);
    audioOutput->setVolume(0.5f);
#else
    bgmPlayer->setVolume(50);
    audioOutput = nullptr;
#endif
    currentBgmIndex = -1;
    bgmFiles << "bgm/bgm_1.mp3" << "bgm/bgm_2.mp3" << "bgm/bgm_3.mp3" << "bgm/bgm_4.mp3";

    scene.load(0);
    startTimer(16);
    setMouseTracking(true);
}

QString GameWidget::findResDir() const
{
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        // 优先：当前目录的 resources 子目录（最常见场景）
        QDir::currentPath() + "/resources",
        QDir::currentPath() + "/../resources",
        QDir::currentPath() + "/../../resources",
        // 应用目录的 resources 子目录
        appDir + "/resources",
        appDir + "/../resources",
        appDir,
        appDir + "/..",
        // 旧开发路径
        "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium"
    };
    // 优先选择包含回溯背景的目录（资源最完整）
    for (const QString &c : candidates) {
        if (QFile::exists(c + "/bg/bg_church_fb.png") && QFile::exists(c + "/bg/bg_church.png"))
            return QDir(c).absolutePath();
    }
    // 回退：只检查普通背景
    for (const QString &c : candidates) {
        if (QFile::exists(c + "/bg/bg_church.png")) return QDir(c).absolutePath();
    }
    return appDir;
}

QPixmap GameWidget::loadTransparent(const QString &name, QColor bg, int thr)
{
    QPixmap p(resDir + "/" + name);
    if (p.isNull()) return p;
    return removeBackground(p, bg, thr);
}

// =================== 音乐播放 ===================
void GameWidget::playRandomBgm()
{
    if (bgmFiles.isEmpty()) return;

    // 随机选择一首不同于当前的BGM
    int newIndex;
    do {
        newIndex = QRandomGenerator::global()->bounded(bgmFiles.size());
    } while (newIndex == currentBgmIndex && bgmFiles.size() > 1);

    currentBgmIndex = newIndex;
    QString bgmPath = resDir + "/" + bgmFiles[currentBgmIndex];

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bgmPlayer->setSource(QUrl::fromLocalFile(bgmPath));
    bgmPlayer->setLoops(QMediaPlayer::Infinite);
#else
    bgmPlayer->setMedia(QUrl::fromLocalFile(bgmPath));
#endif
    bgmPlayer->play();
}

void GameWidget::stopBgm()
{
    bgmPlayer->stop();
    currentBgmIndex = -1;
}

// =================== 主循环 ===================

void GameWidget::timerEvent(QTimerEvent *)
{
    if (state == GameState::Playing) updateGame();
    update();
}

void GameWidget::updateGame()
{
    if (!controlHintShown && controlHintTimer >= 0) {
        controlHintTimer++;
        if (controlHintTimer > 300) controlHintShown = true;
    }

    player.update();
    if (player.getVx() != 0) {
        qDebug() << "Player moving, vx:" << player.getVx();
    }

    animTimer++;
    if (animTimer >= 8) {
        animTimer = 0;
        animFrame = (player.getVx() != 0) ? (animFrame + 1) % 3 : 0;
    }

    QRect pr = player.rect();
    bool landed = false;
    for (const auto &plat : scene.platforms()) {
        if (pr.right() <= plat.left() || pr.left() >= plat.right()) continue;
        int feetY = pr.bottom();
        int prevFeetY = feetY - player.getVy();
        if (player.getVy() >= 0 && prevFeetY <= plat.top() + 2 && feetY >= plat.top()) {
            pr.moveBottom(plat.top());
            landed = true;
        }
    }
    if (landed) {
        player.rect().moveBottom(pr.bottom());
        player.land();
    }
    if (player.rect().bottom() > 620) player.rect().moveBottom(620);

    camX = player.rect().x() - width() / 3;
    if (camX < 0) camX = 0;
    if (camX > 1280 - width()) camX = 1280 - width();
}

void GameWidget::startDialog(const QString &key)
{
    DialogData d = dialogue->getDialog(key);
    if (d.text.isEmpty()) return;
    currentDialog = d;
    currentDialogKey = key;
    dialogHasChoices = !d.choices.isEmpty();
    syncDialogBg();
    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
}

void GameWidget::syncDialogBg()
{
    // 根据对话类型决定是否显示回溯背景
    if (currentDialog.inFlashbackState()) {
        showingFlashbackBg = true;
        flashbackBgColor = scene.flashbackColor();
        qDebug() << "Flashback BG enabled for dialog:" << currentDialogKey;
    } else {
        showingFlashbackBg = false;
        qDebug() << "Normal BG for dialog:" << currentDialogKey;
    }
}

void GameWidget::advanceDialog()
{
    if (!currentDialog.nextKey.isEmpty()) {
        QString nextKey = currentDialog.nextKey;
        DialogData d = dialogue->getDialog(nextKey);
        if (d.text.isEmpty()) {
            handleDialogEnd();
            return;
        }
        currentDialog = d;
        currentDialogKey = nextKey;
        dialogHasChoices = !d.choices.isEmpty();
        syncDialogBg();
        state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
    } else {
        handleDialogEnd();
    }
}

void GameWidget::handleDialogEnd()
{
    QString endedKey = currentDialogKey;

    if (endedKey == "church_auto_flashback") {
        showingFlashbackBg = false;
        startDialog("church_enter");
        return;
    }
    if (endedKey == "church_statue_flash_2") {
        statueTalked = true;
    }
    if (endedKey == "church_circle_flash_3") {
        circleTalked = true;
    }
    if (endedKey == "church_candle_flash_2") {
        candleTalked = true;
    }
    if (endedKey == "library_auto_flashback") {
        showingFlashbackBg = false;
        startDialog("library_enter");
        return;
    }
    if (endedKey == "volcano_ghost_5") {
        startDialog("sword_ghost_choice");
        return;
    }
    if (endedKey == "volcano_ghost_12") {
        ghostTalked = true;
        swordDropped = true;
    }
    if (endedKey == "volcano_sword_flash_3") {
        swordDropped = true;
    }
    // 龙虾对话结束后，幻梦机器解锁
    if (endedKey == "lobster_meet_6c") {
        lobsterSaved = true;
    }
    // 石碑调查结束后，石棺解锁
    if (endedKey == "pyramid_tablet_narration") {
        coffinUnlocked = true;
    }
    showingFlashbackBg = false;
    state = GameState::Playing;
}

// =================== 场景切换 ===================

void GameWidget::changeScene(int targetScene, int spawnX, int spawnY)
{
    player.sceneId = targetScene;
    player.setPos(spawnX, spawnY);
    scene.load(targetScene);
    showingFlashbackBg = false;
    state = GameState::Playing;

    // 进入新场景时播放随机BGM
    playRandomBgm();

    if (targetScene == 0 && !churchStartShown) {
        churchStartShown = true;
        flashbackUsedScenes.insert(0);
        startDialog("church_auto_flashback");
    }
    else if (targetScene == 4 && !libraryFlashbackShown) {
        libraryFlashbackShown = true;
        flashbackUsedScenes.insert(4);
        startDialog("library_auto_flashback");
    }
    else if (targetScene == 5 && !loopShockShown) {
        loopShockShown = true;
        startDialog("loop_shock");
    }
}

void GameWidget::checkCollisions()
{
    QRect interactRect = player.rect().adjusted(-8, -8, 8, 8);

    // ---- 场景0：教堂 ----
    if (player.sceneId == 0) {
        QRect circleR(576, 580, 128, 40);
        QRect statueR(300, 480, 100, 140);
        QRect candleR(150, 520, 80, 100);

        if (interactRect.intersects(circleR)) {
            // 法阵发光后触发结局选项
            if (seedDiscovered || returnedFromLibrary) {
                startDialog("final_choice");
            } else if (!circleTalked) {
                startDialog("church_circle");
            } else {
                startDialog("church_circle_flash");
            }
            return;
        }
        if (interactRect.intersects(statueR)) {
            if (!statueTalked) {
                startDialog("church_statue");
            } else {
                startDialog("church_statue_flash");
            }
            return;
        }
        if (interactRect.intersects(candleR)) {
            if (!candleTalked) {
                startDialog("church_candle");
            } else {
                startDialog("church_candle_flash");
            }
            return;
        }
    }

    // ---- 场景1：地下城 ----
    if (player.sceneId == 1) {
        QRect dreamR(500, 440, 200, 200);
        QRect holoR(840, 495, 120, 150);

        if (interactRect.intersects(dreamR)) {
            // 幻梦机器需要先和龙虾对话结束后才能使用
            if (!lobsterSaved) {
                startDialog("dream_machine_locked");
            } else {
                startDialog("dream_machine_choice");
            }
            return;
        }
        if (interactRect.intersects(holoR)) { 
            startDialog("dungeon_core"); 
            return; 
        }
    }

    // ---- 场景2：金字塔 ----
    if (player.sceneId == 2) {
        QRect paintR(550, 490, 200, 120);
        QRect tabletR(350, 490, 80, 100);
        QRect coffinR(810, 520, 160, 110);

        if (interactRect.intersects(paintR)) { 
            startDialog("pyramid_wall"); 
            return; 
        }
        if (interactRect.intersects(tabletR)) {
            startDialog("pyramid_tablet");
            return;
        }
        if (interactRect.intersects(coffinR)) {
            // 石棺需要先调查石碑后才能使用
            if (!coffinUnlocked) {
                startDialog("pyramid_coffin_locked");
            } else {
                startDialog("pyramid_coffin");
            }
            return;
        }
    }

    // ---- 场景3：火山 ----
    if (player.sceneId == 3) {
        QRect swordR(830, 500, 60, 120);

        if (interactRect.intersects(swordR)) {
            if (ghostTalked && !swordPicked) {
                swordPicked = true;
                player.addProp("broken_sword");
                startDialog("volcano_sword");
            } else if (swordPicked) {
                changeScene(4, 100, 540);
            }
            return;
        }
    }

    // ---- 场景4：图书馆 ----
    if (player.sceneId == 4) {
        QRect bookR(300, 480, 150, 140);
        QRect manuR(500, 580, 60, 40);
        QRect vesselR(700, 570, 60, 50);
        QRect portalR(1050, 420, 100, 200);

        if (interactRect.intersects(bookR)) { 
            startDialog("library_bookshelf"); 
            return; 
        }
        if (interactRect.intersects(manuR)) {
            startDialog("library_manuscript");
            return;
        }
        if (interactRect.intersects(vesselR)) {
            if (!seedDiscovered) {
                seedDiscovered = true;
                startDialog("library_seed");
            } else if (!seedPicked) {
                seedPicked = true;
                startDialog("seed_choice");
            }
            return;
        }
        if (interactRect.intersects(portalR)) {
            returnedFromLibrary = true;
            changeScene(0, 200, 300);
            return;
        }
    }

    // ---- 场景5：时空闭环 ----
    if (player.sceneId == 5) {
        QRect bookR(300, 480, 150, 140);
        QRect circleR(576, 580, 128, 40);

        if (interactRect.intersects(bookR) && !bookshelfSearched) {
            bookshelfSearched = true;
            startDialog("scene5_bookshelf");
            return;
        }
        if (interactRect.intersects(circleR)) {
            startDialog("scene5_circle_choice");
            return;
        }
    }

    // ---- NPC ----
    for (auto &npc : scene.npcs()) {
        if (!interactRect.intersects(npc.rect())) continue;
        QString id = npc.id();

        if (id == "well") {
            if (!wellCleared) {
                wellCleared = true;
                startDialog("church_well");
            } else {
                changeScene(1, 100, 300);
            }
            return;
        }
        if (id == "stone_door") {
            if (returnedFromLibrary) {
                changeScene(5, 100, 540);
            } else if (player.hasProp("broken_sword")) {
                changeScene(4, 100, 540);
            } else {
                startDialog("stone_door");
            }
            return;
        }
        if (id == "lobster") {
            if (!lobsterFreed) {
                lobsterFreed = true;
                startDialog("lobster_question");
                return;
            }
            if (!lobsterSaved) {
                lobsterSaved = true;
                npc.setTalked(true);
                startDialog("lobster_meet");
            }
            return;
        }
        if (id == "sword_ghost") {
            if (!ghostTalked) {
                startDialog("volcano_ghost");
            } else if (!swordDropped) {
                startDialog("sword_ghost_choice");
            }
            return;
        }
    }

    // ---- 出口 ----
    for (const auto &exit : scene.exits()) {
        if (interactRect.intersects(exit.rect)) {
            if (!exit.requireProp.isEmpty() && !player.hasProp(exit.requireProp)) return;
            changeScene(exit.targetScene, exit.spawnX, exit.spawnY);
            return;
        }
    }
}

// =================== 选择处理、重置、键盘输入 ===================

void GameWidget::handleInteraction(const QString &action)
{
    // 龙虾选项
    if (action == "lobster_resolve") {
        startDialog("lobster_meet");
        return;
    }
    if (action == "lobster_doubt") {
        startDialog("lobster_meet");
        return;
    }
    
    // 幻梦机器选项
    if (action == "go_desert") { 
        changeScene(2, 100, 300); 
        return; 
    }
    if (action == "go_volcano") { 
        changeScene(3, 100, 300); 
        return; 
    }
    if (action == "go_library") { 
        changeScene(4, 100, 540); 
        return; 
    }
    if (action == "continue_volcano") { 
        state = GameState::Playing; 
        return; 
    }
    if (action == "go_forward") { 
        changeScene(3, 100, 300); 
        return; 
    }
    if (action == "ending_dream") { 
        triggerEnding(4); 
        return; 
    }
    if (action == "dream_enter") { triggerEnding(4); return; }
    if (action == "leave_dream") { state = GameState::Playing; showingFlashbackBg = false; return; }
    
    // 石棺选项
    if (action == "coffin_enter") { triggerEnding(5); return; }
    if (action == "leave_coffin") { state = GameState::Playing; showingFlashbackBg = false; return; }
    
    // 断剑选项
    if (action == "take_sword") { startDialog("volcano_sword_flash"); return; }
    if (action == "leave_ghost") { state = GameState::Playing; showingFlashbackBg = false; return; }
    
    // 手稿选项
    if (action == "study_manuscript") { triggerEnding(3); return; }
    if (action == "leave_manuscript") { state = GameState::Playing; showingFlashbackBg = false; return; }
    
    // 结局选项
    if (action == "ending_newborn") { triggerEnding(1); return; }
    if (action == "ending_time") { triggerEnding(3); return; }
    if (action == "ending_wait") { state = GameState::Playing; showingFlashbackBg = false; return; }
    if (action == "ending_return") { triggerEnding(2); return; }
    if (action == "take_seed") { state = GameState::Playing; showingFlashbackBg = false; return; }
    if (action == "study_time_magic") { triggerEnding(3); return; }
    if (action == "leave_book") { state = GameState::Playing; showingFlashbackBg = false; return; }

    state = GameState::Playing;
    showingFlashbackBg = false;
}

void GameWidget::resetGame()
{
    player.clearProps();
    player.sceneId = 0;
    player.setPos(100, 300);

    delete dialogue;
    dialogue = new Dialogue();

    flashbackUsed = false;
    showingFlashbackBg = false;
    flashbackUsedScenes.clear();
    churchStartShown = false;
    statueTalked = false;
    circleTalked = false;
    candleTalked = false;
    loopShockShown = false;
    libraryFlashbackShown = false;
    ghostTalked = false;
    bookshelfSearched = false;
    swordDropped = false;
    swordPicked = false;
    seedDiscovered = false;
    seedPicked = false;
    lobsterFreed = false;
    lobsterSaved = false;
    wellCleared = false;
    dreamMachineUnlocked = false;
    dreamMachineInteracted = false;
    coffinUnlocked = false;
    returnedFromLibrary = false;
    returnedFromScene6 = false;
    manuscriptRevealed = false;
    pendingTeleport = false;
    pendingTeleportScene = -1;

    endingType = 0;
    endingTimer = 0;

    currentDialog = DialogData();
    currentDialog.type = DialogType::Talk;
    currentDialog.isEnding = false;
    currentDialog.endingType = 0;
    currentDialogKey = "";
    dialogHasChoices = false;

    controlHintShown = false;
    controlHintTimer = 0;
    hintText = "";
    showHintButton = false;
    showHintPanel = false;

    camX = 0;
    animFrame = 0;
    animTimer = 0;

    stopBgm();  // 重置时停止音乐

    scene.load(0);
    state = GameState::Title;
}

void GameWidget::triggerEnding(int type)
{
    endingType = type;
    endingTimer = 0;
    state = GameState::Ending;
    showingFlashbackBg = false;
}

void GameWidget::keyPressEvent(QKeyEvent *e)
{
    if (state == GameState::Title) {
        if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
            scene.load(0);
            churchStartShown = true;
            flashbackUsedScenes.insert(0);
            startDialog("church_auto_flashback");
            controlHintTimer = 0;
            playRandomBgm();  // 游戏开始时播放BGM
        }
        return;
    }
    if (state == GameState::Ending) {
        resetGame();
        return;
    }
    if (state == GameState::Dialog) {
        if (e->key() == Qt::Key_E || e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
            advanceDialog();
        }
        return;
    }
    if (state == GameState::Choice) {
        int cnt = currentDialog.choices.size();
        int idx = -1;
        if (e->key() == Qt::Key_1 && cnt >= 1) idx = 0;
        else if (e->key() == Qt::Key_2 && cnt >= 2) idx = 1;
        else if (e->key() == Qt::Key_3 && cnt >= 3) idx = 2;
        if (idx >= 0) handleInteraction(currentDialog.choices[idx].action);
        return;
    }

    player.keyPress(e);
    if (e->key() == Qt::Key_A || e->key() == Qt::Key_Left || e->key() == Qt::Key_D || e->key() == Qt::Key_Right) {
        qDebug() << "Movement key pressed:" << e->key();
    }
    if (e->key() == Qt::Key_E) checkCollisions();
}

void GameWidget::keyReleaseEvent(QKeyEvent *e)
{
    // 始终传递方向键释放事件，防止对话/菜单期间松开方向键后角色无法停止
    int k = e->key();
    if (k == Qt::Key_A || k == Qt::Key_Left || k == Qt::Key_D || k == Qt::Key_Right) {
        player.keyRelease(e);
    }
}

void GameWidget::mousePressEvent(QMouseEvent *) {}

// =================== 存根函数（头文件声明但新逻辑中未使用） ===================

void GameWidget::startFlashback(const QString &trigger)
{
    Q_UNUSED(trigger)
}

void GameWidget::startLibraryFlashback()
{
    // 已由 changeScene 中的逻辑处理
}

void GameWidget::updateFlashback()
{
    // 已由对话系统处理
}

void GameWidget::drawFlashback(QPainter &p)
{
    Q_UNUSED(p)
}

void GameWidget::drawPixelRect(QPainter &p, QRect r, QColor c)
{
    p.setPen(c.darker(150));
    p.setBrush(c);
    p.drawRect(r);
}

// =================== 渲染函数 ===================

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    if (state == GameState::Title) { drawTitle(p); return; }
    if (state == GameState::Ending) { drawEnding(p); return; }

    drawGame(p);

    if (state == GameState::Dialog || state == GameState::Choice) {
        drawDialog(p);
    }
}

void GameWidget::drawTitle(QPainter &p)
{
    static QPixmap titleBg;
    static bool loaded = false;
    if (!loaded) { titleBg.load(resDir + "/ui/title_bg.png"); loaded = true; }

    // 深色背景
    p.fillRect(rect(), QColor(10, 8, 15));

    // 图片完整居中显示
    if (!titleBg.isNull()) {
        float ir = (float)titleBg.width() / titleBg.height();
        float sr = (float)width() / height();
        int dw, dh, dx, dy;
        if (ir > sr) { dw = width(); dh = int(width() / ir); dx = 0; dy = (height() - dh) / 2; }
        else { dh = height(); dw = int(height() * ir); dx = (width() - dw) / 2; dy = 0; }
        p.drawPixmap(dx, dy, dw, dh, titleBg);
    }

    // 半透明遮罩
    p.fillRect(rect(), QColor(0, 0, 0, 80));

    int w = width();
    int midY = height() / 2;

    // ========== 统一字体大小：14pt ==========
    QFont poemFont("SimHei", 14);
    QFont titleFont("SimHei", 14, QFont::Bold);

    // 诗句呼吸动画
    float poemAlpha = 160 + 60 * std::sin(QDateTime::currentMSecsSinceEpoch() * 0.0008);
    float a1 = 180 + 75 * std::sin(QDateTime::currentMSecsSinceEpoch() * 0.001);

    // ========== 左侧：标题 ==========
    int leftX = 80;

    p.setPen(QColor(220, 200, 150, int(a1)));
    p.setFont(titleFont);
    p.drawText(leftX, midY - 20, "千年迟赴：文明灰烬");

    // 副标题已删除

    p.setPen(QColor(120, 100, 70, 150));
    p.setFont(poemFont);
    p.drawText(leftX, midY - 45, "✦ ───────────── ✦");
    p.drawText(leftX, midY + 35, "✦ ───────────── ✦");

    // ========== 右侧：诗句 ==========
    int rightX = w - 280;

    p.setPen(QColor(200, 180, 140, int(poemAlpha)));
    p.setFont(poemFont);
    p.drawText(rightX, midY - 20, "山河有尽，文明有痕，");
    p.drawText(rightX, midY + 8,  "有缘之人，循迹而来");

    p.setPen(QColor(120, 100, 70, 150));
    p.setFont(poemFont);
    p.drawText(rightX, midY - 45, "✦ ───────────── ✦");
    p.drawText(rightX, midY + 35, "✦ ───────────── ✦");

    // ========== 底部提示 ==========
    float blink = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.004) + 1) * 0.5;
    int tipX = w - 260;
    p.setPen(QColor(200, 200, 200, int(80 + blink * 175)));
    p.setFont(poemFont);
    p.drawText(tipX, midY + 80, "按 空格键 开始游戏");

    // ========== 底部装饰线 ==========
    p.setPen(QColor(100, 90, 70, 100));
    p.drawLine(60, height() - 40, w - 60, height() - 40);

    // 角落花纹
    p.setPen(QColor(140, 120, 80, 120));
    p.setFont(QFont("SimHei", 16));
    p.drawText(30, 50, "✧");
    p.drawText(w - 50, 50, "✧");
    p.drawText(30, height() - 30, "✧");
    p.drawText(w - 50, height() - 30, "✧");
}

void GameWidget::drawGame(QPainter &p)
{
    // 回溯对话：回溯背景 + 暖色滤镜 + 暗角，不显示游戏元素
    if (showingFlashbackBg) {
        drawScrollingBg(p);
        p.fillRect(rect(), QColor(255, 245, 230, 50));
        QLinearGradient grad(0, 0, 0, height());
        grad.setColorAt(0, QColor(0, 0, 0, 60));
        grad.setColorAt(0.3, QColor(0, 0, 0, 0));
        grad.setColorAt(0.7, QColor(0, 0, 0, 0));
        grad.setColorAt(1, QColor(0, 0, 0, 60));
        p.fillRect(rect(), grad);
        return;
    }

    // 现实对话 / 正常游戏：显示完整游戏画面
    p.fillRect(rect(), scene.bgColor());
    drawScrollingBg(p);
    p.save();
    p.translate(-camX, 0);
    drawPixelGround(p);
    drawProps(p);
    drawNpcs(p);
    drawPlayer(p);
    p.restore();

    drawHUD(p);
}

void GameWidget::drawScrollingBg(QPainter &p)
{
    int sceneId = player.sceneId;
    int w = width();
    int h = height();

    // 视差微动
    float parallax = 0.03f;
    int scrollX = int(-camX * parallax);

    // 选择背景图：回溯时显示盛景版
    QPixmap bg;
    if (showingFlashbackBg && sceneId < sceneBgFlashback.size()) {
        bg = sceneBgFlashback[sceneId];
        if (!bg.isNull()) {
            qDebug() << "Using flashback BG for scene" << sceneId;
        } else {
            qDebug() << "Flashback BG is null for scene" << sceneId << "- trying to reload";
        }
    }
    if (bg.isNull() && sceneId < sceneBg.size()) {
        bg = sceneBg[sceneId];
    }

    if (!bg.isNull()) {
        // 自适应画面：按比例缩放，覆盖整个屏幕
        int bgImgW = int(bg.width() * (float)h / bg.height());
        if (bgImgW < w) bgImgW = w; // 保证至少覆盖屏幕宽度

        // 居中绘制，微动偏移
        int drawX = scrollX + (w - bgImgW) / 2;
        p.drawPixmap(drawX, 0, bgImgW, h, bg);
    } else {
        // 无背景图时使用纯色底色
        p.fillRect(rect(), scene.bgColor());
    }

    // 回溯时额外加载盛景背景（运行时重试）
    if (showingFlashbackBg && sceneId < sceneBgFlashback.size() && sceneBgFlashback[sceneId].isNull()) {
        QString fbFile = resDir + "/bg/bg_church_fb.png";
        if (sceneId == 1) fbFile = resDir + "/bg/bg_dungeon_fb.jpg";
        else if (sceneId == 2) fbFile = resDir + "/bg/bg_pyramid_fb.png";
        else if (sceneId == 3) fbFile = resDir + "/bg/bg_volcano_fb.jpg";
        else if (sceneId == 4) fbFile = resDir + "/bg/bg_library_fb.jpg";
        QPixmap fb(fbFile);
        if (!fb.isNull()) {
            sceneBgFlashback[sceneId] = fb;
            int bgImgW = int(fb.width() * (float)h / fb.height());
            if (bgImgW < w) bgImgW = w;
            int drawX = (w - bgImgW) / 2;
            p.drawPixmap(drawX, 0, bgImgW, h, fb);
        }
    }

    // 场景5（终局）额外：银白流光粒子
    if (sceneId == 5) {
        p.setPen(Qt::NoPen);
        unsigned int seed = 5 * 31 + 7;
        for (int i = 0; i < 20; i++) {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            int px = (seed * 3) % w;
            int py = (seed * 7) % (h - 100) + 50;
            float a = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.002 + i) + 1) * 0.5;
            p.setBrush(QColor(220, 220, 240, int(a * 60)));
            p.drawRect(px, py, 2, 2);
        }
    }
}

void GameWidget::drawPlayer(QPainter &p)
{
    QRect r = player.rect();
    if (!spritesLoaded) {
        p.setBrush(QColor(200,200,220));
        p.setPen(QColor(150,150,170));
        p.drawRect(r);
        return;
    }
    QPixmap sp;
    if (player.getVx() != 0) {
        sp = player.facingRight ? playerWalkR[animFrame] : playerWalkL[animFrame];
    } else {
        sp = playerIdle;
    }
    if (!sp.isNull()) {
        int dh = r.height() + 20;
        int dw = dh * sp.width() / sp.height();
        int dx = r.x() + r.width()/2 - dw/2;
        int dy = r.y() + r.height() - dh;
        p.drawPixmap(dx, dy, dw, dh, sp);
    }
}

void GameWidget::drawProps(QPainter &p)
{
    QColor whiteBg(255,255,255), blackBg(0,0,0);

    if (player.sceneId == 0) {
        static QPixmap magicCircle, magicCircleGlow, churchStatue, churchCandle, portalDoor;
        static bool loaded = false;
        if (!loaded) {
            magicCircle = removeBackground(QPixmap(resDir+"/prop/prop_magic_circle.png"), blackBg, 30);
            magicCircleGlow = removeBackground(QPixmap(resDir+"/prop/prop_magic_circle_glow.png"), blackBg, 30);
            churchStatue = QPixmap(resDir+"/prop/prop_church_statue.png");
            churchCandle = removeBackground(QPixmap(resDir+"/prop/prop_church_candle.jpg"), blackBg, 40);
            portalDoor = removeBackground(QPixmap(resDir+"/prop/prop_portal_door.png"), blackBg, 30);
            loaded = true;
        }
        QPixmap &circle = (seedDiscovered || returnedFromLibrary) && !magicCircleGlow.isNull() ? magicCircleGlow : magicCircle;
        if (!circle.isNull()) p.drawPixmap(576, 580, 128, 40, circle);
        if (!churchStatue.isNull()) p.drawPixmap(300, 480, 100, 140, churchStatue);
        if (!churchCandle.isNull()) p.drawPixmap(150, 520, 80, 100, churchCandle);
    }

    if (player.sceneId == 1) {
        static QPixmap holoScreen, dreamMachine;
        static bool loaded = false;
        if (!loaded) {
            holoScreen = removeBackground(QPixmap(resDir+"/prop/prop_holo_screen.png"), blackBg, 30);
            dreamMachine = QPixmap(resDir+"/prop/prop_dream_machine.png");
            loaded = true;
        }
        // 幻梦机器：使用图片素材
        if (!dreamMachine.isNull()) {
            p.drawPixmap(500, 440, 200, 200, dreamMachine);
        } else {
            // 回退：纯色块绘制
            p.setPen(QColor(100, 90, 130));
            p.setBrush(QColor(60, 50, 90, 200));
            p.drawRect(500, 440, 200, 200);
            p.setPen(QColor(150, 140, 180, 150));
            p.setBrush(Qt::NoBrush);
            p.drawRect(505, 445, 190, 190);
            p.setBrush(QColor(180, 170, 220, 80));
            p.drawEllipse(580, 520, 40, 40);
        }
        if (!holoScreen.isNull()) p.drawPixmap(840, 495, 120, 150, holoScreen);
    }

    if (player.sceneId == 2) {
        static QPixmap wallPaint, stoneTablet, stoneCoffin;
        static bool loaded = false;
        if (!loaded) {
            wallPaint = removeBackground(QPixmap(resDir+"/prop/prop_wall_painting.png"), blackBg, 30);
            stoneTablet = removeBackground(QPixmap(resDir+"/prop/prop_stone_tablet.png"), blackBg, 30);
            stoneCoffin = removeBackground(QPixmap(resDir+"/prop/prop_stone_coffin.png"), whiteBg, 30);
            loaded = true;
        }
        if (!stoneTablet.isNull()) p.drawPixmap(350, 490, 80, 100, stoneTablet);
        if (!wallPaint.isNull()) p.drawPixmap(550, 490, 200, 120, wallPaint);
        if (!stoneCoffin.isNull()) p.drawPixmap(810, 520, 160, 110, stoneCoffin);

        if (coffinUnlocked) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(80,150,255,50));
            p.drawEllipse(850, 570, 180, 140);
        }
    }

    if (player.sceneId == 3 && swordDropped) {
        if (!swordPicked) {
            static QPixmap brokenSword;
            static bool loaded = false;
            if (!loaded) {
                brokenSword = QPixmap(resDir+"/prop/prop_broken_sword.png");
                loaded = true;
            }
            if (!brokenSword.isNull()) {
                p.drawPixmap(830, 500, 60, 120, brokenSword);
                float bob = std::sin(QDateTime::currentMSecsSinceEpoch()*0.005)*3;
                p.setBrush(QColor(255,220,50));
                p.setPen(QColor(200,170,0));
                p.drawRect(870, 485+bob, 14, 14);
                p.setPen(Qt::white);
                p.setFont(QFont("Arial",10,QFont::Bold));
                p.drawText(QRect(870,485+bob,14,14), Qt::AlignCenter, "!");
            }
        }
    }

    if (player.sceneId == 4) {
        static QPixmap bookshelf, manuscript, seedVessel, seed, portalDoor;
        static bool loaded = false;
        if (!loaded) {
            bookshelf = removeBackground(QPixmap(resDir+"/prop/prop_bookshelf.png"), whiteBg, 30);
            manuscript = removeBackground(QPixmap(resDir+"/prop/prop_manuscript.png"), whiteBg, 30);
            seedVessel = removeBackground(QPixmap(resDir+"/prop/prop_seed_vessel.png"), whiteBg, 30);
            seed = removeBackground(QPixmap(resDir+"/prop/prop_seed.png"), whiteBg, 30);
            portalDoor = removeBackground(QPixmap(resDir+"/prop/prop_portal_door.png"), blackBg, 30);
            loaded = true;
        }
        if (!bookshelf.isNull()) p.drawPixmap(300, 480, 150, 140, bookshelf);
        if (!manuscript.isNull()) p.drawPixmap(500, 580, 60, 40, manuscript);
        if (!seedDiscovered) {
            if (!seedVessel.isNull()) p.drawPixmap(700, 570, 60, 50, seedVessel);
        } else if (!seedPicked) {
            if (!seed.isNull()) p.drawPixmap(700, 570, 60, 50, seed);
        }
        if (!portalDoor.isNull()) p.drawPixmap(1050, 420, 100, 200, portalDoor);
    }

    if (player.sceneId == 5) {
        static QPixmap magicCircleGlow;
        static bool loaded = false;
        if (!loaded) {
            magicCircleGlow = removeBackground(QPixmap(resDir+"/prop/prop_magic_circle_glow.png"), blackBg, 30);
            loaded = true;
        }
        if (!magicCircleGlow.isNull()) p.drawPixmap(576, 580, 128, 40, magicCircleGlow);
    }
}

void GameWidget::drawNpcs(QPainter &p)
{
    QColor whiteBg(255,255,255);
    for (auto &npc : scene.npcs()) {
        QRect r = npc.rect();
        QString id = npc.id();

        if (id == "well") {
            static QPixmap wellBlocked, wellOpen;
            static bool loaded = false;
            if (!loaded) {
                wellBlocked = removeBackground(QPixmap(resDir+"/prop/prop_well_blocked.png"), whiteBg, 30);
                wellOpen = removeBackground(QPixmap(resDir+"/prop/prop_well_open.png"), whiteBg, 30);
                loaded = true;
            }
            QPixmap &img = wellCleared ? wellOpen : wellBlocked;
            if (!img.isNull()) {
                int s = 120;
                p.drawPixmap(r.center().x()-s/2, r.center().y()-s/2+20, s, s, img);
            }
        }
        else if (id == "stone_door") {
            static QPixmap doorImg, portalImg;
            static bool loaded = false;
            if (!loaded) {
                doorImg = removeBackground(QPixmap(resDir+"/npc/npc_stone_door.png"), whiteBg, 30);
                portalImg = QPixmap(resDir+"/prop/prop_portal_door.png");
                loaded = true;
            }
            QPixmap &img = returnedFromLibrary && !portalImg.isNull() ? portalImg : doorImg;
            if (!img.isNull()) {
                int dh = 140;
                int dw = int(dh * (float)img.width()/img.height());
                p.drawPixmap(r.center().x()-dw/2, r.top(), dw, dh, img);
            }
        }
        else if (id == "lobster") {
            static QPixmap trapped, saved;
            static bool loaded = false;
            if (!loaded) {
                trapped = removeBackground(QPixmap(resDir+"/npc/npc_lobster_trapped.png"), whiteBg, 60);
                saved = removeBackground(QPixmap(resDir+"/npc/npc_lobster_saved.png"), whiteBg, 60);
                loaded = true;
            }
            QPixmap &img = lobsterFreed ? saved : trapped;
            if (!img.isNull()) {
                int dh = 100;
                int dw = int(dh * (float)img.width()/img.height());
                p.drawPixmap(r.center().x()-dw/2, r.bottom()-dh, dw, dh, img);
            }
        }
        else if (id == "sword_ghost") {
            if (swordDropped) continue;
            static QPixmap ghostImg;
            static bool loaded = false;
            if (!loaded) {
                ghostImg = removeBackground(QPixmap(resDir+"/npc/npc_sword_ghost.png"), whiteBg, 30);
                loaded = true;
            }
            if (!ghostImg.isNull()) {
                int dh = 140;
                int dw = int(dh * (float)ghostImg.width()/ghostImg.height());
                p.drawPixmap(r.center().x()-dw/2, r.bottom()-dh, dw, dh, ghostImg);
            }
        }

        if (!npc.talked() && (id != "well" || !wellCleared)) {
            float bob = std::sin(QDateTime::currentMSecsSinceEpoch()*0.005)*3;
            p.setBrush(QColor(255,220,50));
            p.setPen(QColor(200,170,0));
            p.drawRect(r.right()-2, r.top()-18+bob, 14, 14);
            p.setPen(Qt::white);
            p.setFont(QFont("Arial",10,QFont::Bold));
            p.drawText(QRect(r.right()-2, r.top()-18+bob, 14, 14), Qt::AlignCenter, "!");
        }
    }
}

void GameWidget::drawDialog(QPainter &p)
{
    bool isFb = currentDialog.inFlashbackState();  // 是否回溯
    bool isNr = currentDialog.isNarration;  // 是否旁白（描述性叙述）

    // ====== 背景处理 ======
    if (isFb) {
        // 回溯对话：半透明遮罩更浅，让回溯背景更清晰
        p.fillRect(rect(), QColor(0, 0, 0, 60));
    } else {
        // 现实对话：正常半透明遮罩，游戏画面可见
        p.fillRect(rect(), QColor(0, 0, 0, 100));
    }

    // ====== 立绘（仅现实对话显示，旁白不显示立绘）======
    QString speaker = currentDialog.speaker;
    bool showHero  = (!isFb && !isNr && speaker == "勇者");
    bool showKing  = (!isFb && !isNr && speaker == "国王");
    bool showGhost = (!isFb && !isNr && speaker == "守剑幽魂");

    static QPixmap heroImg, kingImg, ghostImg;
    static bool portraitLoaded = false;
    if (!portraitLoaded) {
        heroImg  = QPixmap(resDir + "/portrait/portrait_hero.png");
        kingImg  = QPixmap(resDir + "/portrait/portrait_king.jpg");
        ghostImg = QPixmap(resDir + "/portrait/portrait_ghost.png");
        if (!heroImg.isNull()) heroImg = removeBackground(heroImg, QColor(255,255,255), 60);
        if (!kingImg.isNull()) kingImg = removeBackground(kingImg, QColor(255,255,255), 60);
        if (!ghostImg.isNull()) ghostImg = removeBackground(ghostImg, QColor(255,255,255), 60);
        portraitLoaded = true;
    }

    // 绘制立绘
    if (showHero && !heroImg.isNull()) {
        int ph = 300;
        int pw = qMin(220, int(ph * (float)heroImg.width()/heroImg.height()));
        p.drawPixmap(30, height()-ph-40, pw, ph, heroImg);
    }
    if (showKing && !kingImg.isNull()) {
        int ph = 300;
        int pw = qMin(220, int(ph * (float)kingImg.width()/kingImg.height()));
        p.drawPixmap(30, height()-ph-40, pw, ph, kingImg);
    }
    if (showGhost && !ghostImg.isNull()) {
        int ph = 300;
        int pw = qMin(220, int(ph * (float)ghostImg.width()/ghostImg.height()));
        p.drawPixmap(width()-pw-30, height()-ph-40, pw, ph, ghostImg);
    }

    // ====== 对话框尺寸（缩小）======
    int boxX = 80, boxY = height() - 260, boxW = width() - 160, boxH = 220;
    bool hasLeftPortrait = (showHero && !heroImg.isNull()) || (showKing && !kingImg.isNull());
    bool hasRightPortrait = (showGhost && !ghostImg.isNull());
    if (hasLeftPortrait) { boxX = 260; boxW = width() - 320; }
    if (hasRightPortrait) { boxW = width() - 320; }

    // ====== 对话框背景 ======
    QColor borderColor;
    if (isFb) {
        // 回溯：暖色调边框
        borderColor = QColor(150, 130, 90);
    } else if (isNr) {
        // 旁白：冷色调边框
        borderColor = QColor(80, 90, 120);
    } else {
        // 现实普通对话
        borderColor = QColor(120, 100, 80);
    }
    p.setPen(borderColor);
    p.setBrush(QColor(15, 12, 20, 240));
    p.drawRect(boxX, boxY, boxW, boxH);
    p.setPen(QColor(borderColor.red(), borderColor.green(), borderColor.blue(), 150));
    p.setBrush(Qt::NoBrush);
    p.drawRect(boxX+4, boxY+4, boxW-8, boxH-8);

    // ====== 说话人名字（旁白不显示） ======
    if (!isNr && !speaker.isEmpty()) {
        QColor sc = Qt::white;
        QString sColor = currentDialog.speakerColor;
        if (sColor == "gray") sc = QColor(160, 160, 170);
        else if (sColor == "yellow") sc = QColor(255, 220, 100);
        else if (sColor == "gold") sc = QColor(255, 200, 50);

        p.setPen(sc);
        p.setFont(QFont("SimHei", 14, QFont::Bold));
        p.drawText(boxX+20, boxY+30, speaker);

        // 分隔线
        p.setPen(QColor(sc.red(), sc.green(), sc.blue(), 80));
        p.drawLine(boxX+20, boxY+40, boxX+boxW-40, boxY+40);
    } else if (isNr) {
        // 旁白：不显示名字，用淡色装饰线代替
        p.setPen(QColor(100, 110, 140, 60));
        p.drawLine(boxX+20, boxY+25, boxX+boxW-40, boxY+25);
    }

    // ====== 对话文本 ======
    int textTop = boxY + (isNr ? 40 : 55);  // 旁白时文本区域上移
    int textHeight = (state == GameState::Choice) ? 120 : 170;

    if (isNr) {
        // 旁白样式：斜体 + 淡蓝灰色调
        QFont narrationFont("SimHei", 13);
        narrationFont.setItalic(true);
        p.setFont(narrationFont);
        if (isFb) {
            // 回溯旁白：暖灰色
            p.setPen(QColor(210, 200, 180));
        } else {
            // 现实旁白：冷蓝灰色
            p.setPen(QColor(180, 190, 210));
        }
    } else {
        // 普通对话样式
        p.setPen(QColor(230, 230, 230));
        p.setFont(QFont("SimHei", 13));
    }

    p.drawText(QRect(boxX+20, textTop, boxW-40, textHeight),
               Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, currentDialog.text);

    // ====== 提示文本（tip）======
    if (!currentDialog.tipText.isEmpty()) {
        int tipY = textTop + textHeight + 5;
        int tipHeight = 30;
        p.setPen(QColor(150, 140, 120, 180));
        p.setFont(QFont("SimHei", 10));
        p.drawText(QRect(boxX+20, tipY, boxW-40, tipHeight),
                   Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, currentDialog.tipText);
    }

    // ====== 选项/继续提示 ======
    if (state == GameState::Choice && !currentDialog.choices.isEmpty()) {
        int y = boxY + boxH - 30 - currentDialog.choices.size() * 28;
        for (int i = 0; i < currentDialog.choices.size(); i++) {
            p.setBrush(QColor(40, 35, 50, 180));
            p.setPen(QColor(120, 110, 140));
            p.drawRect(boxX+30, y+i*28, boxW-60, 24);
            p.setPen(QColor(230, 230, 220));
            p.setFont(QFont("SimHei", 12));
            p.drawText(boxX+40, y+i*28+17,
                       QString("%1. %2").arg(i+1).arg(currentDialog.choices[i].text));
        }
    } else {
        float blink = (std::sin(QDateTime::currentMSecsSinceEpoch()*0.005)+1)*0.5;
        int alpha = int(100 + blink*155);
        p.setPen(QColor(220, 200, 120, alpha));
        p.setFont(QFont("SimHei", 11));
        p.drawText(boxX+boxW-100, boxY+boxH-15, "[按E继续]");
    }
}

// =================== 结局渲染（★ 文字完整显示） ===================
void GameWidget::drawEnding(QPainter &p)
{
    endingTimer++;
    EndingData ed = Ending::getEnding(endingType);

    float fadeIn = qMin(endingTimer / 60.0f, 1.0f);

    // 加载结局背景
    static QPixmap endingBgs[6];
    static bool loaded = false;
    if (!loaded) {
        endingBgs[1].load(resDir + "/ending/ending_seed.png");
        endingBgs[2].load(resDir + "/ending/ending_return.png");
        endingBgs[3].load(resDir + "/ending/ending_time.png");
        endingBgs[4].load(resDir + "/ending/ending_dream.png");
        endingBgs[5].load(resDir + "/ending/ending_coffin.png");
        loaded = true;
    }

    QPixmap *bg = nullptr;
    if (endingType >= 1 && endingType <= 5 && !endingBgs[endingType].isNull()) {
        bg = &endingBgs[endingType];
    }

    if (bg && !bg->isNull()) {
        p.fillRect(rect(), QColor(0,0,0));
        float ia = (float)bg->width() / bg->height();
        float wa = (float)width() / height();
        int dw, dh, dx, dy;
        if (ia > wa) { dw = width(); dh = int(width()/ia); }
        else { dh = height(); dw = int(height()*ia); }
        dx = (width()-dw)/2;
        dy = (height()-dh)/2;
        p.setOpacity(fadeIn);
        p.drawPixmap(dx, dy, dw, dh, *bg);
        p.setOpacity(1.0);
        // 暗色叠加，让文字更清晰
        p.fillRect(rect(), QColor(0,0,0, int(fadeIn * 120)));
    } else {
        QColor bgc = ed.bgColor;
        bgc.setAlpha(int(fadeIn * 255));
        p.fillRect(rect(), bgc);
    }

    if (endingTimer < 30) return;

    // 结局名称
    p.setPen(ed.textColor);
    p.setFont(QFont("SimHei", 36, QFont::Bold));
    QFontMetrics fm(p.font());
    int tw = fm.horizontalAdvance(ed.name);
    p.drawText(width()/2 - tw/2, 100, ed.name);

    // 分隔线
    p.setPen(QColor(ed.textColor.red(),ed.textColor.green(),ed.textColor.blue(),100));
    p.drawLine(width()/2-150, 130, width()/2+150, 130);

    // ★ 结局描述：足够大的显示区域（容纳完整文字）
    p.setPen(QColor(ed.textColor.red(),ed.textColor.green(),ed.textColor.blue(),230));
    p.setFont(QFont("SimHei", 15));
    int descW = qMin(900, width() - 100);
    int descH = height() - 250;
    p.drawText(QRect((width()-descW)/2, 150, descW, descH),
               Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, ed.desc);

    // 重新开始提示
    if (endingTimer > 120) {
        float blink = (std::sin(QDateTime::currentMSecsSinceEpoch()*0.004)+1)*0.5;
        p.setPen(QColor(200,200,200,int(100+blink*155)));
        p.setFont(QFont("SimHei", 12));
        p.drawText(QRect(0, height()-50, width(), 30), Qt::AlignCenter,
                   "按 任意键 返回标题");
    }
}

void GameWidget::drawHUD(QPainter &p)
{
    p.setPen(QColor(180,170,150));
    p.setFont(QFont("SimHei", 11));
    p.drawText(10, 25, scene.name());

    QString h = hintText.isEmpty() ? scene.hint() : hintText;
    if (!h.isEmpty()) {
        p.setPen(QColor(140,140,130));
        p.setFont(QFont("SimHei", 9));
        QStringList lines = h.split('\n');
        int y = 45;
        for (const QString &line : lines) {
            if (y > 100) break;
            p.drawText(10, y, line);
            y += 18;
        }
    }

    // 道具栏
    QStringList props = player.props();
    if (!props.isEmpty()) {
        int x = width() - 200;
        p.setPen(QColor(150,140,120));
        p.setFont(QFont("SimHei", 9));
        p.drawText(x, 25, "道具：");
        x += 40;
        for (const auto &prop : props) {
            QString lb = (prop=="broken_sword") ? "断剑" : (prop=="seed") ? "种子" : prop;
            p.drawText(x, 25, lb);
            x += 50;
        }
    }

    // 交互提示
    if (isNearInteractable() && state == GameState::Playing) {
        float pulse = (std::sin(QDateTime::currentMSecsSinceEpoch()*0.004)+1)*0.5;
        int ix = width()-60, iy = 50;
        p.setBrush(QColor(30,28,35,200));
        p.setPen(QColor(180,160,100,int(180+pulse*75)));
        p.drawRect(ix, iy, 24, 24);
        p.setPen(QColor(255,230,150,int(200+pulse*55)));
        p.setFont(QFont("Arial", 14, QFont::Bold));
        p.drawText(QRect(ix, iy, 24, 24), Qt::AlignCenter, "E");
    }

    // 首次操作提示
    if (!controlHintShown && controlHintTimer > 0) {
        int alpha = (controlHintTimer > 240) ? int(220 * (300-controlHintTimer)/60.0) : 220;
        alpha = qMax(0, qMin(220, alpha));
        int pw = 420, ph = 40;
        int px = (width()-pw)/2, py = height()-60;
        p.setBrush(QColor(15,12,20,alpha));
        p.setPen(QColor(120,110,80,alpha));
        p.drawRect(px, py, pw, ph);
        p.setPen(QColor(255,240,200,alpha));
        p.setFont(QFont("SimHei", 11));
        p.drawText(QRect(px, py, pw, ph), Qt::AlignCenter,
                   "[A/D] 移动     [空格] 跳跃     [E] 互动");
    }
}

bool GameWidget::isNearInteractable() const
{
    QRect ir = player.rect().adjusted(-8,-8,8,8);
    for (const auto &npc : scene.npcs()) {
        if (npc.id() == "well" || npc.id() == "stone_door") {
            if (ir.intersects(npc.rect())) return true;
            continue;
        }
        if (npc.id() == "sword_ghost") {
            if (ir.intersects(npc.rect())) return true;
            continue;
        }
        if (npc.talked()) continue;
        if (ir.intersects(npc.rect())) return true;
    }
    for (const auto &exit : scene.exits()) {
        if (ir.intersects(exit.rect)) return true;
    }
    return false;
}

void GameWidget::drawPixelGround(QPainter &p)
{
    int sid = player.sceneId;
    static QPixmap tileChurch, tileDungeon, tilePyramid, tileVolcano, tileLibrary;
    static bool loaded = false;
    if (!loaded) {
        tileChurch.load(resDir + "/tile/tile_church.png");
        tileDungeon.load(resDir + "/tile/tile_dungeon.png");
        tilePyramid.load(resDir + "/tile/tile_pyramid.png");
        tileVolcano.load(resDir + "/tile/tile_volcano.png");
        tileLibrary.load(resDir + "/tile/tile_library.png");
        if (tileLibrary.isNull()) tileLibrary = tileChurch;
        loaded = true;
    }
    QPixmap *tile = nullptr;
    switch (sid) {
        case 0: case 5: tile = &tileChurch; break;
        case 1: tile = &tileDungeon; break;
        case 2: tile = &tilePyramid; break;
        case 3: tile = &tileVolcano; break;
        case 4: tile = &tileLibrary; break;
        default: tile = &tileChurch;
    }

    QRect ground(0, 620, 1280, 100);
    if (tile && !tile->isNull()) {
        const int TS = 64;
        for (int y = ground.top(); y < ground.bottom(); y += TS)
            for (int x = 0; x < ground.right(); x += TS)
                p.drawPixmap(x, y, qMin(TS, ground.right()-x),
                            qMin(TS, ground.bottom()-y), *tile);
    } else {
        p.fillRect(ground, scene.groundColor());
    }
}

// =================== 图片透明化 ===================
QPixmap GameWidget::removeBackground(const QPixmap &pixmap, QColor bgColor, int threshold)
{
    if (pixmap.isNull()) return pixmap;
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    int bgR = bgColor.red(), bgG = bgColor.green(), bgB = bgColor.blue();
    for (int y = 0; y < img.height(); y++) {
        QRgb *line = reinterpret_cast<QRgb*>(img.scanLine(y));
        for (int x = 0; x < img.width(); x++) {
            QRgb px = line[x];
            int r = qRed(px), g = qGreen(px), b = qBlue(px);
            int dist = std::sqrt((r-bgR)*(r-bgR)+(g-bgG)*(g-bgG)+(b-bgB)*(b-bgB));
            if (dist < threshold) line[x] = qRgba(0,0,0,0);
            else if (dist < threshold*1.5) {
                float a = (dist-threshold)/(threshold*0.5f);
                line[x] = qRgba(r,g,b,int(qMin(1.0f,a)*255));
            }
        }
    }
    return QPixmap::fromImage(img);
}

void GameWidget::drawIntro(QPainter &) {}
