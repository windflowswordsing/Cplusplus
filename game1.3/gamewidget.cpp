#include "gamewidget.h"
#include <QKeyEvent>
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
    churchStartShown = false;
    loopShockShown = false;
    swordPicked = false;
    seedPicked = false;
    lobsterSaved = false;
    titleStarCount = 60;
    showHintButton = false;
    showHintPanel = false;
    hintButtonRect = QRect(width() - 100, 10, 80, 30);
    hintText = "";

    // 加载背景图片（使用绝对路径）
    bgChurch.load("C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/DreamJourney/bg_church.png");

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
    player.update();

    // 平台碰撞
    scene.load(player.sceneId);
    for (auto &plat : scene.platforms()) {
        if (player.rect().intersects(plat)) {
            // 简单碰撞修正
            int overlapBottom = player.rect().bottom() - plat.top();
            int overlapTop = plat.bottom() - player.rect().top();
            int overlapLeft = player.rect().right() - plat.left();
            int overlapRight = plat.right() - player.rect().left();

            int minOverlap = overlapBottom;
            if (overlapTop < minOverlap) minOverlap = overlapTop;
            if (overlapLeft < minOverlap) minOverlap = overlapLeft;
            if (overlapRight < minOverlap) minOverlap = overlapRight;

            if (minOverlap == overlapBottom && player.rect().top() < plat.top()) {
                player.rect().moveBottom(plat.top());
            } else if (minOverlap == overlapTop) {
                player.rect().moveTop(plat.bottom());
            } else if (minOverlap == overlapLeft) {
                player.rect().moveRight(plat.left());
            } else {
                player.rect().moveLeft(plat.right());
            }
        }
    }

    // 防止掉出地图
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
    scene.load(player.sceneId);

    // 道具碰撞
    for (auto &prop : scene.props()) {
        if (player.rect().intersects(prop.rect()) && !prop.picked()) {
            QString id = prop.id();

            // 特殊道具处理
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
        if (player.rect().intersects(npc.rect())) {
            QString id = npc.id();

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
        if (player.rect().intersects(exit.rect)) {
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
    state = GameState::Flashback;
}

void GameWidget::updateFlashback()
{
    flashbackTimer--;

    if (flashbackTimer <= 0) {
        // 场景5后回溯永久耗尽
        if (player.sceneId == 4) {
            flashbackUsed = true;
        }

        // 回到对话状态显示残影对话
        if (!currentDialogKey.isEmpty()) {
            DialogData fb = dialogue.getFlashback(scene.flashbackTrigger());
            currentDialog = fb;
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
            state = GameState::Playing;
            scene.load(0);
            // 显示提示按钮，不阻塞
            churchStartShown = true;
            showHintButton = true;
            hintText = "这里就是被召唤的世界？没有生机，没有战火，只有废墟……\n召唤阵还在，可一切都太晚了。";
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

    // H键显示/隐藏提示
    if (e->key() == Qt::Key_H) {
        showHintButton = !showHintButton;
    }
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
    scene.load(player.sceneId);

    // 背景 - 教堂场景使用图片
    if (player.sceneId == 0 || player.sceneId == 5) {
        if (!bgChurch.isNull()) {
            // 绘制背景图片，保持比例填充
            p.drawPixmap(rect(), bgChurch.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        } else {
            p.fillRect(rect(), scene.bgColor());
        }
    } else {
        p.fillRect(rect(), scene.bgColor());
    }

    // 地面装饰线
    p.setPen(scene.groundColor().lighter(120));
    p.drawLine(0, 620 - camX * 0 + 620, width(), 620);

    p.save();
    p.translate(-camX, 0);

    // 平台
    for (auto &plat : scene.platforms()) {
        drawPixelRect(p, plat, scene.groundColor());
        // 平台顶部高亮
        p.setPen(scene.groundColor().lighter(150));
        p.drawLine(plat.topLeft(), plat.topRight());
    }

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

    // 身体（深蓝色像素块）
    p.setPen(Qt::black);
    p.setBrush(QColor(60, 80, 140));
    p.drawRect(r.adjusted(4, 12, -4, -10));

    // 头部
    p.setBrush(QColor(220, 190, 160));
    p.drawRect(r.adjusted(6, 0, -6, -20));

    // 头发
    p.setBrush(QColor(40, 30, 20));
    p.drawRect(r.adjusted(5, -2, -5, -22));

    // 眼睛
    p.setBrush(Qt::white);
    int eyeX = player.facingRight ? r.x() + 14 : r.x() + 6;
    p.drawRect(eyeX, r.y() + 6, 4, 4);
    p.setBrush(Qt::black);
    int pupilX = player.facingRight ? eyeX + 2 : eyeX;
    p.drawRect(pupilX, r.y() + 7, 2, 2);

    // 腿
    p.setBrush(QColor(40, 40, 100));
    p.drawRect(r.x() + 5, r.bottom() - 10, 6, 10);
    p.drawRect(r.right() - 11, r.bottom() - 10, 6, 10);

    // 手持断剑
    if (player.hasProp("broken_sword")) {
        p.setBrush(QColor(180, 180, 200));
        int swordX = player.facingRight ? r.right() + 2 : r.left() - 4;
        p.drawRect(swordX, r.y() + 8, 3, 32);
        p.setBrush(QColor(140, 140, 160));
        p.drawRect(swordX - 2, r.y() + 6, 7, 4); // 剑柄
    }
}

void GameWidget::drawProps(QPainter &p)
{
    for (auto &prop : scene.props()) {
        if (prop.picked()) continue;
        if (prop.label().isEmpty()) continue; // 装饰性道具

        QRect r = prop.rect();
        QColor c = prop.color();

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
        // 继续提示
        float blink = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) + 1) * 0.5;
        p.setPen(QColor(180, 180, 180, int(100 + blink * 155)));
        p.setFont(QFont("SimHei", 10));
        p.drawText(boxX + boxW - 160, boxY + boxH - 15, "按 E / 空格 继续");
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
    scene.load(player.sceneId);

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
