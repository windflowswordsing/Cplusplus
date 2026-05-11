#include "gamewidget.h"
#include <QKeyEvent>
#include <QCoreApplication>
#include <QDir>
#include <cmath>

GameWidget::GameWidget(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    state = GameState::Intro;
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
    lobsterFreed = false;
    wellCleared = false;
    dreamMachineInteracted = false;
    coffinUnlocked = false;
    pendingTeleport = false;
    pendingTeleportScene = -1;
    libraryFlashbackShown = false;
    seedDiscovered = false;
    manuscriptRevealed = false;
    returnedFromScene6 = false;
    swordDropped = false;
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

    // 加载玩家精灵图（自动去白色底色，阈值60以去除浅灰背景）
    playerIdle = removeBackground(QPixmap(resDir + "/player_idle.png"), bgColor, 60);
    playerWalkR << removeBackground(QPixmap(resDir + "/player_walk_r1.png"), bgColor, 60)
                << removeBackground(QPixmap(resDir + "/player_walk_r2.png"), bgColor, 60)
                << removeBackground(QPixmap(resDir + "/player_walk_r3.png"), bgColor, 60);
    playerWalkL << removeBackground(QPixmap(resDir + "/player_walk_l1.png"), bgColor, 60)
                << removeBackground(QPixmap(resDir + "/player_walk_l2.png"), bgColor, 60)
                << removeBackground(QPixmap(resDir + "/player_walk_l3.png"), bgColor, 60);
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
    if (state == GameState::Intro || state == GameState::Title || state == GameState::Ending) {
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

    // 场景1幻梦机器交互检测（固定位置，不依赖道具列表）
    if (player.sceneId == 1) {
        QRect dreamMachineRect(600 - 100, 540 - 100, 200, 200); // 图片区域
        if (interactRect.intersects(dreamMachineRect)) {
            // 救下龙虾前幻梦机器无法互动
            if (!lobsterSaved) {
                // 显示提示：需要先救龙虾
                currentDialog = dialogue.getDialog("dream_machine_locked");
                currentDialogKey = "dream_machine_locked";
                dialogHasChoices = false;
                state = GameState::Dialog;
                return;
            }
            // 已发现种子：显示完整选择（包括回到原来世界）
            if (seedDiscovered) {
                DialogData dd = dialogue.getDialog("dream_machine_with_seed");
                if (!dd.text.isEmpty()) {
                    currentDialog = dd;
                    currentDialogKey = "dream_machine_with_seed";
                    dialogHasChoices = !dd.choices.isEmpty();
                    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                }
                return;
            }
            // 救下龙虾后解锁幻梦机器互动
            if (!dreamMachineInteracted) {
                dreamMachineInteracted = true;
                DialogData dd = dialogue.getDialog("dream_machine");
                if (!dd.text.isEmpty()) {
                    currentDialog = dd;
                    currentDialogKey = "dream_machine";
                    dialogHasChoices = !dd.choices.isEmpty();
                    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                }
                return;
            }
        }

        // 场景1全息光屏交互检测（固定位置）
        QRect holoScreenRect(900 - 60, 570 - 75, 120, 150); // 图片区域
        if (interactRect.intersects(holoScreenRect)) {
            currentDialog = dialogue.getDialog("holo_screen");
            currentDialogKey = "holo_screen";
            dialogHasChoices = false;
            state = GameState::Dialog;
            return;
        }
    }

    // 场景2图片道具交互检测（壁画、铭文、石棺）
    if (player.sceneId == 2) {
        // 壁画
        QRect paintingRect(350, 490, 200, 120);
        if (interactRect.intersects(paintingRect)) {
            currentDialog = dialogue.getDialog("wall_painting");
            currentDialogKey = "wall_painting";
            dialogHasChoices = false;
            state = GameState::Dialog;
            return;
        }
        // 铭文石碑（互动后解锁石棺）
        QRect tabletRect(600, 520, 80, 100);
        if (interactRect.intersects(tabletRect)) {
            coffinUnlocked = true; // 解锁石棺
            currentDialog = dialogue.getDialog("stone_tablet");
            currentDialogKey = "stone_tablet";
            dialogHasChoices = false;
            state = GameState::Dialog;
            return;
        }
        // 石棺（检查是否已解锁）
        QRect coffinRect(830, 540, 120, 80);
        if (interactRect.intersects(coffinRect)) {
            if (!coffinUnlocked) {
                // 未解锁：显示提示
                currentDialog = dialogue.getDialog("stone_coffin_locked");
                currentDialogKey = "stone_coffin_locked";
                dialogHasChoices = false;
                state = GameState::Dialog;
            } else {
                // 已解锁：显示选择
                currentDialog = dialogue.getDialog("stone_coffin");
                currentDialogKey = "stone_coffin";
                dialogHasChoices = !dialogue.getDialog("stone_coffin").choices.isEmpty();
                state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
            }
            return;
        }
    }

    // 道具碰撞
    for (auto &prop : scene.props()) {
        // 断剑在拾取后仍然可以交互（用于传送），其他道具需要未拾取
        bool canInteract = !prop.picked() || prop.id() == "broken_sword";
        if (interactRect.intersects(prop.rect()) && canInteract) {
            QString id = prop.id();
            if (id == "broken_sword" && !swordPicked) {
                // 断剑需要先通过幽灵对话才能看到
                if (!swordDropped) continue;
                // 触发取走断剑的选择
                DialogData dd = dialogue.getDialog("sword_ghost_choice");
                if (!dd.text.isEmpty()) {
                    currentDialog = dd;
                    currentDialogKey = "sword_ghost_choice";
                    dialogHasChoices = !dd.choices.isEmpty();
                    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                    return;
                }
            }
            // 断剑已拾取后，再次交互传送到场景五（图书馆）
            if (id == "broken_sword" && swordPicked && player.sceneId == 3) {
                player.sceneId = 4;
                scene.load(4);
                player.setPos(100, 540);
                state = GameState::Playing;
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

            // 魔法书（地面）：按E互动
            if (id == "manuscript_hidden") {
                if (!seedDiscovered) {
                    // 未发现种子：显示提示对话
                    currentDialog = dialogue.getDialog("manuscript_locked");
                    currentDialogKey = "manuscript_locked";
                    dialogHasChoices = false;
                    state = GameState::Dialog;
                } else {
                    // 已发现种子：显示选择
                    DialogData dd = dialogue.getDialog("manuscript_choice");
                    currentDialog = dd;
                    currentDialogKey = "manuscript_choice";
                    dialogHasChoices = !dd.choices.isEmpty();
                    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                }
                return;
            }

            // 回溯触发物品：与特定物品交互进入回溯场景
            if (id == "church_flashback_item" && !flashbackUsedScenes.contains(0)) {
                flashbackUsedScenes.insert(0);
                startFlashback("church");
                return;
            }
            if (id == "dungeon_flashback_item" && !flashbackUsedScenes.contains(1)) {
                flashbackUsedScenes.insert(1);
                startFlashback("dungeon");
                return;
            }
            if (id == "pyramid_flashback_item" && !flashbackUsedScenes.contains(2)) {
                flashbackUsedScenes.insert(2);
                startFlashback("pyramid");
                return;
            }
            if (id == "volcano_flashback_item" && !flashbackUsedScenes.contains(3)) {
                flashbackUsedScenes.insert(3);
                startFlashback("volcano");
                return;
            }
            if (id == "library_flashback_item" && !flashbackUsedScenes.contains(4)) {
                flashbackUsedScenes.insert(4);
                startFlashback("library");
                return;
            }

            // 种子容器：按E互动后出现种子
            if (id == "seed_vessel" && !seedDiscovered) {
                seedDiscovered = true;
                currentDialog = dialogue.getDialog("seed_discovered");
                currentDialogKey = "seed_discovered";
                dialogHasChoices = false;
                state = GameState::Dialog;
                return;
            }

            // 种子（发现后）：按E互动触发选择
            if (id == "seed" && seedDiscovered) {
                DialogData dd = dialogue.getDialog("seed_choice");
                currentDialog = dd;
                currentDialogKey = "seed_choice";
                dialogHasChoices = !dd.choices.isEmpty();
                state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                return;
            }

            // 传送门：按E互动回到场景1
            if (id == "portal_door") {
                player.sceneId = 0;
                scene.load(0);
                player.setPos(100, 300);
                returnedFromScene6 = true;  // 标记从场景六返回
                state = GameState::Playing;
                return;
            }

            // 场景5法阵：触发最终选择
            if (id == "magic_circle" && player.sceneId == 5 && player.hasProp("seed")) {
                DialogData dd = dialogue.getDialog("final_choice");
                currentDialog = dd;
                currentDialogKey = "final_choice";
                dialogHasChoices = !dd.choices.isEmpty();
                state = GameState::Choice;
                return;
            }

            // 场景0法阵：触发对话和回溯
            if (id == "magic_circle" && player.sceneId == 0) {
                currentDialog = dialogue.getDialog("church_circle");
                currentDialogKey = "church_circle";
                dialogHasChoices = false;
                state = GameState::Dialog;
                return;
            }

            // 场景0神像：触发对话和回溯（包含长椅对话内容）
            if (id == "church_statue" && player.sceneId == 0) {
                currentDialog = dialogue.getDialog("church_statue");
                currentDialogKey = "church_statue";
                dialogHasChoices = false;
                state = GameState::Dialog;
                return;
            }

            // 场景0烛台：触发对话和回溯
            if (id == "church_candle" && player.sceneId == 0) {
                currentDialog = dialogue.getDialog("church_candle");
                currentDialogKey = "church_candle";
                dialogHasChoices = false;
                state = GameState::Dialog;
                return;
            }

            // 古木门：不拾取，只作为出口提示
            if (id == "old_door") {
                continue;  // 跳过，让出口检测处理
            }

            // 通用道具拾取
            if (!prop.label().isEmpty()) {
                player.addProp(id);
                prop.pick();

                // 检查是否有对应对话（道具ID到对话key的映射）
                QString dialogKey = id;
                if (id == "seed") dialogKey = "pick_seed";
                if (id == "broken_sword") dialogKey = "pick_sword";
                
                DialogData dd = dialogue.getDialog(dialogKey);
                if (!dd.text.isEmpty()) {
                    currentDialog = dd;
                    currentDialogKey = dialogKey;
                    dialogHasChoices = !dd.choices.isEmpty();
                    state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
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
                if (!lobsterFreed) {
                    // 第一次互动：释放龙虾，切换图片
                    lobsterFreed = true;
                    return;
                } else {
                    // 第二次互动：触发对话
                    lobsterSaved = true;
                    npc.setTalked(true);
                    currentDialog = dialogue.getDialog("lobster");
                    currentDialogKey = "lobster";
                    dialogHasChoices = false;
                    state = GameState::Dialog;
                    return;
                }
            }

            if (id == "sword_ghost" && swordPicked) {
                // 已拾取断剑，幽魂消散，不再触发
                continue;
            }

            // 守剑幽魂：第一次互动触发对话链，对话结束后出现选择
            if (id == "sword_ghost" && !swordPicked) {
                if (!swordDropped) {
                    // 第一次按E：触发幽灵对话链
                    swordDropped = true;  // 标记已对话（但不掉落断剑，等选择后）
                    DialogData dd = dialogue.getDialog("volcano_ghost");
                    if (!dd.text.isEmpty()) {
                        currentDialog = dd;
                        currentDialogKey = "volcano_ghost";
                        dialogHasChoices = false;
                        state = GameState::Dialog;
                        return;
                    }
                }
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

// ==================== 重置游戏 ====================

void GameWidget::resetGame()
{
    // 重置玩家状态
    player.clearProps();
    player.sceneId = 0;
    player.setPos(100, 300);
    
    // 重置所有游戏标记
    flashbackUsed = false;
    showingFlashbackBg = false;
    flashbackUsedScenes.clear();
    churchStartShown = false;
    loopShockShown = false;
    swordPicked = false;
    seedPicked = false;
    lobsterSaved = false;
    lobsterFreed = false;
    wellCleared = false;
    dreamMachineInteracted = false;
    coffinUnlocked = false;
    pendingTeleport = false;
    pendingTeleportScene = -1;
    libraryFlashbackShown = false;
    seedDiscovered = false;
    manuscriptRevealed = false;
    returnedFromScene6 = false;
    swordDropped = false;
    
    // 重置结局
    endingType = 0;
    endingTimer = 0;
    
    // 重置对话系统
    currentDialog = DialogData();
    currentDialogKey = "";
    dialogHasChoices = false;
    
    // 重置提示
    showHintButton = false;
    showHintPanel = false;
    hintText = "";
    controlHintShown = false;
    controlHintTimer = 0;
    
    // 重新加载场景
    scene.load(0);
    
    // 切换到标题画面
    state = GameState::Title;
}

// ==================== 场景切换 ====================

void GameWidget::changeScene(int targetScene, int spawnX, int spawnY)
{
    player.sceneId = targetScene;
    player.setPos(spawnX, spawnY);
    scene.load(targetScene);

    // 根据场景更新提示
    switch (targetScene) {
    case 0: hintText = "地面似乎有奇怪的图案\n古井似乎联通着哪里"; break;
    case 1: hintText = "地下城弥漫着诡异的蓝光\n一只龙虾被困在废墟中"; break;
    case 2: hintText = "沙漠中矗立着古老的金字塔\n石棺中似乎藏着永生的秘密"; break;
    case 3: hintText = "火山口散发着灼热的气息\n断剑的幽魂在此守候千年"; break;
    case 4: hintText = "图书馆秘境中藏着古老的智慧\n幻梦机器等待被启动"; break;
    case 5: hintText = "一切回到了起点……"; break;
    default: hintText = ""; break;
    }

    // 场景进入事件
    // 场景0（教堂）：首次进入自动触发纯净回溯
    if (targetScene == 0 && !churchStartShown) {
        churchStartShown = true;
        flashbackUsedScenes.insert(0); // 标记回溯已使用
        // 纯净回溯：只有背景色+对话，无角色/NPC/道具/地砖
        flashbackBgColor = scene.flashbackColor();
        showingFlashbackBg = true;
        DialogData fb = dialogue.getDialog("church_auto_flashback");
        if (!fb.text.isEmpty()) {
            currentDialog = fb;
            currentDialogKey = "church_auto_flashback";
            dialogHasChoices = false;
            state = GameState::Dialog;
        } else {
            state = GameState::Playing;
        }
    }
    // 场景4（图书馆）：进入自动触发纯净回溯（圣女独白）
    else if (targetScene == 4 && !libraryFlashbackShown) {
        libraryFlashbackShown = true;
        flashbackUsedScenes.insert(4); // 标记回溯已使用
        flashbackBgColor = scene.flashbackColor();
        showingFlashbackBg = true;
        DialogData fb = dialogue.getDialog("library_auto_flashback");
        if (!fb.text.isEmpty()) {
            currentDialog = fb;
            currentDialogKey = "library_auto_flashback";
            dialogHasChoices = false;
            state = GameState::Dialog;
        } else {
            state = GameState::Playing;
        }
    }
    // 场景5（终局）
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
    flashbackBgColor = scene.flashbackColor();
    flashbackText = dialogue.getFlashback(trigger).text;
    showingFlashbackBg = true;  // 切换为回溯盛景背景
    
    // 直接进入对话状态
    DialogData fb = dialogue.getFlashback(trigger);
    if (!fb.text.isEmpty()) {
        currentDialog = fb;
        currentDialogKey = trigger + "_flashback";
        // 如果有多段对话，设置nextKey
        dialogHasChoices = !fb.choices.isEmpty();
        state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
    } else {
        state = GameState::Playing;
    }
}

void GameWidget::startLibraryFlashback()
{
    // 场景4自动回溯：圣女内心矛盾独白
    flashbackBgColor = scene.flashbackColor();
    showingFlashbackBg = true;
    
    DialogData fb = dialogue.getDialog("library_auto_flashback");
    if (!fb.text.isEmpty()) {
        currentDialog = fb;
        currentDialogKey = "library_auto_flashback";
        dialogHasChoices = false;
        state = GameState::Dialog;
    } else {
        state = GameState::Playing;
    }
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
    else if (action == "dream_teleport") {
        // 幻梦机器传送至场景2（金字塔）
        player.sceneId = 2;
        scene.load(2);
        player.setPos(100, 540);
        state = GameState::Playing;
    }
    else if (action == "take_sword") {
        // 选择取走断剑：触发回溯对话（volcano_sword_flash）
        flashbackBgColor = scene.flashbackColor();
        showingFlashbackBg = true;
        DialogData dd = dialogue.getDialog("volcano_sword_flash");
        if (!dd.text.isEmpty()) {
            currentDialog = dd;
            currentDialogKey = "volcano_sword_flash";
            dialogHasChoices = false;
            state = GameState::Dialog;
        }
    }
    else if (action == "leave_ghost") {
        // 离开幽魂
        state = GameState::Playing;
    }
    else if (action == "coffin_enter") {
        triggerEnding(5); // 枯骨永眠
    }
    else if (action == "coffin_teleport") {
        // 石棺暗道传送至场景3（火山）
        player.sceneId = 3;
        scene.load(3);
        player.setPos(100, 540);
        state = GameState::Playing;
    }
    else if (action == "leave_coffin") {
        state = GameState::Playing;
    }
    else if (action == "study_manuscript") {
        // 研究手稿触发结局（逆时赴约）
        triggerEnding(3);
    }
    else if (action == "leave_manuscript") {
        state = GameState::Playing;
    }
    else if (action == "use_seed") {
        // 使用种子触发结局（文明新生）
        triggerEnding(1);
    }
    else if (action == "leave_seed") {
        state = GameState::Playing;
    }
    else if (action == "return_original_world") {
        // 回到原来的世界（逆时赴约结局）
        triggerEnding(3);
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
    // 开场画面
    if (state == GameState::Intro) {
        if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
            state = GameState::Title;
        }
        return;
    }
    // 标题画面
    if (state == GameState::Title) {
        if (e->key() == Qt::Key_Space || e->key() == Qt::Key_Return) {
            // 先加载场景，然后触发开场对话
            scene.load(0); // 加载第一个场景
            churchStartShown = true;
            flashbackUsedScenes.insert(0); // 标记回溯已使用
            // 触发开场对话（不使用回溯背景，显示正常游戏场景）
            showingFlashbackBg = false;
            currentDialog = dialogue.getDialog("church_enter");
            currentDialogKey = "church_enter";
            dialogHasChoices = false;
            state = GameState::Dialog;
            showHintButton = true;
            hintText = "这里就是被召唤的世界？没有生机，没有战火，只有废墟……\n召唤阵还在，可一切都太晚了。";
            controlHintTimer = 0;
        }
        return;
    }

    // 结局画面
    if (state == GameState::Ending) {
        // 重置游戏
        resetGame();
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
                // 如果新对话是回溯对话，设置回溯背景
                if (currentDialog.isFlashback) {
                    flashbackBgColor = scene.flashbackColor();
                    showingFlashbackBg = true;
                }
            } else {
                // 对话结束
                // 如果是开场自动回溯结束，继续显示开场对话
                if (currentDialogKey == "church_auto_flashback") {
                    currentDialog = dialogue.getDialog("church_start");
                    currentDialogKey = "church_start";
                    dialogHasChoices = false;
                    // 保持回溯背景，继续对话状态
                    return;
                }
                // 如果是图书馆自动回溯结束，继续显示进入对话
                if (currentDialogKey == "library_auto_flashback") {
                    currentDialog = dialogue.getDialog("library_enter");
                    currentDialogKey = "library_enter";
                    dialogHasChoices = false;
                    // 关闭回溯背景，继续对话状态
                    showingFlashbackBg = false;
                    return;
                }
                // 如果当前对话是回溯对话（flashback标记），继续显示下一段回溯对话
                if (currentDialog.isFlashback && !currentDialog.nextKey.isEmpty()) {
                    currentDialog = dialogue.getDialog(currentDialog.nextKey);
                    currentDialogKey = currentDialog.nextKey;
                    dialogHasChoices = !currentDialog.choices.isEmpty();
                    // 保持回溯背景
                    showingFlashbackBg = true;
                    return;
                }
                // 幽灵对话链结束后，触发选择（取走断剑/离开）
                if (currentDialogKey == "volcano_ghost_5") {
                    DialogData dd = dialogue.getDialog("volcano_sword_choice");
                    if (!dd.text.isEmpty()) {
                        currentDialog = dd;
                        currentDialogKey = "volcano_sword_choice";
                        dialogHasChoices = !dd.choices.isEmpty();
                        state = dialogHasChoices ? GameState::Choice : GameState::Dialog;
                        return;
                    }
                }
                state = GameState::Playing;
                // 如果是回溯对话结束，关闭回溯背景
                if (showingFlashbackBg) {
                    showingFlashbackBg = false;
                    // 幽灵断剑回溯结束：幽灵消失，断剑出现，标记可拾取
                    if (currentDialogKey == "volcano_sword_flash_3") {
                        swordPicked = true;
                        player.addProp("broken_sword");
                        for (auto &prop : scene.props()) {
                            if (prop.id() == "broken_sword") {
                                prop.pick();
                                break;
                            }
                        }
                    }
                }
                // 检查是否有待执行的传送
                if (pendingTeleport && pendingTeleportScene >= 0) {
                    player.sceneId = pendingTeleportScene;
                    scene.load(pendingTeleportScene);
                    player.setPos(100, 540);
                    pendingTeleport = false;
                    pendingTeleportScene = -1;
                    // 如果传送到场景4，自动触发回溯
                    if (player.sceneId == 4 && !libraryFlashbackShown) {
                        libraryFlashbackShown = true;
                        startLibraryFlashback();
                    }
                }
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

    // F键回溯功能已删除，改为与特定物品交互进入回溯场景

    // H键显示/隐藏提示（已改为左上角直接显示，此功能保留但简化）
    if (e->key() == Qt::Key_H) {
        // 提示现在直接显示在左上角，不需要切换
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
    // 鼠标点击事件（提示已改为左上角直接显示，无需处理）
    Q_UNUSED(e)
}

// ==================== 渲染 ====================

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false); // 像素风格，关闭抗锯齿

    if (state == GameState::Intro) {
        drawIntro(p);
        return;
    }
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

    // 提示文字已移到左上角直接显示（在drawHUD中）
}

void GameWidget::drawIntro(QPainter &p)
{
    // 开场画面：深色背景 + 诗句
    p.fillRect(rect(), QColor(10, 8, 15));

    // 诗句逐行显示
    QFont poemFont("SimHei", 18);
    p.setFont(poemFont);
    p.setPen(QColor(200, 190, 170));

    QString line1 = "山 河 有 尽";
    QString line2 = "文 明 有 痕";
    QString line3 = "有 缘 之 人";
    QString line4 = "循 迹 而 来";

    int startY = height() / 2 - 80;

    // 淡入效果
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() % 100000;
    int alpha1 = qMin(255, (int)(elapsed / 40));
    int alpha2 = qMin(255, qMax(0, (int)((elapsed - 1500) / 40)));
    int alpha3 = qMin(255, qMax(0, (int)((elapsed - 3000) / 40)));
    int alpha4 = qMin(255, qMax(0, (int)((elapsed - 4500) / 40)));

    p.setPen(QColor(200, 190, 170, alpha1));
    p.drawText(QRect(0, startY, width(), 40), Qt::AlignCenter, line1);

    p.setPen(QColor(200, 190, 170, alpha2));
    p.drawText(QRect(0, startY + 45, width(), 40), Qt::AlignCenter, line2);

    p.setPen(QColor(200, 190, 170, alpha3));
    p.drawText(QRect(0, startY + 90, width(), 40), Qt::AlignCenter, line3);

    p.setPen(QColor(200, 190, 170, alpha4));
    p.drawText(QRect(0, startY + 135, width(), 40), Qt::AlignCenter, line4);

    // 提示文字（闪烁）
    if (elapsed > 6000) {
        int blinkAlpha = 128 + 127 * std::sin(QDateTime::currentMSecsSinceEpoch() * 0.003);
        p.setPen(QColor(150, 140, 130, blinkAlpha));
        p.setFont(QFont("SimHei", 12));
        p.drawText(QRect(0, height() - 80, width(), 30), Qt::AlignCenter, "按 空格键 继续");
    }
}

void GameWidget::drawTitle(QPainter &p)
{
    // 加载背景图片（懒加载）
    static QPixmap titleBgImg;
    static bool titleBgLoaded = false;
    if (!titleBgLoaded) {
        QString resDir = QCoreApplication::applicationDirPath();
        if (!QFile::exists(resDir + "/title_bg.png")) {
            resDir = "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium";
        }
        titleBgImg.load(resDir + "/title_bg.png");
        titleBgLoaded = true;
    }

    // 绘制背景图片（缩放填满屏幕）
    if (!titleBgImg.isNull()) {
        // 保持比例缩放，填满屏幕
        float imgRatio = (float)titleBgImg.width() / titleBgImg.height();
        float screenRatio = (float)width() / height();
        int drawW, drawH, drawX, drawY;
        if (imgRatio > screenRatio) {
            // 图片更宽，以高度为准
            drawH = height();
            drawW = int(drawH * imgRatio);
            drawX = (width() - drawW) / 2;
            drawY = 0;
        } else {
            // 图片更高，以宽度为准
            drawW = width();
            drawH = int(drawW / imgRatio);
            drawX = 0;
            drawY = (height() - drawH) / 2;
        }
        p.drawPixmap(drawX, drawY, drawW, drawH, titleBgImg);
    } else {
        // 备用：深色背景
        p.fillRect(rect(), QColor(15, 12, 20));
    }

    // 半透明遮罩（让文字更清晰）
    p.fillRect(QRect(0, height() - 150, width(), 150), QColor(0, 0, 0, 120));

    // 标题 - 半透明神秘风格
    float titleAlpha = 180 + 75 * std::sin(QDateTime::currentMSecsSinceEpoch() * 0.001);
    p.setPen(QColor(220, 200, 150, int(titleAlpha)));
    p.setFont(QFont("SimHei", 32, QFont::Bold));
    p.drawText(QRect(0, height() - 130, width(), 40), Qt::AlignCenter, "千年迟赴");

    // 副标题
    p.setPen(QColor(180, 170, 140, 200));
    p.setFont(QFont("SimHei", 14));
    p.drawText(QRect(0, height() - 90, width(), 30), Qt::AlignCenter, "文明灰烬 · 圣女之约");

    // 开始提示 - 闪烁效果
    float blink = (std::sin(QDateTime::currentMSecsSinceEpoch() * 0.004) + 1) * 0.5;
    p.setPen(QColor(200, 200, 200, int(80 + blink * 175)));
    p.setFont(QFont("SimHei", 12));
    p.drawText(QRect(0, height() - 45, width(), 30), Qt::AlignCenter, "按 空格键 开始游戏");
}

void GameWidget::drawGame(QPainter &p)
{
    // 底色填充
    p.fillRect(rect(), scene.bgColor());

    // === 卷轴背景层（视差滚动） ===
    drawScrollingBg(p);

    // 回溯状态：显示回溯背景色，但不绘制角色、NPC、道具、地砖
    if (showingFlashbackBg) {
        // 绘制回溯背景色
        p.fillRect(rect(), flashbackBgColor);
        // 继续执行，让对话框能够绘制
    }

    // 如果是回溯状态，跳过游戏元素绘制
    if (showingFlashbackBg) {
        return;
    }

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
        }
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

    // ===== 场景0：破败教堂特殊绘制 =====
    if (player.sceneId == 0) {
        // 魔法阵图片（懒加载）- 去黑背
        static QPixmap magicCircleImg, magicCircleGlowImg;
        static bool magicCircleLoaded = false;
        if (!magicCircleLoaded) {
            QColor blackBg(0, 0, 0);
            magicCircleImg = removeBackground(QPixmap(resDir + "/prop_magic_circle.png"), blackBg, 30);
            magicCircleGlowImg = removeBackground(QPixmap(resDir + "/prop_magic_circle_glow.png"), blackBg, 30);
            magicCircleLoaded = true;
        }

        // 绘制魔法阵（发现种子后显示发光版本，否则显示普通版本）
        int imgW = 128, imgH = 40;
        if (seedDiscovered && !magicCircleGlowImg.isNull()) {
            p.drawPixmap(450, 580, imgW, imgH, magicCircleGlowImg);
        } else if (!magicCircleImg.isNull()) {
            p.drawPixmap(450, 580, imgW, imgH, magicCircleImg);
        } else {
            // 备用：绘制像素块魔法阵
            p.setBrush(QColor(180, 160, 80));
            p.setPen(QColor(150, 130, 60));
            p.drawEllipse(450, 580, imgW, imgH);
        }

        // 断裂女神像图片（懒加载）- 已是透明背景，直接加载
        static QPixmap churchStatueImg;
        static bool churchStatueLoaded = false;
        if (!churchStatueLoaded) {
            churchStatueImg.load(resDir + "/prop_church_statue.png");
            churchStatueLoaded = true;
        }

        // 绘制断裂女神像
        if (!churchStatueImg.isNull()) {
            int imgW = 100, imgH = 140;
            p.drawPixmap(300, 480, imgW, imgH, churchStatueImg);
        }

        // 烛台图片（懒加载）- 去黑背
        static QPixmap churchCandleImg;
        static bool churchCandleLoaded = false;
        if (!churchCandleLoaded) {
            QColor blackBg(0, 0, 0);
            churchCandleImg = removeBackground(QPixmap(resDir + "/prop_church_candle.jpg"), blackBg, 40);
            churchCandleLoaded = true;
        }

        // 绘制烛台
        if (!churchCandleImg.isNull()) {
            int imgW = 80, imgH = 100;
            p.drawPixmap(150, 520, imgW, imgH, churchCandleImg);
        }
    }

    // 幻梦机器图片（懒加载）- 去黑背
    static QPixmap dreamMachineImg;
    static bool dreamMachineLoaded = false;
    if (!dreamMachineLoaded) {
        QColor blackBg(0, 0, 0);
        dreamMachineImg = removeBackground(QPixmap(resDir + "/prop_dream_machine.png"), blackBg, 30);
        dreamMachineLoaded = true;
    }

    // 全息光屏图片（懒加载）- 去黑背
    static QPixmap holoScreenImg;
    static bool holoScreenLoaded = false;
    if (!holoScreenLoaded) {
        QColor blackBg(0, 0, 0);
        holoScreenImg = removeBackground(QPixmap(resDir + "/prop_holo_screen.png"), blackBg, 30);
        holoScreenLoaded = true;
    }

    // 场景1：绘制幻梦机器图片（固定位置）
    if (player.sceneId == 1 && !dreamMachineImg.isNull()) {
        int imgSize = 200;
        int drawX = 600 - imgSize / 2;  // 固定位置 x=600
        int drawY = 540 - imgSize / 2;  // 固定位置 y=540
        p.drawPixmap(drawX, drawY, imgSize, imgSize, dreamMachineImg);

        // 标签
        p.setPen(QColor(200, 200, 200));
        p.setFont(QFont("SimHei", 10, QFont::Bold));
        p.drawText(drawX, drawY - 10, imgSize, 20, Qt::AlignCenter, "幻梦机器");
    }

    // 场景1：绘制全息光屏图片（固定位置）
    if (player.sceneId == 1 && !holoScreenImg.isNull()) {
        int imgW = 120;
        int imgH = 150;
        int drawX = 900 - imgW / 2;  // 固定位置 x=900
        int drawY = 570 - imgH / 2;  // 固定位置 y=570
        p.drawPixmap(drawX, drawY, imgW, imgH, holoScreenImg);

        // 标签
        p.setPen(QColor(200, 200, 200));
        p.setFont(QFont("SimHei", 10, QFont::Bold));
        p.drawText(drawX, drawY - 10, imgW, 20, Qt::AlignCenter, "全息光屏");
    }

    // 场景2图片道具（壁画、铭文、石棺）- 自动去背景
    static QPixmap wallPaintingImg, stoneTabletImg, stoneCoffinImg;
    static bool scene2ImgsLoaded = false;
    if (!scene2ImgsLoaded) {
        QColor bgColor(0, 0, 0);  // 黑色背景
        QColor whiteBg(255, 255, 255);  // 白色背景
        // 正确加载：壁画去黑背，铭文去黑背，石棺去白背
        wallPaintingImg = removeBackground(QPixmap(resDir + "/prop_wall_painting.png"), bgColor, 30);
        stoneTabletImg = removeBackground(QPixmap(resDir + "/prop_stone_tablet.png"), bgColor, 30);
        stoneCoffinImg = removeBackground(QPixmap(resDir + "/prop_stone_coffin.png"), whiteBg, 30);
        scene2ImgsLoaded = true;
    }

    if (player.sceneId == 2) {
        // 铭文石碑（左侧，解锁后发金色光芒）
        if (!stoneTabletImg.isNull()) {
            int imgW = 80, imgH = 100;
            p.drawPixmap(350, 490, imgW, imgH, stoneTabletImg);
            // 解锁后添加金色发光效果
            if (coffinUnlocked) {
                QColor glow(255, 200, 50, 60);
                p.setPen(Qt::NoPen);
                p.setBrush(glow);
                p.drawEllipse(390, 540, imgW + 30, imgH + 30);
                p.setPen(QColor(255, 220, 80));
            } else {
                p.setPen(QColor(200, 200, 200));
            }
            p.setFont(QFont("SimHei", 9));
            QString tabletLabel = coffinUnlocked ? "铭文石碑（已解读）" : "铭文石碑";
            p.drawText(350, 484, imgW, 16, Qt::AlignCenter, tabletLabel);
        }
        // 壁画（右侧，墙面装饰，大尺寸）
        if (!wallPaintingImg.isNull()) {
            int imgW = 200, imgH = 120;
            p.drawPixmap(550, 490, imgW, imgH, wallPaintingImg);
            p.setPen(QColor(200, 200, 200));
            p.setFont(QFont("SimHei", 9));
            p.drawText(550, 484, imgW, 16, Qt::AlignCenter, "繁华壁画");
        }
        // 石棺（解锁后显示蓝色光芒）
        if (!stoneCoffinImg.isNull()) {
            int imgW = 120, imgH = 80;
            p.drawPixmap(830, 540, imgW, imgH, stoneCoffinImg);
            if (coffinUnlocked) {
                // 解锁后蓝色光芒
                QColor glow(80, 150, 255, 50);
                p.setPen(Qt::NoPen);
                p.setBrush(glow);
                p.drawEllipse(890, 580, imgW + 40, imgH + 40);
                p.setPen(QColor(120, 180, 255));
            } else {
                // 未解锁：金色封印
                p.setPen(QColor(200, 180, 100));
                p.setBrush(QColor(200, 180, 100, 30));
                p.drawRect(830, 540, imgW, imgH);
                p.setPen(QColor(200, 200, 200));
            }
            p.setFont(QFont("SimHei", 9));
            QString coffinLabel = coffinUnlocked ? "国王石棺（已解锁）" : "国王石棺（封印中）";
            p.drawText(830, 534, imgW, 16, Qt::AlignCenter, coffinLabel);
        }
    }

    // 场景4手稿图片绘制 - 去白背
    static QPixmap manuscriptImg;
    static bool manuscriptImgLoaded = false;
    if (!manuscriptImgLoaded) {
        QColor whiteBg(255, 255, 255);
        manuscriptImg = removeBackground(QPixmap(resDir + "/prop_manuscript.png"), whiteBg, 30);
        manuscriptImgLoaded = true;
    }

    // 场景4（图书馆）图片绘制 - 去白背
    static QPixmap bookshelfImg;
    static QPixmap seedVesselImg;
    static QPixmap seedImg;
    static QPixmap portalDoorImg;
    static bool scene4ImgsLoaded = false;
    if (!scene4ImgsLoaded) {
        QColor whiteBg(255, 255, 255);
        QColor blackBg(0, 0, 0);
        bookshelfImg = removeBackground(QPixmap(resDir + "/prop_bookshelf.png"), whiteBg, 30);
        seedVesselImg = removeBackground(QPixmap(resDir + "/prop_seed_vessel.png"), whiteBg, 30);
        seedImg = removeBackground(QPixmap(resDir + "/prop_seed.png"), whiteBg, 30);
        portalDoorImg = removeBackground(QPixmap(resDir + "/prop_portal_door.png"), blackBg, 30);
        scene4ImgsLoaded = true;
    }

    if (player.sceneId == 4) {
        // 书架（装饰）
        if (!bookshelfImg.isNull()) {
            int imgW = 150, imgH = 140;
            p.drawPixmap(300, 480, imgW, imgH, bookshelfImg);
        }

        // 魔法书（地面）
        if (!manuscriptImg.isNull()) {
            int imgW = 60, imgH = 40;
            p.drawPixmap(500, 580, imgW, imgH, manuscriptImg);
            p.setPen(QColor(200, 200, 200));
            p.setFont(QFont("SimHei", 9));
            p.drawText(500, 574, imgW, 16, Qt::AlignCenter, "残缺手稿");
            // 发光提示
            float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
            p.setBrush(QColor(255, 220, 50));
            p.setPen(QColor(200, 170, 0));
            p.drawRect(555, 565 + bob, 14, 14);
            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(QRect(555, 565 + bob, 14, 14), Qt::AlignCenter, "!");
        }

        // 种子容器（地面）
        if (!seedDiscovered) {
            if (!seedVesselImg.isNull()) {
                int imgW = 60, imgH = 50;
                p.drawPixmap(700, 570, imgW, imgH, seedVesselImg);
                // 交互提示
                float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
                p.setBrush(QColor(255, 220, 50));
                p.setPen(QColor(200, 170, 0));
                p.drawRect(725, 555 + bob, 14, 14);
                p.setPen(Qt::white);
                p.setFont(QFont("Arial", 10, QFont::Bold));
                p.drawText(QRect(725, 555 + bob, 14, 14), Qt::AlignCenter, "!");
            }
        } else {
            // 种子（发现后，地面）
            if (!seedImg.isNull()) {
                int imgW = 60, imgH = 50;
                p.drawPixmap(700, 570, imgW, imgH, seedImg);
                p.setPen(QColor(100, 220, 120));
                p.setFont(QFont("SimHei", 9));
                p.drawText(700, 564, imgW, 16, Qt::AlignCenter, "文明种子");
                // 发光提示
                float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
                p.setBrush(QColor(255, 220, 50));
                p.setPen(QColor(200, 170, 0));
                p.drawRect(725, 555 + bob, 14, 14);
                p.setPen(Qt::white);
                p.setFont(QFont("Arial", 10, QFont::Bold));
                p.drawText(QRect(725, 555 + bob, 14, 14), Qt::AlignCenter, "!");
            }
        }

        // 传送门
        if (!portalDoorImg.isNull()) {
            int imgW = 100, imgH = 200;
            p.drawPixmap(1050, 420, imgW, imgH, portalDoorImg);
            p.setPen(QColor(150, 150, 200));
            p.setFont(QFont("SimHei", 9));
            p.drawText(1050, 414, imgW, 16, Qt::AlignCenter, "传送门");
        }
    }

    // 场景3（火山）断剑图片绘制 - 已是透明背景，回溯结束后显示
    static QPixmap brokenSwordImg;
    static bool brokenSwordImgLoaded = false;
    if (!brokenSwordImgLoaded) {
        brokenSwordImg.load(resDir + "/prop_broken_sword.png");
        brokenSwordImgLoaded = true;
    }
    if (player.sceneId == 3 && swordDropped && !brokenSwordImg.isNull()) {
        int imgW = 60, imgH = 120;
        int drawX = 830, drawY = 500;
        p.drawPixmap(drawX, drawY, imgW, imgH, brokenSwordImg);
        p.setPen(QColor(200, 200, 200));
        p.setFont(QFont("SimHei", 9, QFont::Bold));
        QString swordLabel = swordPicked ? "传送至图书馆" : "火山断剑";
        p.drawText(drawX, drawY - 8, imgW, 20, Qt::AlignCenter, swordLabel);
        // 发光效果
        float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
        p.setBrush(QColor(255, 220, 50));
        p.setPen(QColor(200, 170, 0));
        p.drawRect(drawX + imgW - 2, drawY - 18 + bob, 14, 14);
        p.setPen(Qt::white);
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.drawText(QRect(drawX + imgW - 2, drawY - 18 + bob, 14, 14), Qt::AlignCenter, "!");
    }

    for (auto &prop : scene.props()) {
        if (prop.picked()) continue;
        if (prop.label().isEmpty()) continue; // 装饰性道具

        // 跳过已有图片绘制的道具（避免重复绘制像素块）
        QString pid = prop.id();
        if (pid == "magic_circle" || pid == "holo_screen" ||
            pid == "wall_painting" || pid == "stone_tablet" || pid == "stone_coffin" ||
            pid == "manuscript" || pid == "seed_container" || pid == "seed" ||
            pid == "portal_door" || pid == "broken_sword" ||
            pid == "bookshelf1" || pid == "bookshelf2" || pid == "time_corridor" ||
            pid == "old_door" || pid == "dream_machine" || pid == "ember_ground" ||
            pid == "lava_rock1" || pid == "lava_rock2" ||
            pid == "bookshelf" || pid == "manuscript_hidden" || pid == "seed_vessel" ||
            pid == "pyramid" || pid == "sand_drift") {
            continue;
        }

        // 其他道具不绘制像素块（所有道具都使用图片）
        continue;
    }
}

void GameWidget::drawNpcs(QPainter &p)
{
    // 资源目录（懒加载）
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

    // 古井图片（懒加载）- 去白背
    static QPixmap wellBlockedImg;
    static QPixmap wellOpenImg;
    static bool wellImgLoaded = false;
    if (!wellImgLoaded) {
        QColor whiteBg(255, 255, 255);
        wellBlockedImg = removeBackground(QPixmap(resDir + "/prop_well_blocked.png"), whiteBg, 30);
        wellOpenImg = removeBackground(QPixmap(resDir + "/prop_well_open.png"), whiteBg, 30);
        wellImgLoaded = true;
    }

    // 龙虾图片（懒加载，自动去白色背景）
    static QPixmap lobsterTrappedImg;
    static QPixmap lobsterSavedImg;
    static bool lobsterImgLoaded = false;
    if (!lobsterImgLoaded) {
        QColor bgColor(255, 255, 255);  // 白色背景
        lobsterTrappedImg = removeBackground(QPixmap(resDir + "/npc_lobster_trapped.png"), bgColor, 60);
        lobsterSavedImg = removeBackground(QPixmap(resDir + "/npc_lobster_saved.png"), bgColor, 60);
        lobsterImgLoaded = true;
    }

    for (auto &npc : scene.npcs()) {
        QRect r = npc.rect();
        QString id = npc.id();

        // 古井：绘制图片
        if (id == "well") {
            QPixmap &img = wellCleared ? wellOpenImg : wellBlockedImg;
            if (!img.isNull()) {
                int imgSize = 120;
                int drawX = r.center().x() - imgSize / 2;
                int drawY = r.center().y() - imgSize / 2 + 20;
                p.drawPixmap(drawX, drawY, imgSize, imgSize, img);
                QString label = wellCleared ? "疏通的古井（按E进入）" : "被堵住的古井（按E疏通）";
                p.setPen(QColor(200, 200, 200));
                p.setFont(QFont("SimHei", 9, QFont::Bold));
                p.drawText(drawX, drawY - 8, imgSize, 20, Qt::AlignCenter, label);
            }
            // 始终显示 "!" 提示
            float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
            p.setBrush(QColor(255, 220, 50));
            p.setPen(QColor(200, 170, 0));
            p.drawRect(r.right() - 2, r.top() - 18 + bob, 14, 14);
            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(QRect(r.right() - 2, r.top() - 18 + bob, 14, 14), Qt::AlignCenter, "!");
            continue;
        }

        // 龙虾：绘制图片（被困/已释放两种状态）
        if (id == "lobster") {
            QPixmap &img = lobsterFreed ? lobsterSavedImg : lobsterTrappedImg;
            if (!img.isNull()) {
                int drawH = 100;
                int drawW = int(drawH * (float)img.width() / img.height());
                int drawX = r.center().x() - drawW / 2;
                int drawY = r.bottom() - drawH; // 脚部对齐
                p.drawPixmap(drawX, drawY, drawW, drawH, img);
                QString label = lobsterFreed ? "灵智龙虾" : "被困的龙虾（按E释放）";
                p.setPen(QColor(200, 200, 200));
                p.setFont(QFont("SimHei", 9, QFont::Bold));
                p.drawText(drawX, drawY - 8, drawW, 20, Qt::AlignCenter, label);
            }
            // 未救时显示 "!" 提示
            if (!lobsterSaved) {
                float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
                p.setBrush(QColor(255, 220, 50));
                p.setPen(QColor(200, 170, 0));
                p.drawRect(r.right() - 2, r.top() - 18 + bob, 14, 14);
                p.setPen(Qt::white);
                p.setFont(QFont("Arial", 10, QFont::Bold));
                p.drawText(QRect(r.right() - 2, r.top() - 18 + bob, 14, 14), Qt::AlignCenter, "!");
            }
            continue;
        }

        // 石门：绘制图片
        if (id == "stone_door") {
            // 从场景六回来后，使用传送门图片
            if (returnedFromScene6) {
                static QPixmap portalDoorImg;
                if (portalDoorImg.isNull()) {
                    portalDoorImg.load(resDir + "/prop_portal_door.png");
                }
                if (!portalDoorImg.isNull()) {
                    int drawH = 140;
                    int drawW = int(drawH * (float)portalDoorImg.width() / portalDoorImg.height());
                    int drawX = r.center().x() - drawW / 2;
                    int drawY = r.top();
                    p.drawPixmap(drawX, drawY, drawW, drawH, portalDoorImg);
                    p.setPen(QColor(180, 220, 255));
                    p.setFont(QFont("SimHei", 9));
                    p.drawText(drawX, drawY - 8, drawW, 20, Qt::AlignCenter, "时空之门");
                }
                continue;
            }

            // 石门图片 - 去白背
            static QPixmap doorImg;
            static bool doorImgLoaded = false;
            if (!doorImgLoaded) {
                QColor whiteBg(255, 255, 255);
                doorImg = removeBackground(QPixmap(resDir + "/npc_stone_door.png"), whiteBg, 30);
                doorImgLoaded = true;
            }
            if (!doorImg.isNull()) {
                // 按比例缩放：高度140，保持宽高比
                int drawH = 140;
                int drawW = int(drawH * (float)doorImg.width() / doorImg.height());
                int drawX = r.center().x() - drawW / 2;
                int drawY = r.top();
                p.drawPixmap(drawX, drawY, drawW, drawH, doorImg);
                p.setPen(QColor(220, 220, 200));
                p.setFont(QFont("SimHei", 9));
                p.drawText(drawX, drawY - 8, drawW, 20, Qt::AlignCenter, npc.label());
            }
            // 始终显示 "!" 提示
            float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
            p.setBrush(QColor(255, 220, 50));
            p.setPen(QColor(200, 170, 0));
            p.drawRect(r.right() - 2, r.top() - 18 + bob, 14, 14);
            p.setPen(Qt::white);
            p.setFont(QFont("Arial", 10, QFont::Bold));
            p.drawText(QRect(r.right() - 2, r.top() - 18 + bob, 14, 14), Qt::AlignCenter, "!");
            continue;
        }

        // 守剑幽魂：绘制图片（场景3）- 去白背
        if (id == "sword_ghost") {
            // 断剑掉落后幽魂消失
            if (swordDropped || swordPicked) {
                continue;
            }
            static QPixmap ghostImg;
            static bool ghostImgLoaded = false;
            if (!ghostImgLoaded) {
                QColor whiteBg(255, 255, 255);
                ghostImg = removeBackground(QPixmap(resDir + "/npc_sword_ghost.png"), whiteBg, 30);
                ghostImgLoaded = true;
            }
            if (!ghostImg.isNull()) {
                int drawH = 140;
                int drawW = int(drawH * (float)ghostImg.width() / ghostImg.height());
                int drawX = r.center().x() - drawW / 2;
                int drawY = r.bottom() - drawH;
                p.drawPixmap(drawX, drawY, drawW, drawH, ghostImg);
                p.setPen(QColor(200, 200, 200));
                p.setFont(QFont("SimHei", 9, QFont::Bold));
                p.drawText(drawX, drawY - 8, drawW, 20, Qt::AlignCenter, "守剑幽魂");
                // 显示 "!" 提示
                float bob = std::sin(QDateTime::currentMSecsSinceEpoch() * 0.005) * 3;
                p.setBrush(QColor(255, 220, 50));
                p.setPen(QColor(200, 170, 0));
                p.drawRect(r.right() - 2, r.top() - 18 + bob, 14, 14);
                p.setPen(Qt::white);
                p.setFont(QFont("Arial", 10, QFont::Bold));
                p.drawText(QRect(r.right() - 2, r.top() - 18 + bob, 14, 14), Qt::AlignCenter, "!");
            }
            continue;
        }

        // 所有NPC都有图片，不绘制像素块
        continue;

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

    // 检查是否需要显示立绘
    // 守剑幽魂相关对话 - 右侧立绘
    bool showGhostPortrait = (currentDialog.speaker == "守剑幽魂" ||
                               currentDialogKey.startsWith("volcano_ghost") ||
                               currentDialogKey.startsWith("volcano_sword"));
    // 绿洲国王相关对话 - 左侧立绘
    bool showKingPortrait = (currentDialog.speaker == "国王" ||
                              currentDialogKey.startsWith("pyramid_king"));
    // 勇者相关对话 - 左侧立绘
    bool showHeroPortrait = (currentDialog.speaker == "勇者");

    // 幽灵立绘图片（懒加载）- 暂时不去背，直接加载
    static QPixmap ghostPortraitImg;
    static bool ghostPortraitLoaded = false;
    if (!ghostPortraitLoaded) {
        QString resDir = QCoreApplication::applicationDirPath();
        if (!QFile::exists(resDir + "/portrait_ghost.png")) {
            resDir = "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium";
        }
        ghostPortraitImg.load(resDir + "/portrait_ghost.png");
        ghostPortraitLoaded = true;
    }

    // 国王立绘图片（懒加载）- 暂时不去背，直接加载
    static QPixmap kingPortraitImg;
    static bool kingPortraitLoaded = false;
    if (!kingPortraitLoaded) {
        QString resDir = QCoreApplication::applicationDirPath();
        if (!QFile::exists(resDir + "/portrait_king.jpg")) {
            resDir = "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium";
        }
        kingPortraitImg.load(resDir + "/portrait_king.jpg");
        kingPortraitLoaded = true;
    }

    // 勇者立绘图片（懒加载）- 暂时不去背，直接加载
    static QPixmap heroPortraitImg;
    static bool heroPortraitLoaded = false;
    if (!heroPortraitLoaded) {
        QString resDir = QCoreApplication::applicationDirPath();
        if (!QFile::exists(resDir + "/portrait_hero.png")) {
            resDir = "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium";
        }
        heroPortraitImg.load(resDir + "/portrait_hero.png");
        heroPortraitLoaded = true;
    }

    // 如果显示立绘，调整对话框位置和宽度
    int boxX = 60, boxY = height() - 340, boxW = width() - 120, boxH = 300;
    if (showGhostPortrait && !ghostPortraitImg.isNull()) {
        boxW = width() - 320;  // 缩小对话框宽度，为右侧立绘留出空间
    }
    if ((showKingPortrait && !kingPortraitImg.isNull()) ||
        (showHeroPortrait && !heroPortraitImg.isNull())) {
        boxX = 260;  // 向右移动对话框，为左侧立绘留出空间
        boxW = width() - 320;  // 缩小对话框宽度
    }

    // 先绘制立绘（在对话框背景之前）
    if (showHeroPortrait && !heroPortraitImg.isNull()) {
        int portraitH = 280;
        int portraitW = int(portraitH * (float)heroPortraitImg.width() / heroPortraitImg.height());
        if (portraitW > 200) portraitW = 200;
        int portraitX = 30;
        int portraitY = height() - portraitH - 40;
        p.drawPixmap(portraitX, portraitY, portraitW, portraitH, heroPortraitImg);
    }
    if (showKingPortrait && !kingPortraitImg.isNull()) {
        int portraitH = 280;
        int portraitW = int(portraitH * (float)kingPortraitImg.width() / kingPortraitImg.height());
        if (portraitW > 200) portraitW = 200;
        int portraitX = 30;
        int portraitY = height() - portraitH - 40;
        p.drawPixmap(portraitX, portraitY, portraitW, portraitH, kingPortraitImg);
    }
    if (showGhostPortrait && !ghostPortraitImg.isNull()) {
        int portraitH = 280;
        int portraitW = int(portraitH * (float)ghostPortraitImg.width() / ghostPortraitImg.height());
        if (portraitW > 200) portraitW = 200;
        int portraitX = width() - portraitW - 30;
        int portraitY = height() - portraitH - 40;
        p.drawPixmap(portraitX, portraitY, portraitW, portraitH, ghostPortraitImg);
    }

    // 对话框背景
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

    // 对话文本（增大区域高度）
    p.setPen(QColor(220, 220, 220));
    p.setFont(QFont("SimHei", 12));
    p.drawText(QRect(boxX + 20, boxY + 40, boxW - 40, 200),
               Qt::AlignLeft | Qt::TextWordWrap, currentDialog.text);

    // 选项
    if (state == GameState::Choice && !currentDialog.choices.isEmpty()) {
        int y = boxY + 245;  // 调整选项起始位置
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

    // 获取资源目录
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

    // 加载所有结局背景图片
    static QPixmap endingSeedBg, endingDreamBg, endingCoffinBg, endingTimeBg, endingReturnBg;
    static bool endingBgsLoaded = false;
    if (!endingBgsLoaded) {
        endingSeedBg.load(resDir + "/ending_seed.png");
        endingDreamBg.load(resDir + "/ending_dream.png");
        endingCoffinBg.load(resDir + "/ending_coffin.png");
        endingTimeBg.load(resDir + "/ending_time.png");
        endingReturnBg.load(resDir + "/ending_return.png");
        endingBgsLoaded = true;
    }

    // 根据结局类型选择背景图
    QPixmap* bgImg = nullptr;
    if (endingType == 1) bgImg = &endingSeedBg;      // 文明新生
    else if (endingType == 2) bgImg = &endingReturnBg; // 归世幻梦（回到原来世界）
    else if (endingType == 3) bgImg = &endingTimeBg;   // 逆时赴约（研究手稿）
    else if (endingType == 4) bgImg = &endingDreamBg;  // 幻梦沉沦（启动幻梦机器）
    else if (endingType == 5) bgImg = &endingCoffinBg; // 枯骨永眠（石棺）

    if (bgImg && !bgImg->isNull()) {
        // 绘制背景图片（居中缩放）
        p.fillRect(rect(), QColor(0, 0, 0));
        float imgAspect = (float)bgImg->width() / bgImg->height();
        float winAspect = (float)width() / height();
        int drawW, drawH, drawX, drawY;
        if (imgAspect > winAspect) {
            drawW = width();
            drawH = (int)(width() / imgAspect);
        } else {
            drawH = height();
            drawW = (int)(height() * imgAspect);
        }
        drawX = (width() - drawW) / 2;
        drawY = (height() - drawH) / 2;

        // 渐入效果
        QPixmap scaled = bgImg->scaled(drawW, drawH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmap faded(scaled.size());
        faded.fill(Qt::transparent);
        QPainter pf(&faded);
        pf.setOpacity(fadeIn);
        pf.drawPixmap(0, 0, scaled);
        pf.end();
        p.drawPixmap(drawX, drawY, faded);

        // 半透明暗色叠加（让文字更清晰）
        if (fadeIn > 0.5f) {
            p.fillRect(rect(), QColor(0, 0, 0, int((fadeIn - 0.5f) * 80)));
        }
    } else {
        // 其他结局使用纯色背景
        QColor bg = ed.bgColor;
        bg.setAlpha(int(fadeIn * 255));
        p.fillRect(rect(), bg);
    }

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

    // 结局描述（增大区域以显示完整文字）
    p.setPen(QColor(ed.textColor.red(), ed.textColor.green(), ed.textColor.blue(), 200));
    p.setFont(QFont("SimHei", 14));
    p.drawText(QRect(width() / 2 - 300, height() / 2 - 30, 600, 200),
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

    // 提示文字（直接显示在左上角）
    QString displayHint = hintText.isEmpty() ? scene.hint() : hintText;
    if (!displayHint.isEmpty()) {
        p.setPen(QColor(140, 140, 130));
        p.setFont(QFont("SimHei", 9));
        // 处理换行显示
        QStringList lines = displayHint.split('\n');
        int y = 45;
        for (const QString &line : lines) {
            if (y > 100) break; // 最多显示3行
            p.drawText(10, y, line);
            y += 18;
        }
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

        // 绘制3个按键提示：A/D 移动 | 空格 跳跃 | E 互动（回溯改为与物品交互）
        struct KeyHint {
            QString key;
            QString label;
            QColor keyColor;
        };
        QList<KeyHint> hints = {
            {"A/D", "移动", QColor(180, 170, 140, alpha)},
            {"空格", "跳跃", QColor(180, 170, 140, alpha)},
            {"E", "互动", QColor(180, 170, 140, alpha)}
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

// ==================== 像素地面绘制 ====================

void GameWidget::drawPixelGround(QPainter &p)
{
    int sceneId = player.sceneId;
    
    // 资源目录（懒加载）
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
    
    // 地砖图片（懒加载）- 每次检查确保加载成功
    static QPixmap tileChurch, tileDungeon, tilePyramid, tileVolcano;
    static bool tilesLoaded = false;
    if (!tilesLoaded) {
        tileChurch.load(resDir + "/tile_church.png");
        tileDungeon.load(resDir + "/tile_dungeon.png");
        tilePyramid.load(resDir + "/tile_pyramid.png");
        tileVolcano.load(resDir + "/tile_volcano.png");
        tilesLoaded = true;
    }
    
    // 根据场景ID选择地砖
    QPixmap *tile = nullptr;
    switch (sceneId) {
        case 0: tile = &tileChurch; break;    // 破败教堂
        case 1: tile = &tileDungeon; break;   // 赛博朋克地下城
        case 2: tile = &tilePyramid; break;   // 沙漠金字塔
        case 3: tile = &tileVolcano; break;   // 末日火山
        case 4: tile = &tileChurch; break;    // 图书馆秘境（复用教堂地砖）
        case 5: tile = &tileChurch; break;    // 时空终局教堂（复用教堂地砖）
        default: tile = &tileChurch; break;
    }
    
    // 主地面区域
    QRect mainGround(0, 620, 1280, 100);
    
    // 用地砖平铺地面（固定64x64紧密平铺）
    if (tile && !tile->isNull()) {
        const int TILE_SIZE = 64;  // 固定地砖尺寸，紧密平铺
        
        // 从地面顶部开始平铺
        for (int y = mainGround.top(); y < mainGround.bottom(); y += TILE_SIZE) {
            for (int x = 0; x < mainGround.right(); x += TILE_SIZE) {
                // 计算实际绘制区域（裁剪到地面边界内）
                int drawW = qMin(TILE_SIZE, mainGround.right() - x);
                int drawH = qMin(TILE_SIZE, mainGround.bottom() - y);
                
                if (drawW > 0 && drawH > 0) {
                    // 缩放图片到固定尺寸，实现紧密平铺
                    p.drawPixmap(x, y, drawW, drawH, *tile);
                }
            }
        }
    } else {
        // 备用：绘制纯色地面
        QColor groundColor;
        switch (sceneId) {
            case 0: groundColor = QColor(60, 55, 50); break;
            case 1: groundColor = QColor(40, 42, 55); break;
            case 2: groundColor = QColor(140, 120, 70); break;
            case 3: groundColor = QColor(50, 35, 30); break;
            case 4: groundColor = QColor(70, 65, 60); break;
            case 5: groundColor = QColor(60, 55, 50); break;
            default: groundColor = QColor(60, 55, 50); break;
        }
        p.fillRect(mainGround, groundColor);
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
