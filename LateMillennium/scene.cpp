#include "scene.h"

Scene::Scene() {}

void Scene::load(int sceneId)
{
    m_platforms.clear();
    m_props.clear();
    m_npcs.clear();
    m_exits.clear();
    m_hint.clear();
    m_flashbackTrigger.clear();

    if (sceneId == 0) {
        // ===== 场景一：破败教堂 =====
        m_name = "破败教堂";
        m_bgColor = QColor(30, 28, 35);       // 灰败冷色
        m_groundColor = QColor(60, 55, 50);   // 碎石灰
        m_flashbackColor = QColor(80, 70, 50); // 暖色调教堂

        // 平台：仅地面（无障碍物）
        m_platforms << QRect(0, 620, 1280, 100);   // 主地面

        // 道具 - 场景一可交互物品
        m_props << Prop("magic_circle", 450, 580, 128, 40, QColor(180, 160, 80), "古老法阵");
        m_props << Prop("church_statue", 300, 480, 80, 140, QColor(140, 130, 120), "断裂的神像");
        m_props << Prop("church_candle", 150, 520, 40, 100, QColor(180, 160, 100), "熄灭的烛台");
        m_props << Prop("church_flashback_item", 200, 550, 60, 70, QColor(120, 110, 90), "残破的圣像"); // 回溯触发物品

        // NPC
        m_npcs << Npc("well", 850, 500, 100, 70, QColor(60, 55, 50), "被堵住的古井"); // 门左侧
        m_npcs << Npc("stone_door", 1150, 480, 64, 140, QColor(80, 75, 85), "闭锁石门");

        // 出口：通过石门道具交互进入图书馆，不在此处定义出口

        // 回溯触发
        m_flashbackTrigger = "church";
        m_hint = "断裂的神像、熄灭的烛台……\n这里曾经发生过什么？";
    }
    else if (sceneId == 1) {
        // ===== 场景二：赛博朋克地下城 =====
        m_name = "赛博朋克地下城";
        m_bgColor = QColor(15, 18, 35);       // 冷蓝暗紫
        m_groundColor = QColor(40, 42, 55);   // 金属地面
        m_flashbackColor = QColor(50, 40, 60); // 暖色繁华

        m_platforms << QRect(0, 620, 1280, 100);

        // 道具（全部放在地面上）
        m_props << Prop("dungeon_flashback_item", 600, 580, 50, 40, QColor(80, 70, 90), "废弃的终端"); // 回溯触发物品
        // 幻梦机器、全息光屏使用图片绘制，不在此处定义

        // NPC（放在地面上）
        m_npcs << Npc("lobster", 220, 590, 32, 30, QColor(200, 60, 40), "灵智龙虾");

        // 出口
        m_exits << SceneExit{QRect(1220, 560, 60, 60), 2, 100, 540, "沙漠通道"}; // 右侧→沙漠金字塔

        m_flashbackTrigger = "dungeon";
        m_hint = "地下城弥漫着诡异的蓝光……\n一只龙虾被困在废墟中……";
    }
    else if (sceneId == 2) {
        // ===== 场景三：沙漠金字塔 =====
        m_name = "沙漠金字塔";
        m_bgColor = QColor(60, 50, 30);       // 土黄
        m_groundColor = QColor(140, 120, 70); // 沙地
        m_flashbackColor = QColor(60, 80, 50); // 绿洲暖色

        m_platforms << QRect(0, 620, 1280, 100);

        // 道具（全部放在地面上）
        m_props << Prop("pyramid", 400, 380, 200, 240, QColor(160, 140, 90), "金字塔"); // 装饰性，底部在地面
        m_props << Prop("pyramid_flashback_item", 150, 580, 60, 40, QColor(140, 120, 70), "古老的陶罐"); // 回溯触发物品
        // 壁画、铭文、石棺使用图片绘制，不在此处定义
        m_props << Prop("sand_drift", 100, 600, 1280, 20, QColor(160, 140, 80), ""); // 沙地装饰

        // 出口
        m_exits << SceneExit{QRect(1150, 580, 60, 40), 3, 100, 300, ""}; // 塔外出口→火山

        m_flashbackTrigger = "pyramid";
        m_hint = "沙漠中矗立着古老的金字塔……\n石棺中似乎藏着永生的秘密……";
    }
    else if (sceneId == 3) {
        // ===== 场景四：末日火山 =====
        m_name = "末日火山";
        m_bgColor = QColor(35, 20, 15);       // 黑灰暗红
        m_groundColor = QColor(50, 35, 30);   // 焦黑
        m_flashbackColor = QColor(40, 60, 30); // 茂林暖色

        m_platforms << QRect(0, 620, 1280, 100);

        // 道具
        m_props << Prop("lava_rock1", 200, 595, 32, 25, QColor(60, 40, 35), "");
        m_props << Prop("lava_rock2", 500, 590, 40, 30, QColor(55, 38, 32), "");
        m_props << Prop("ember_ground", 0, 615, 1280, 5, QColor(80, 30, 20), ""); // 灰烬
        m_props << Prop("broken_sword", 820, 590, 40, 30, QColor(180, 180, 200), "断剑"); // 断剑（初始隐藏）
        m_props << Prop("volcano_flashback_item", 350, 580, 50, 40, QColor(100, 80, 70), "烧焦的日记"); // 回溯触发物品

        // NPC
        m_npcs << Npc("sword_ghost", 820, 560, 24, 60, QColor(150, 150, 160, 100), "守剑幽魂");

        // 无自动出口（断剑通过幽魂交互获取）

        m_flashbackTrigger = "volcano";
        m_hint = "火山口散发着灼热的气息……\n断剑的幽魂在此守候千年……";
    }
    else if (sceneId == 4) {
        // ===== 场景五：图书馆秘境 =====
        m_name = "图书馆秘境";
        m_bgColor = QColor(40, 35, 50);       // 淡紫米白
        m_groundColor = QColor(70, 65, 60);   // 朽木地板
        m_flashbackColor = QColor(60, 55, 40); // 暖色图书馆

        m_platforms << QRect(0, 620, 1280, 100);

        // 道具（魔法书和种子放在地面上）
        m_props << Prop("time_corridor", 50, 300, 80, 320, QColor(180, 180, 200, 80), "时空通道");
        m_props << Prop("bookshelf", 300, 480, 150, 140, QColor(80, 60, 40), "倒塌书架"); // 书架（装饰）
        m_props << Prop("manuscript_hidden", 500, 580, 60, 40, QColor(200, 190, 160), ""); // 魔法书（地面）
        m_props << Prop("seed_vessel", 700, 570, 60, 50, QColor(200, 180, 100), ""); // 种子容器（地面）
        m_props << Prop("portal_door", 1050, 420, 100, 200, QColor(100, 100, 150), "传送门");
        m_props << Prop("library_flashback_item", 900, 580, 60, 40, QColor(120, 110, 100), "散落的笔记"); // 回溯触发物品

        // 出口：传送门回到场景1
        m_exits << SceneExit{QRect(1050, 420, 100, 200), 0, 200, 300, ""}; // 传送门→教堂（出现在门旁边）

        m_flashbackTrigger = "library";
        m_hint = "图书馆秘境中藏着古老的智慧……\n书架上似乎藏着什么……";
    }
    else if (sceneId == 5) {
        // ===== 场景六：时空闭环（终局教堂） =====
        m_name = "时空闭环";
        m_bgColor = QColor(30, 28, 35);       // 与破败教堂一致
        m_groundColor = QColor(60, 55, 50);
        m_flashbackColor = QColor(80, 70, 50);

        m_platforms << QRect(0, 620, 1280, 100);

        // 只有法阵（用于触发最终选择）
        m_props << Prop("magic_circle", 450, 580, 128, 40, QColor(180, 160, 80), "古老法阵");

        // 无出口，只有结局选择
        m_hint = "一切回到了起点……\n种子承载三种力量，你的选择，便是世界的终局。";
        m_flashbackTrigger = "";  // 终局无回溯
    }
}
