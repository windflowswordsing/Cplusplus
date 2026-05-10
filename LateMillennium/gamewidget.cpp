#include "gamewidget.h"
#include <QKeyEvent>
#include <QCoreApplication>
#include <QDir>
#include <cmath>

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
    churchStartShown = false;
    loopShockShown = false;
    swordPicked = false;
    seedPicked = false;
    lobsterSaved = false;
    wellCleared = false;
    titleStarCount = 60;
    showHintButton = false;
    showHintPanel = false;
    hintButtonRect = QRect(width() - 100, 10, 80, 30);
    hintText = "";
    controlHintShown = false;
    controlHintTimer = 0;

    // 获取资源目录
    QString appDir = QCoreApplication::applicationDirPath();
    // 尝试多个可能的路径
    QString resDir = appDir;
    if (!QFile::exists(resDir + "/bg_church.png")) {
        // 如果build目录没有图片，使用项目目录
        resDir = "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium";
    }
    if (!QFile::exists(resDir + "/bg_church.png")) {
        // 最后尝试当前工作目录
        resDir = QDir::currentPath();
    }
    
    // 去底色参数：白色背景，容差30（适应近白色/灰白色）
    QColor bgColor(255, 255, 255);
    
    // 加载6个场景的废墟背景图
    QStringList bgNames = {
        "bg_church.png",    // 0: 破败教堂
        "bg_dungeon.png",   // 1: 赛博地下城
        "bg_pyramid.png",   // 2: 沙漠金字塔
        "bg_volcano.jpg",   // 3: 末日火山
        "bg_library.jpg",   // 4: 图书馆秘境
        "bg_church.png"     // 5: 终局（复用教堂）
    };
    for (int i = 0; i < bgNames.size(); i++) {
        QPixmap p(resDir + "/" + bgNames[i]);
        if (p.isNull()) {
            qDebug() << "Failed to load background:" << bgNames[i] << "from" << resDir;
        }
        sceneBg << p;
    }
    bgChurch = sceneBg[0]; // 兼容旧代码

    // 加载5个场景的回溯盛景背景图
    QStringList fbNames = {
        "bg_church_fb.png",    // 0: 教堂盛景
        "bg_dungeon_fb.jpg",   // 1: 地下城盛景
        "bg_pyramid_fb.png",   // 2: 金字塔盛景
        "bg_volcano_fb.jpg",   // 3: 火山盛景
        "bg_library_fb.jpg"    // 4: 图书馆盛景
    };
    for (int i = 0; i < fbNames.size(); i++) {
        QPixmap p(resDir + "/" + fbNames[i]);
        if (p.isNull()) {
            qDebug() << "Failed to load flashback background:" << fbNames[i] << "from" << resDir;
        }
        sceneBgFlashback << p;
    }

    // 加载玩家精灵图（自动去白色底色）
    playerIdle = removeBackground(QPixmap(resDir + "/player_idle.png"), bgColor);
    playerWalkR << removeBackground(QPixmap(resDir + "/player_walk_r1.png"), bgColor)
                << removeBackground(QPixmap(resDir + "/player_walk_r2.png"), bgColor)
                << removeBackground(QPixmap(resDir + "/player_walk_r3.png"), bgColor);
    playerWalkL << removeBackground(QPixmap(resDir + "/player_walk_l1.png"), bgColor)
                << removeBackground(QPixmap(resDir + "/player_walk_l2.png"), bgColor)
                << removeBackground(QPixmap(resDir + "/player_walk_l3.png"), bgColor);
    spritesLoaded = !playerIdle.isNull() && playerWalkR.size() == 3 && playerWalkL.size() == 3;
    
    // 调试输出
    if (!spritesLoaded) {
        qDebug("Warning: Failed to load player sprites from %s", qPrintable(appDir));
    }
    if (bgChurch.isNull()) {
        qDebug("Warning: Failed to load background image from %s", qPrintable(appDir));
    }

    animFrame = 0;
    animTimer = 0;

    startTimer(16); // ~60fps
    setMouseTracking(true);
}

// ==================== 主循环 ====================

void GameWidget::timerEvent(QTimerEvent *)
{
    if (state == GameState::Title || state == GameState::Ending) {
        update();
        return;
    }
    if (state == GameState::Flashback) {
        updateFlashback();
        update();
        return;
    }
    if (state == GameState::Dialog || state == GameState::Choice) {
        update();
        return;
    }

    // Playing
    updateGame();
    update();
}

void GameWidget::updateGame()
{
    // 操作提示计时（约5秒后淡出）
    if (!controlHintShown && controlHintTimer >= 0) {
        controlHintTimer++;
        if (controlHintTimer > 300) { // ~5秒 @60fps
            controlHintShown = true;
        }
    }

    player.update();

    // 更新动画帧
    animTimer++;
    if (animTimer >= 8) { // 每8帧切换一次（约7.5fps动画速度）
        animTimer = 0;
        if (player.getVx() != 0) {
            animFrame = (animFrame + 1) % 3;
        } else {
            animFrame = 0;
        }
    }

    // === 单向平台碰撞检测（只从上方着陆） ===
    QRect pr = player.rect();
    bool landed = false;

    for (const auto &plat : scene.platforms()) {
        // 水平方向有重叠
        if (pr.right() <= plat.left() || pr.left() >= plat.right())
            continue;

        // 角色正在下落，且脚部穿过平台顶部
        int feetY = pr.bottom();
        int prevFeetY = feetY - player.getVy(); // 上一帧脚部位置

        if (player.getVy() >= 0 &&
            prevFeetY <= plat.top() + 2 &&  // 上一帧脚在平台顶部附近或上方
            feetY >= plat.top()) {           // 当前帧脚穿过或到达平台顶部
            // 着陆：将脚对齐到平台顶部
            pr.moveBottom(plat.top());
            landed = true;
        }
    }

    if (landed) {
        player.rect().moveBottom(pr.bottom());
        player.land(); // 通知Player已着陆，重置vy和onGround
    }

    // 防止掉出地图底部
    if (player.rect().bottom() > 620) {
        player.rect().moveBottom(620);
    }

    checkCollisions();

    // 摄像机跟随
    camX = player.rect().x() - width() / 3;
    if (camX < 0) camX = 0;
    if (camX > 1280 - width()) camX = 1280 - width();
}

void GameWidget::checkCollisions()
{
    // 不再每帧重新load，场景数据在changeScene时已加载
    // scene.load(player.sceneId); // 已移除

    // 扩大玩家交互检测范围（比碰撞框大一些，方便交互）
    QRect interactRect = player.rect().adjusted(-8, -8, 8, 8);

    // 道具碰撞
    for (auto &prop : scene.props()) {
        if (interactRect.intersects(prop.rect()) && !prop.picked()) {
            QString id = prop.id();

            if (id == "dream_machine") {
                // 救下龙虾前幻梦机器无法互动
                if (!lobsterSaved) continue;
                // 救下龙虾后解锁幻梦机器互动
                if (!prop.picked()) {
                    prop.pick(); // 标记为已交互（不消失）
                    DialogData dd = dialogue.getDialog("dream_machine");
                    if (!dd.text.isEmpty()) {
                        currentDialog = dd;
                        currentDialogKey = "dream_machine";
                        dialogHasChoices = !dd.choices.isEmpty();
                        state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                    }
                    return;
                }
                continue;
            }
            if (id == "broken_sword" && !swordPicked) {
                swordPicked = true;
                player.addProp("broken_sword");
                prop.pick();
                currentDialog = dialogue.getDialog("pick_sword");
                currentDialogKey = "pick_sword";
                dialogHasChoices = false;
                state = GameState::Dialog;
                return;
            }
            if (id == "seed" && !seedPicked) {
                seedPicked = true;
                player.addProp("seed");
                prop.pick();
                currentDialog = dialogue.getDialog("pick_seed");
                currentDialogKey = "pick_seed";
                dialogHasChoices = false;
                state = GameState::Dialog;
                return;
            }

            // 通用道具拾取
            if (!prop.label().isEmpty()) {
                player.addProp(id);
                prop.pick();

                // 检查是否有对应对话
                QString dialogKey = id;
                DialogData dd = dialogue.getDialog(dialogKey);
                if (!dd.text.isEmpty()) {
                    currentDialog = dd;
                    currentDialogKey = dialogKey;
                    dialogHasChoices = !dd.choices.isEmpty();
                    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;

                    // 回溯触发
                    if (scene.flashbackTrigger() == id && !flashbackUsed) {
                        startFlashback(id);
                        return;
                    }
                    return;
                }
            }
        }
    }

    // NPC碰撞
    for (auto &npc : scene.npcs()) {
        if (interactRect.intersects(npc.rect())) {
            QString id = npc.id();

            // 古井：按E互动，不标记talked（可重复交互）
            if (id == "well") {
                if (!wellCleared) {
                    wellCleared = true;
                    currentDialog = dialogue.getDialog("well_clear");
                    currentDialogKey = "well_clear";
                    dialogHasChoices = false;
                    state = GameState::Dialog;
                } else {
                    changeScene(1, 100, 300); // 进入地下城
                }
                return;
            }

            // 石门：按E互动，不标记talked（可重复交互）
            if (id == "stone_door") {
                if (player.hasProp("broken_sword")) {
                    changeScene(4, 100, 300); // 进入图书馆
                } else {
                    currentDialog = dialogue.getDialog("stone_door_locked");
                    currentDialogKey = "stone_door_locked";
                    dialogHasChoices = false;
                    state = GameState::Dialog;
                }
                return;
            }

            // 特殊NPC处理
            if (id == "lobster" && !lobsterSaved) {
                lobsterSaved = true;
                npc.setTalked(true);
                currentDialog = dialogue.getDialog("lobster");
                currentDialogKey = "lobster";
                dialogHasChoices = false;
                state = GameState::Dialog;

                // 回溯触发
                if (scene.flashbackTrigger() == id && !flashbackUsed) {
                    startFlashback(id);
                }
                return;
            }

            if (id == "sword_ghost" && swordPicked) {
                // 已拾取断剑，幽魂消散，不再触发
                continue;
            }

            if (!npc.talked()) {
                DialogData dd = dialogue.getDialog(id);
                if (!dd.text.isEmpty()) {
                    npc.setTalked(true);
                    currentDialog = dd;
                    currentDialogKey = id;
                    dialogHasChoices = !dd.choices.isEmpty();
                    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                    return;
                }
            }
        }
    }

    // 出口碰撞
    for (auto &exit : scene.exits()) {
        if (interactRect.intersects(exit.rect)) {
            // 检查道具需求
            if (!exit.requireProp.isEmpty() && !player.hasProp(exit.requireProp)) {
                // 没有需要的道具
                if (exit.requireProp == "broken_sword") {
                    currentDialog = dialogue.getDialog("stone_door_locked");
                    currentDialogKey = "stone_door_locked";
                    dialogHasChoices = false;
                    state = GameState::Dialog;
                }
                return;
            }
            changeScene(exit.targetScene, exit.spawnX, exit.spawnY);
            return;
        }
    }
}

// ==================== 场景切换 ====================

void GameWidget::changeScene(int targetScene, int spawnX, int spawnY)
{
    player.sceneId = targetScene;
    player.setPos(spawnX, spawnY);
    scene.load(targetScene);

    // 场景进入事件
    if (targetScene == 0 && !churchStartShown) {
        churchStartShown = true;
        currentDialog = dialogue.getDialog("church_start");
        currentDialogKey = "church_start";
        dialogHasChoices = false;
        state = GameState::Dialog;
    }
    else if (targetScene == 4 && player.hasProp("broken_sword")) {
        currentDialog = dialogue.getDialog("library_enter");
        currentDialogKey = "library_enter";
        dialogHasChoices = false;
        state = GameState::Dialog;
    }
    else if (targetScene == 5) {
        if (!loopShockShown) {
            loopShockShown = true;
            currentDialog = dialogue.getDialog("loop_shock");
            currentDialogKey = "loop_shock";
            dialogHasChoices = false;
            state = GameState::Dialog;
        }
    }
}

// ==================== 回溯系统 ====================

void GameWidget::startFlashback(const QString &trigger)
{
    flashbackTimer = FLASHBACK_DURATION;
    flashbackBgColor = scene.flashbackColor();
    flashbackText = dialogue.getFlashback(trigger).text;
    showingFlashbackBg = true;  // 切换为回溯盛景背景
    state = GameState::Flashback;
}

void GameWidget::updateFlashback()
{
    flashbackTimer--;

    if (flashbackTimer <= 0) {
        showingFlashbackBg = false;  // 切回废墟背景

        // 回溯结束后显示残影对话
        DialogData fb = dialogue.getFlashback(scene.flashbackTrigger());
        if (!fb.text.isEmpty()) {
            currentDialog = fb;
            currentDialogKey = scene.flashbackTrigger() + "_flashback";
            dialogHasChoices = false;
            state = GameState::Dialog;
        } else {
            state = GameState::Playing;
        }
    }
}

// ==================== 交互处理 ====================

void GameWidget::handleInteraction(const QString &action)
{
    if (action == "dream_enter") {
        triggerEnding(4); // 幻梦沉沦
    }
    else if (action == "coffin_enter") {
        triggerEnding(5); // 枯骨永眠
    }
    else if (action == "ending_newborn") {
        triggerEnding(1); // 文明新生
    }
    else if (action == "ending_dream") {
        triggerEnding(2); // 归世幻梦
    }
    else if (action == "ending_time") {
        triggerEnding(3); // 逆时赴约
    }
    else {
        state = GameState::Playing;
    }
}

// ==================== 结局 ====================

void GameWidget::triggerEnding(int type)
{
    endingType = type;
    endingTimer = 0;
    state = GameState::Ending;
}

// ==================== 键盘输入 ====================

void GameWidget::keyPressEvent(QKeyEvent *e)
{
    // 标题画面
    if (state == GameState::Title) {
        if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
            scene.load(0); // 加载第一个场景
            // 触发初始对话
            churchStartShown = true;
            currentDialog = dialogue.getDialog("church_start");
            currentDialogKey = "church_start";
            dialogHasChoices = false;
            state = GameState::Dialog;
            showHintButton = true;
            hintText = "这里就是被召唤的世界？没有生机，没有战火，只有废墟……\n召唤阵还在，可一切都太晚了。";
            controlHintTimer = 0; // 对话结束后开始显示操作提示
        }
        return;
    }

    // 结局画面
    if (state == GameState::Ending) {
        // 重置游戏
        player.clearProps();
        player.sceneId = 0;
        player.setPos(100, 300);
        flashbackUsed = false;
        churchStartShown = false;
        loopShockShown = false;
        swordPicked = false;
        seedPicked = false;
        lobsterSaved = false;
        endingType = 0;
        state = GameState::Title;
        return;
    }

    // 回溯动画中不可操作
    if (state == GameState::Flashback) return;

    // 对话状态
    if (state == GameState::Dialog) {
        if (e->key() == Qt::Key_E || e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
            // 推进对话
            if (!currentDialog.nextKey.isEmpty()) {
                currentDialog = dialogue.getDialog(currentDialog.nextKey);
                currentDialogKey = currentDialog.nextKey;
                dialogHasChoices = !currentDialog.choices.isEmpty();
                state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
            } else {
                // 对话结束
                state = GameState::Playing;
            }
        }
        return;
    }

    // 选择状态
    if (state == GameState::Choice) {
        int choiceCount = currentDialog.choices.size();
        if (e->key() == Qt::Key_1 && choiceCount >= 1) {
            handleInteraction(currentDialog.choices[0].action);
        } else if (e->key() == Qt::Key_2 && choiceCount >= 2) {
            handleInteraction(currentDialog.choices[1].action);
        } else if (e->key() == Qt::Key_3 && choiceCount >= 3) {
            handleInteraction(currentDialog.choices[2].action);
        }
        return;
    }

    // Playing
    player.keyPress(e);

    // E键交互
    if (e->key() == Qt::Key_E) {
        checkCollisions();
    }

    // F键时间回溯（场景中间区域可触发，每个场景只能用一次）
    if (e->key() == Qt::Key_F && !flashbackUsed) {
        int sceneId = player.sceneId;
        // 检查该场景是否已使用过回溯
        if (!flashbackUsedScenes.contains(sceneId)) {
            QRect pr = player.rect();
            // 场景中间区域：x在300~980之间
            if (pr.center().x() >= 300 && pr.center().x() <= 980) {
                flashbackUsedScenes.insert(sceneId);  // 标记该场景已使用
                startFlashback(scene.flashbackTrigger());
            }
        }
    }

    // H键显示/隐藏提示
    if (e->key() == Qt::Key_H) {
        showHintButton = !showHintButton;
    }
}

void GameWidget::keyReleaseEvent(QKeyEvent *e)
{
    // 只在Playing状态下处理按键释放
    if (state != GameState::Playing) return;
    player.keyRelease(e);
}

// ==================== 鼠标事件 ====================

void GameWidget::mousePressEvent(QMouseEvent *e)
{
    // 点击提示按钮
    if (showHintButton && hintButtonRect.contains(e->pos())) {
        showHintPanel = !showHintPanel;
        return;
    }

    // 点击提示面板外部关闭
    if (showHintPanel) {
        QRect panelRect(width() - 320, 50, 300, 150);
        if (!panelRect.contains(e->pos()) && !hintButtonRect.contains(e->pos())) {
            showHintPanel = false;
        }
    }
}

// ==================== 渲染 ====================

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false); // 像素风格，关闭抗锯齿

    if (state == GameState::Title) {
        drawTitle(p);
        return;
    }
    if (state == GameState::Ending) {
        drawEnding(p);
        return;
    }
    if (state == GameState::Flashback) {
        drawFlashback(p);
        return;
    }

    drawGame(p);

    if (state == GameState::Dialog || state == GameState::Choice) {
        drawDialog(p);
    }

    // 绘制提示按钮和面板
    if (state == GameState::Playing && showHintButton) {
        drawHintButton(p);
    }
}

void GameWidget::drawTitle(QPainter &p)
{
    // 深色背景
    p.fillRect(rect(), QColor(15, 12, 20));

    // 星空
    p.setPen(Qt::NoPen);
    for (int i = 0; i < titleStarCount; i++) {
        int sx = (i * 137 + 50) % width();
        int sy = (i * 97 + 20) % (height() / 2);
        float alpha = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.001 + i * 0.5) + 1) * 0.5;
        p.setBrush(QColor(200, 200, 220, int(alpha * 200)));
        int size = (i % 3) + 1;
        p.drawRect(sx, sy, size, size); // 像素点
    }

    // 标题
    p.setPen(QColor(180, 160, 100));
    p.setFont(QFont("SimHei", 36, QFont::Bold));
    p.drawText(rect(), Qt::AlignCenter, "千年迟赴");

    // 副标题
    p.setPen(QColor(120, 110, 90));
    p.setFont(QFont("SimHei", 16));
    p.drawText(QRect(0, height()/2 + 30, width(), 30), Qt::AlignCenter, "文明余烬与圣女之约");

    // 开始提示
    float blink = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.004) + 1) * 0.5;
    p.setPen(QColor(200, 200, 200, int(100 + blink * 155)));
    p.setFont(QFont("SimHei", 14));
    p.drawText(QRect(0, height() - 80, width(), 30), Qt::AlignCenter, "按 空格键 开始游戏");

    // 操作说明
    p.setPen(QColor(100, 100, 110));
    p.setFont(QFont("SimHei", 10));
    p.drawText(QRect(0, height() - 40, width(), 20), Qt::AlignCenter,
               "A/D 移动 | 空格 跳跃 | E 交互 | 1/2/3 选择");
}

void GameWidget::drawGame(QPainter &p)
{
    // 底色填充
    p.fillRect(rect(), scene.bgColor());

    // === 卷轴背景层（视差滚动） ===
    drawScrollingBg(p);

    // === 前景层（跟随摄像机） ===
    p.save();
    p.translate(-camX, 0);

    // 绘制像素风格地面和平台
    drawPixelGround(p);

    // 道具
    drawProps(p);

    // NPC
    drawNpcs(p);

    // 玩家
    drawPlayer(p);

    p.restore();

    // HUD
    drawHUD(p);
}

void GameWidget::drawPixelRect(QPainter &p, QRect r, QColor c)
{
    // 像素风格：实心矩形 + 深色边框
    p.setPen(c.darker(150));
    p.setBrush(c);
    p.drawRect(r);
}

void GameWidget::drawPlayer(QPainter &p)
{
    QRect r = player.rect();

    if (spritesLoaded) {
        // 使用精灵图
        QPixmap sprite;
        if (player.getVx() != 0) {
            // 行走动画
            if (player.facingRight) {
                sprite = playerWalkR[animFrame];
            } else {
                sprite = playerWalkL[animFrame];
            }
        } else {
            // 静止
            sprite = playerIdle;
        }

        if (!sprite.isNull()) {
            // 缩放精灵图适配碰撞框大小
            int drawH = r.height() + 20; // 稍微放大一点
            int drawW = drawH * sprite.width() / sprite.height();
            int drawX = r.x() + r.width() / 2 - drawW / 2;
            int drawY = r.y() + r.height() - drawH;
            p.drawPixmap(drawX, drawY, drawW, drawH, sprite);
            return;
        }
    }

    // 回退：像素块绘制（精灵图加载失败时）
    p.setPen(Qt::black);
    p.setBrush(QColor(60, 80, 140));
    p.drawRect(r.adjusted(4, 12, -4, -10));

    p.setBrush(QColor(220, 190, 160));
    p.drawRect(r.adjusted(6, 0, -6, -20));

    p.setBrush(QColor(40, 30, 20));
    p.drawRect(r.adjusted(5, -2, -5, -22));

    p.setBrush(Qt::white);
    int eyeX = player.facingRight ? r.x() + 14 : r.x() + 6;
    p.drawRect(eyeX, r.y() + 6, 4, 4);
    p.setBrush(Qt::black);
    int pupilX = player.facingRight ? eyeX + 2 : eyeX;
    p.drawRect(pupilX, r.y() + 7, 2, 2);

    p.setBrush(QColor(40, 40, 100));
    p.drawRect(r.x() + 5, r.bottom() - 10, 6, 10);
    p.drawRect(r.right() - 11, r.bottom() - 10, 6, 10);

    if (player.hasProp("broken_sword")) {
        p.setBrush(QColor(180, 180, 200));
        int swordX = player.facingRight ? r.right() + 2 : r.left() - 4;
        p.drawRect(swordX, r.y() + 8, 3, 32);
        p.setBrush(QColor(140, 140, 160));
        p.drawRect(swordX - 2, r.y() + 6, 7, 4);
    }
}

void GameWidget::drawProps(QPainter &p)
{
    // 获取资源目录（与构造函数一致）
    static QString resDir;
    if (resDir.isEmpty()) {
        resDir = QCoreApplication::applicationDirPath();
        if (!QFile::exists(resDir + "/bg_church.png")) {
            resDir = "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium";
        }
        if (!QFile::exists(resDir + "/bg_church.png")) {
            resDir = QDir::currentPath();
        }
    }
    
    // 幻梦机器图片（懒加载）
    static QPixmap dreamMachineImg;
    if (dreamMachineImg.isNull()) {
        dreamMachineImg.load(resDir + "/prop_dream_machine.png");
    }

    for (auto &prop : scene.props()) {
        if (prop.picked()) continue;
        if (prop.label().isEmpty()) continue; // 装饰性道具

        QRect r = prop.rect();
        QColor c = prop.color();

        // 幻梦机器：绘制大型图片
        if (prop.id() == "dream_machine" && !dreamMachineImg.isNull()) {
            // 放大显示：200x200 像素
            int imgSize = 200;
            int drawX = r.center().x() - imgSize / 2;
            int drawY = r.center().y() - imgSize / 2;
            p.drawPixmap(drawX, drawY, imgSize, imgSize, dreamMachineImg);

            // 标签
            p.setPen(QColor(200, 200, 200));
            p.setFont(QFont("SimHei", 10, QFont::Bold));
            p.drawText(drawX, drawY - 10, imgSize, 20, Qt::AlignCenter, prop.label());
            continue;
        }

        // 发光效果
        QColor glow = c;
        glow.setAlpha(40);
        p.setPen(Qt::NoPen);
        p.setBrush(glow);
        p.drawEllipse(r.center(), r.width() / 2 + 8, r.height() / 2 + 8);

        // 道具本体
        drawPixelRect(p, r, c);

        // 标签
        p.setPen(QColor(200, 200, 200));
        p.setFont(QFont("SimHei", 9));
        p.drawText(r.x(), r.y() - 6, prop.label());
    }
}

void GameWidget::drawNpcs(QPainter &p)
{
    for (auto &npc : scene.npcs()) {
        QRect r = npc.rect();
        QColor c = npc.color();

        // NPC本体
        drawPixelRect(p, r, c);

        // 标签
        p.setPen(QColor(220, 220, 200));
        p.setFont(QFont("SimHei", 9));
        p.drawText(r.x(), r.y() - 6, npc.label());

        // 交互提示 "!"
        if (!npc.talked()) {
            float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
            p.setBrush(QColor(255, 220, 50));
            p.setPen(QColor(200, 170, 0));
            p.drawRect(r.right() - 2, r.top() - 18 + bob, 14, 14);
            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(QRect(r.right() - 2, r.top() - 18 + bob, 14, 14), Qt::AlignCenter, "!");
        }
    }
}

void GameWidget::drawDialog(QPainter &p)
{
    // 半透明遮罩
    p.fillRect(rect(), QColor(0, 0, 0, 100));

    // 对话框背景
    int boxX = 60, boxY = height() - 220, boxW = width() - 120, boxH = 180;
    p.setPen(QColor(120, 100, 80));
    p.setBrush(QColor(15, 12, 20, 230));
    p.drawRect(boxX, boxY, boxW, boxH);

    // 内边框
    p.setPen(QColor(80, 70, 50, 150));
    p.setBrush(Qt::NoBrush);
    p.drawRect(boxX + 4, boxY + 4, boxW - 8, boxH - 8);

    // 说话人
    QString speakerColor = currentDialog.speakerColor;
    QColor sc = Qt::white;
    if (speakerColor == "gray") sc = QColor(160, 160, 170);
    else if (speakerColor == "yellow") sc = QColor(255, 220, 100);
    else if (speakerColor == "gold") sc = QColor(255, 200, 50);
    else if (speakerColor == "cyan") sc = QColor(100, 200, 255);
    else if (speakerColor == "blue") sc = QColor(100, 150, 255);
    else if (speakerColor == "green") sc = QColor(100, 255, 150);
    else if (speakerColor == "darkYellow") sc = QColor(200, 180, 100);
    else if (speakerColor == "beige") sc = QColor(220, 210, 180);

    p.setPen(sc);
    p.setFont(QFont("SimHei", 13, QFont::Bold));
    p.drawText(boxX + 20, boxY + 28, currentDialog.speaker);

    // 对话文本
    p.setPen(QColor(220, 220, 220));
    p.setFont(QFont("SimHei", 12));
    p.drawText(QRect(boxX + 20, boxY + 40, boxW - 40, 80),
               Qt::AlignLeft | Qt::TextWordWrap, currentDialog.text);

    // 选项
    if (state == GameState::Choice && !currentDialog.choices.isEmpty()) {
        int y = boxY + 125;
        for (int i = 0; i < currentDialog.choices.size(); i++) {
            // 选项背景
            p.setBrush(QColor(40, 35, 50, 180));
            p.setPen(QColor(120, 110, 140));
            p.drawRect(boxX + 30, y + i * 28, boxW - 60, 24);

            // 选项文字
            p.setPen(QColor(230, 230, 220));
            p.setFont(QFont("SimHei", 11));
            p.drawText(boxX + 40, y + i * 28 + 17,
                       QString("%1. %2").arg(i + 1).arg(currentDialog.choices[i].text));
        }
    } else {
        // 像素风格下箭头提示（代替文字"按E继续"）
        float blink = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) + 1) * 0.5;
        int alpha = int(100 + blink * 155);
        int cx = boxX + boxW - 30;
        int cy = boxY + boxH - 22;

        p.setPen(Qt::NoPen);
        // 像素下箭头 ▼
        p.setBrush(QColor(220, 200, 120, alpha));
        // 箭头由像素块组成
        p.drawRect(cx - 2, cy,     2, 2);
        p.drawRect(cx + 2, cy,     2, 2);
        p.drawRect(cx - 4, cy + 2, 2, 2);
        p.drawRect(cx,     cy + 2, 2, 2);
        p.drawRect(cx + 4, cy + 2, 2, 2);
        p.drawRect(cx - 6, cy + 4, 2, 2);
        p.drawRect(cx - 2, cy + 4, 2, 2);
        p.drawRect(cx + 2, cy + 4, 2, 2);
        p.drawRect(cx + 6, cy + 4, 2, 2);
        // 箭杆
        p.drawRect(cx - 1, cy + 6, 4, 4);
    }
}

void GameWidget::drawFlashback(QPainter &p)
{
    float progress = 1.0f - (float)flashbackTimer / FLASHBACK_DURATION; // 0→1

    if (progress < 0.3) {
        // 阶段1：银白流光渐入
        float t = progress / 0.3f;
        p.fillRect(rect(), scene.bgColor());
        QColor silver(200, 200, 220, int(t * 200));
        p.fillRect(rect(), silver);

        // 像素流光粒子
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 40; i++) {
            int px = (i * 173 + flashbackTimer * 3) % width();
            int py = (i * 131 + flashbackTimer * 2) % height();
            p.setBrush(QColor(220, 220, 240, int(t * 180)));
            p.drawRect(px, py, 3, 3);
        }
    }
    else if (progress < 0.7) {
        // 阶段2：暖色调回溯场景 + 圣女剪影
        float t = (progress - 0.3f) / 0.4f;
        p.fillRect(rect(), flashbackBgColor);

        // 暖色场景元素（简化像素块）
        p.setPen(Qt::NoPen);
        // 彩绘玻璃窗
        for (int i = 0; i < 5; i++) {
            QColor glass(180 + i * 15, 100 + i * 20, 50, int(t * 150));
            p.setBrush(glass);
            p.drawRect(200 + i * 60, 100, 40, 80);
        }
        // 人群剪影
        p.setBrush(QColor(60, 50, 40, int(t * 180)));
        for (int i = 0; i < 8; i++) {
            p.drawRect(300 + i * 40, 400, 16, 40);
            p.drawRect(300 + i * 40 + 2, 385, 12, 15);
        }

        // 圣女素白剪影（中央）
        float alpha = t * 255;
        if (t > 0.7) alpha = (1.0f - (t - 0.7f) / 0.3f) * 255; // 即将清晰时淡出
        p.setBrush(QColor(240, 240, 250, int(alpha)));
        p.drawRect(610, 250, 20, 50); // 身体
        p.drawRect(614, 230, 12, 20); // 头

        // 残影对话文字
        if (!flashbackText.isEmpty()) {
            p.setPen(QColor(160, 160, 170, int(t * 255)));
            p.setFont(QFont("SimHei", 12));
            p.drawText(QRect(80, height() - 180, width() - 160, 100),
                       Qt::AlignLeft | Qt::TextWordWrap, flashbackText);
        }
    }
    else {
        // 阶段3：碎裂黑屏
        float t = (progress - 0.7f) / 0.3f;
        p.fillRect(rect(), QColor(0, 0, 0, int(t * 255)));

        // 白色像素块碎裂
        p.setPen(Qt::NoPen);
        for (int i = 0; i < 50; i++) {
            int px = (i * 173) % width();
            int py = (i * 131) % height();
            int dx = (i % 7 - 3) * int(t * 30);
            int dy = (i % 5 - 2) * int(t * 30);
            p.setBrush(QColor(220, 220, 240, int((1 - t) * 200)));
            p.drawRect(px + dx, py + dy, 4, 4);
        }
    }
}

void GameWidget::drawEnding(QPainter &p)
{
    endingTimer++;

    EndingData ed = Ending::getEnding(endingType);

    // 背景渐入
    float fadeIn = qMin(endingTimer / 60.0f, 1.0f);
    QColor bg = ed.bgColor;
    bg.setAlpha(int(fadeIn * 255));
    p.fillRect(rect(), bg);

    if (endingTimer < 30) return; // 等待背景渐入

    // 像素粒子效果
    p.setPen(Qt::NoPen);
    for (int i = 0; i < 30; i++) {
        int px = (i * 173 + endingTimer * 2) % width();
        int py = (i * 131 + endingTimer) % height();
        float a = (std::sin(endingTimer * 0.05 + i) + 1) * 0.5;
        p.setBrush(QColor(ed.textColor.red(), ed.textColor.green(), ed.textColor.blue(), int(a * 120)));
        p.drawRect(px, py, 2, 2);
    }

    // 结局名称
    p.setPen(ed.textColor);
    p.setFont(QFont("SimHei", 32, QFont::Bold));
    QFontMetrics fm(p.font());
    int tw = fm.horizontalAdvance(ed.name);
    p.drawText(width() / 2 - tw / 2, height() / 2 - 80, ed.name);

    // 分隔线
    p.setPen(QColor(ed.textColor.red(), ed.textColor.green(), ed.textColor.blue(), 100));
    p.drawLine(width() / 2 - 150, height() / 2 - 50, width() / 2 + 150, height() / 2 - 50);

    // 结局描述
    p.setPen(QColor(ed.textColor.red(), ed.textColor.green(), ed.textColor.blue(), 200));
    p.setFont(QFont("SimHei", 14));
    p.drawText(QRect(width() / 2 - 280, height() / 2 - 30, 560, 140),
               Qt::AlignCenter | Qt::TextWordWrap, ed.desc);

    // 重新开始提示
    if (endingTimer > 120) {
        float blink = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.004) + 1) * 0.5;
        p.setPen(QColor(200, 200, 200, int(100 + blink * 155)));
        p.setFont(QFont("SimHei", 12));
        p.drawText(QRect(0, height() - 60, width(), 30), Qt::AlignCenter, "按任意键返回标题");
    }
}

void GameWidget::drawHUD(QPainter &p)
{

    // 场景名称
    p.setPen(QColor(180, 170, 150));
    p.setFont(QFont("SimHei", 11));
    p.drawText(10, 25, scene.name());

    // 提示文字
    if (!scene.hint().isEmpty()) {
        p.setPen(QColor(140, 140, 130));
        p.setFont(QFont("SimHei", 9));
        p.drawText(10, 45, scene.hint());
    }

    // 道具栏
    QStringList props = player.props();
    if (!props.isEmpty()) {
        int x = width() - 200;
        p.setPen(QColor(150, 140, 120));
        p.setFont(QFont("SimHei", 9));
        p.drawText(x, 25, "道具：");
        x += 40;
        for (const auto &prop : props) {
            QString label;
            if (prop == "broken_sword") label = "断剑";
            else if (prop == "seed") label = "种子";
            else label = prop;
            p.drawText(x, 25, label);
            x += 50;
        }
    }

    // 回溯能力状态
    if (flashbackUsed) {
        p.setPen(QColor(100, 100, 110));
        p.setFont(QFont("SimHei", 9));
        p.drawText(10, height() - 10, "回溯能力已耗尽");
    } else {
        p.setPen(QColor(150, 150, 200));
        p.setFont(QFont("SimHei", 9));
        p.drawText(10, height() - 10, "回溯能力：可用");
    }

    // 右上角交互提示符号（靠近可交互对象时显示）
    if (isNearInteractable()) {
        float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.006) * 2;
        float pulse = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.004) + 1) * 0.5;
        int ix = width() - 60;
        int iy = 50 + bob;

        p.setPen(Qt::NoPen);

        // 外圈光晕
        QColor glow(255, 220, 100, int(30 + pulse * 40));
        p.setBrush(glow);
        p.drawEllipse(ix - 2, iy - 2, 28, 28);

        // 按键底框（像素风格圆角矩形）
        p.setBrush(QColor(30, 28, 35, 200));
        p.drawRect(ix, iy, 24, 24);
        // 边框
        p.setBrush(QColor(180, 160, 100, int(180 + pulse * 75)));
        p.drawRect(ix, iy, 24, 2);         // 上
        p.drawRect(ix, iy, 2, 24);         // 左
        p.drawRect(ix + 22, iy, 2, 24);    // 右
        p.drawRect(ix, iy + 22, 24, 2);    // 下

        // "E" 字母（像素风格，5x7）
        p.setBrush(QColor(255, 230, 150, int(200 + pulse * 55)));
        // E 的横线
        p.drawRect(ix + 5, iy + 4,  14, 2);  // 顶
        p.drawRect(ix + 5, iy + 11, 14, 2);  // 中
        p.drawRect(ix + 5, iy + 18, 14, 2);  // 底
        // E 的竖线
        p.drawRect(ix + 5, iy + 4,  2, 16);  // 左竖

        // 下方小三角指示
        p.setBrush(QColor(255, 220, 100, int(120 + pulse * 80)));
        p.drawRect(ix + 9, iy + 26, 2, 2);
        p.drawRect(ix + 13, iy + 26, 2, 2);
        p.drawRect(ix + 7, iy + 28, 2, 2);
        p.drawRect(ix + 11, iy + 28, 2, 2);
        p.drawRect(ix + 15, iy + 28, 2, 2);
    }

    // 首次进入游戏的操作提示（5秒后淡出）
    if (!controlHintShown && controlHintTimer > 0) {
        // 计算淡出透明度
        int alpha = 220;
        if (controlHintTimer > 240) { // 最后1秒淡出
            alpha = int(220 * (300 - controlHintTimer) / 60.0);
            alpha = qMax(0, qMin(220, alpha));
        }

        // 面板尺寸与位置（屏幕底部居中）
        int panelW = 420;
        int panelH = 50;
        int panelX = (width() - panelW) / 2;
        int panelY = height() - 70;

        p.setPen(Qt::NoPen);

        // 面板背景
        p.setBrush(QColor(15, 12, 20, alpha));
        p.drawRoundedRect(panelX, panelY, panelW, panelH, 4, 4);

        // 面板边框
        p.setBrush(QColor(120, 110, 80, alpha));
        p.drawRect(panelX, panelY, panelW, 1);
        p.drawRect(panelX, panelY + panelH - 1, panelW, 1);
        p.drawRect(panelX, panelY, 1, panelH);
        p.drawRect(panelX + panelW - 1, panelY, 1, panelH);

        // 绘制4个按键提示：A/D 移动 | 空格 跳跃 | E 互动 | F 回溯
        struct KeyHint {
            QString key;
            QString label;
            QColor keyColor;
        };
        QList<KeyHint> hints = {
            {"A/D", "移动", QColor(180, 170, 140, alpha)},
            {"空格", "跳跃", QColor(180, 170, 140, alpha)},
            {"E", "互动", QColor(180, 170, 140, alpha)},
            {"F", "回溯", QColor(160, 150, 200, alpha)}
        };

        int spacing = panelW / hints.size();
        for (int i = 0; i < hints.size(); i++) {
            int cx = panelX + spacing * i + spacing / 2;

            // 按键框
            int keyW = hints[i].key.length() * 10 + 12;
            int keyH = 20;
            int keyX = cx - keyW / 2;
            int keyY = panelY + 6;

            p.setBrush(QColor(40, 35, 30, alpha));
            p.drawRect(keyX, keyY, keyW, keyH);
            // 按键边框
            p.setBrush(hints[i].keyColor);
            p.drawRect(keyX, keyY, keyW, 1);
            p.drawRect(keyX, keyY + keyH - 1, keyW, 1);
            p.drawRect(keyX, keyY, 1, keyH);
            p.drawRect(keyX + keyW - 1, keyY, 1, keyH);

            // 按键文字
            p.setPen(QColor(255, 240, 200, alpha));
            p.setFont(QFont("SimHei", 9, QFont::Bold));
            p.drawText(QRect(keyX, keyY, keyW, keyH), Qt::AlignCenter, hints[i].key);

            // 功能标签
            p.setPen(QColor(160, 155, 140, alpha));
            p.setFont(QFont("SimHei", 8));
            p.drawText(QRect(cx - 30, keyY + keyH + 2, 60, 16), Qt::AlignCenter, hints[i].label);
        }
    }
}

// ==================== 交互检测 ====================

bool GameWidget::isNearInteractable() const
{
    QRect interactRect = player.rect().adjusted(-8, -8, 8, 8);

    // 检测道具
    for (const auto &prop : scene.props()) {
        if (prop.picked()) continue;
        if (prop.label().isEmpty()) continue;
        // 幻梦机器在救龙虾前不显示交互提示
        if (prop.id() == "dream_machine" && !lobsterSaved) continue;
        if (interactRect.intersects(prop.rect()))
            return true;
    }

    // 检测NPC
    for (const auto &npc : scene.npcs()) {
        // 古井和石门：始终显示交互提示（不标记talked）
        if (npc.id() == "well" || npc.id() == "stone_door") {
            if (interactRect.intersects(npc.rect()))
                return true;
            continue;
        }
        if (npc.talked()) continue;
        if (interactRect.intersects(npc.rect()))
            return true;
    }

    // 检测出口
    for (const auto &exit : scene.exits()) {
        if (interactRect.intersects(exit.rect))
            return true;
    }

    return false;
}

void GameWidget::drawHintButton(QPainter &p)
{
    // 更新按钮位置（右上角）
    hintButtonRect = QRect(width() - 100, 10, 80, 30);

    // 按钮背景
    p.setPen(QColor(180, 160, 100));
    p.setBrush(QColor(40, 35, 30, 200));
    p.drawRect(hintButtonRect);

    // 按钮文字
    p.setPen(QColor(255, 220, 150));
    p.setFont(QFont("SimHei", 10, QFont::Bold));
    p.drawText(hintButtonRect, Qt::AlignCenter, "提示");

    // 绘制提示面板
    if (showHintPanel) {
        QRect panelRect(width() - 320, 50, 300, 150);

        // 面板背景
        p.setPen(QColor(120, 100, 80));
        p.setBrush(QColor(15, 12, 20, 230));
        p.drawRect(panelRect);

        // 内边框
        p.setPen(QColor(80, 70, 50, 150));
        p.setBrush(Qt::NoBrush);
        p.drawRect(panelRect.adjusted(4, 4, -4, -4));

        // 提示文字
        p.setPen(QColor(220, 220, 200));
        p.setFont(QFont("SimHei", 11));
        p.drawText(panelRect.adjusted(15, 15, -15, -15),
                   Qt::AlignLeft | Qt::TextWordWrap, hintText);

        // 关闭提示
        p.setPen(QColor(150, 150, 150));
        p.setFont(QFont("SimHei", 9));
        p.drawText(panelRect.right() - 80, panelRect.bottom() - 10, "点击关闭");
    }
}

// ==================== 像素地面绘制 ====================

void GameWidget::drawPixelGround(QPainter &p)
{
    int sceneId = player.sceneId;
    
    // 主地面（最底层平台）
    QRect mainGround(0, 620, 1280, 100);
    
    // 根据场景ID绘制不同风格的地面
    switch (sceneId) {
        case 0: // 破败教堂
            drawPixelPlatform(p, mainGround, 0);
            break;
        case 1: // 赛博朋克地下城
            drawPixelPlatform(p, mainGround, 1);
            break;
        case 2: // 沙漠金字塔
            drawPixelPlatform(p, mainGround, 2);
            break;
        case 3: // 末日火山
            drawPixelPlatform(p, mainGround, 3);
            break;
        case 4: // 图书馆秘境
            drawPixelPlatform(p, mainGround, 4);
            break;
        case 5: // 时空终局教堂
            drawPixelPlatform(p, mainGround, 5);
            break;
        default:
            drawPixelPlatform(p, mainGround, 0);
            break;
    }
    
    // 绘制其他平台
    for (auto &plat : scene.platforms()) {
        if (plat.top() >= 620) continue; // 跳过主地面
        drawPixelPlatform(p, plat, sceneId);
    }
}

void GameWidget::drawPixelPlatform(QPainter &p, QRect plat, int sceneId)
{
    p.setPen(Qt::NoPen);
    
    // 32x32像素瓦片大小
    const int TILE_SIZE = 32;
    
    // 根据场景ID设置颜色方案
    QColor baseColor, darkColor, lightColor, accentColor;
    QList<QColor> debrisColors; // 碎片/装饰颜色
    
    switch (sceneId) {
        case 0: // 破败教堂 - 冷灰主色调
            baseColor = QColor(60, 55, 50);      // 碎石灰
            darkColor = QColor(45, 42, 38);      // 深灰
            lightColor = QColor(75, 70, 65);     // 浅灰
            accentColor = QColor(80, 75, 70);    // 点缀色
            debrisColors << QColor(70, 65, 60)   // 碎石
                        << QColor(50, 48, 45)    // 灰尘
                        << QColor(40, 60, 35);   // 细小杂草
            break;
            
        case 1: // 赛博朋克地下城 - 冷蓝+暗紫金属质感
            baseColor = QColor(40, 42, 55);      // 金属地面
            darkColor = QColor(25, 28, 40);      // 深蓝紫
            lightColor = QColor(55, 58, 75);     // 浅金属
            accentColor = QColor(100, 80, 180);  // 霓虹紫
            debrisColors << QColor(80, 85, 100)  // 破碎芯片
                        << QColor(70, 75, 90)    // 机械碎屑
                        << QColor(150, 100, 200, 150); // 漏光像素
            break;
            
        case 2: // 沙漠金字塔 - 土黄+暗黄沙砾质地
            baseColor = QColor(140, 120, 70);    // 沙地
            darkColor = QColor(110, 95, 55);     // 暗黄
            lightColor = QColor(160, 140, 85);   // 浅沙
            accentColor = QColor(170, 150, 100); // 石砖
            debrisColors << QColor(150, 130, 80) // 细沙
                        << QColor(130, 110, 65)  // 碎石
                        << QColor(120, 100, 60); // 干燥裂纹
            break;
            
        case 3: // 末日火山 - 黑灰+暗红岩石质地
            baseColor = QColor(50, 35, 30);      // 焦黑
            darkColor = QColor(35, 25, 22);      // 深黑
            lightColor = QColor(70, 50, 45);     // 浅灰
            accentColor = QColor(150, 50, 30);   // 熔红
            debrisColors << QColor(60, 40, 35)   // 灰烬
                        << QColor(180, 60, 40)   // 熔融痕迹
                        << QColor(255, 100, 50); // 火星像素
            break;
            
        case 4: // 图书馆秘境 - 淡紫+米白旧木石混合质地
            baseColor = QColor(70, 65, 60);      // 朽木地板
            darkColor = QColor(55, 50, 48);      // 深木
            lightColor = QColor(90, 85, 80);     // 浅木
            accentColor = QColor(100, 95, 110);  // 淡紫
            debrisColors << QColor(200, 190, 170)// 碎纸
                        << QColor(80, 60, 45)    // 木屑
                        << QColor(180, 175, 165);// 尘埃
            break;
            
        case 5: // 时空终局教堂 - 与破败教堂一致+银白流光
            baseColor = QColor(60, 55, 50);      // 碎石灰
            darkColor = QColor(45, 42, 38);      // 深灰
            lightColor = QColor(75, 70, 65);     // 浅灰
            accentColor = QColor(200, 200, 220); // 银白流光
            debrisColors << QColor(70, 65, 60)   // 碎石
                        << QColor(50, 48, 45)    // 灰尘
                        << QColor(220, 220, 240, 150); // 银白流光像素
            break;
            
        default:
            baseColor = QColor(60, 55, 50);
            darkColor = QColor(45, 42, 38);
            lightColor = QColor(75, 70, 65);
            accentColor = QColor(80, 75, 70);
            debrisColors << QColor(70, 65, 60);
            break;
    }
    
    // 绘制基础地面
    p.setBrush(baseColor);
    p.drawRect(plat);
    
    // 绘制像素瓦片纹理
    int startX = plat.left() - (plat.left() % TILE_SIZE);
    int startY = plat.top() - (plat.top() % TILE_SIZE);
    
    // 使用固定种子生成一致的随机效果
    unsigned int seed = plat.x() * 7 + plat.y() * 13 + sceneId * 17;
    
    for (int y = startY; y < plat.bottom(); y += TILE_SIZE) {
        for (int x = startX; x < plat.right(); x += TILE_SIZE) {
            // 简单伪随机
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            int randVal = seed % 100;
            
            QRect tileRect(x, y, TILE_SIZE, TILE_SIZE);
            if (!plat.contains(tileRect)) {
                tileRect = tileRect.intersected(plat);
            }
            
            // 绘制瓦片基础色（颜色深浅错落）
            if (randVal < 30) {
                p.setBrush(darkColor);
            } else if (randVal < 60) {
                p.setBrush(lightColor);
            } else {
                p.setBrush(baseColor);
            }
            p.drawRect(tileRect);
            
            // 绘制裂纹/裂缝
            if (randVal > 85 && tileRect.width() >= TILE_SIZE && tileRect.height() >= TILE_SIZE) {
                p.setBrush(darkColor.darker(120));
                // 横向裂纹
                int crackY = y + (seed % 24) + 4;
                for (int cx = 0; cx < TILE_SIZE - 4; cx += 4) {
                    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
                    int offset = (seed % 3) - 1;
                    p.drawRect(x + cx, crackY + offset, 3, 1);
                }
            }
            
            // 绘制碎片/装饰像素点
            if (randVal > 70 && !debrisColors.isEmpty()) {
                seed = (seed * 1103515245 + 12345) & 0x7fffffff;
                QColor debrisColor = debrisColors[seed % debrisColors.size()];
                p.setBrush(debrisColor);
                
                // 随机位置的小像素点
                int dx = x + (seed % 28) + 2;
                int dy = y + ((seed * 7) % 28) + 2;
                if (plat.contains(dx, dy)) {
                    p.drawRect(dx, dy, 2, 2);
                }
            }
            
            // 场景特效
            if (sceneId == 1 && randVal > 90) {
                // 赛博朋克：裂缝中漏微光
                seed = (seed * 1103515245 + 12345) & 0x7fffffff;
                p.setBrush(QColor(100, 150, 255, 100 + (seed % 100)));
                int lx = x + (seed % 20) + 6;
                int ly = y + ((seed * 3) % 20) + 6;
                p.drawRect(lx, ly, 2, 2);
            }
            
            if (sceneId == 3 && randVal > 92) {
                // 火山：零星火星
                seed = (seed * 1103515245 + 12345) & 0x7fffffff;
                p.setBrush(QColor(255, 150 + (seed % 100), 50, 150 + (seed % 100)));
                int mx = x + (seed % 24) + 4;
                int my = y + ((seed * 5) % 24) + 4;
                p.drawRect(mx, my, 2, 2);
            }
            
            if (sceneId == 5 && randVal > 88) {
                // 终局教堂：银白流光像素点
                seed = (seed * 1103515245 + 12345) & 0x7fffffff;
                int alpha = 80 + (seed % 120);
                p.setBrush(QColor(220, 220, 240, alpha));
                int sx = x + (seed % 26) + 3;
                int sy = y + ((seed * 7) % 26) + 3;
                p.drawRect(sx, sy, 2, 2);
            }
        }
    }
    
    // 绘制边缘磨损效果
    p.setBrush(darkColor);
    // 顶部边缘
    for (int x = plat.left(); x < plat.right(); x += 4) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        if (seed % 100 > 60) {
            int h = 1 + (seed % 3);
            p.drawRect(x, plat.top(), 4, h);
        }
    }
    
    // 底部边缘
    for (int x = plat.left(); x < plat.right(); x += 4) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        if (seed % 100 > 70) {
            int h = 1 + (seed % 2);
            p.drawRect(x, plat.bottom() - h, 4, h);
        }
    }
    
    // 左右边缘
    for (int y = plat.top(); y < plat.bottom(); y += 4) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        if (seed % 100 > 75) {
            p.drawRect(plat.left(), y, 1 + (seed % 2), 4);
        }
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        if (seed % 100 > 75) {
            p.drawRect(plat.right() - 1 - (seed % 2), y, 1 + (seed % 2), 4);
        }
    }
    
    // 平台顶部高亮线（像素风格）
    p.setBrush(lightColor);
    for (int x = plat.left(); x < plat.right(); x += 2) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        if (seed % 100 > 40) {
            p.drawRect(x, plat.top(), 2, 1);
        }
    }
    
    // 场景特殊装饰
    if (sceneId == 2) {
        // 沙漠：黄沙飘动像素点
        seed = plat.x() * 7 + plat.y() * 13;
        for (int i = 0; i < 15; i++) {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            int sx = plat.left() + (seed % plat.width());
            int sy = plat.top() - 3 - (seed % 8);
            p.setBrush(QColor(180, 160, 100, 100 + (seed % 100)));
            p.drawRect(sx, sy, 2, 2);
        }
    }
    
    if (sceneId == 4) {
        // 图书馆：飘落尘埃像素
        seed = plat.x() * 7 + plat.y() * 13;
        for (int i = 0; i < 10; i++) {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            int dx = plat.left() + (seed % plat.width());
            int dy = plat.top() - 2 - (seed % 5);
            p.setBrush(QColor(200, 195, 180, 80 + (seed % 80)));
            p.drawRect(dx, dy, 1, 1);
        }
    }
}

// ==================== 图片处理 ====================

QPixmap GameWidget::removeBackground(const QPixmap &pixmap, QColor bgColor, int threshold)
{
    if (pixmap.isNull()) return pixmap;

    // 转换为 QImage 以便逐像素操作
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);

    int bgR = bgColor.red();
    int bgG = bgColor.green();
    int bgB = bgColor.blue();

    for (int y = 0; y < img.height(); y++) {
        QRgb *line = reinterpret_cast<QRgb *>(img.scanLine(y));
        for (int x = 0; x < img.width(); x++) {
            QRgb pixel = line[x];
            int r = qRed(pixel);
            int g = qGreen(pixel);
            int b = qBlue(pixel);

            // 计算与背景色的欧几里得距离
            int dist = std::sqrt((r - bgR) * (r - bgR) +
                                 (g - bgG) * (g - bgG) +
                                 (b - bgB) * (b - bgB));

            // 距离小于阈值则设为全透明
            if (dist < threshold) {
                line[x] = qRgba(0, 0, 0, 0);
            }
            // 边缘柔化：距离在阈值~阈值*1.5之间，半透明过渡
            else if (dist < threshold * 1.5) {
                float alpha = (dist - threshold) / (threshold * 0.5f);
                alpha = qMin(1.0f, qMax(0.0f, alpha));
                line[x] = qRgba(r, g, b, int(alpha * 255));
            }
        }
    }

    return QPixmap::fromImage(img);
}

// ==================== 卷轴背景（视差滚动） ====================

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
