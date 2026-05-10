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

        // 平台：地面 + 碎石堆
        m_platforms << QRect(0, 620, 1280, 100);   // 主地面
        m_platforms << QRect(80, 560, 80, 16);     // 左侧碎石堆
        m_platforms << QRect(300, 540, 120, 16);   // 中间台阶
        m_platforms << QRect(500, 560, 100, 16);   // 通往石门的平台1
        m_platforms << QRect(650, 540, 100, 16);   // 通往石门的平台2
        m_platforms << QRect(800, 520, 100, 16);   // 通往石门的平台3
        m_platforms << QRect(950, 500, 120, 16);   // 石门前平台

        // 道具
        m_props << Prop("magic_circle", 450, 580, 128, 40, QColor(180, 160, 80), "古老法阵");
        m_props << Prop("rubble_corner", 80, 530, 60, 30, QColor(90, 85, 75), "石堆");

        // NPC
        m_npcs << Npc("stone_door", 1050, 480, 64, 140, QColor(80, 75, 85), "闭锁石门");
        m_npcs << Npc("well", 1150, 500, 100, 70, QColor(60, 55, 50), "被堵住的古井");

        // 出口：通过石门道具交互进入图书馆，不在此处定义出口

        // 回溯触发
        m_flashbackTrigger = "magic_circle";
        m_hint = "探索废墟，寻找线索";
    }
    else if (sceneId == 1) {
        // ===== 场景二：赛博朋克地下城 =====
        m_name = "赛博朋克地下城";
        m_bgColor = QColor(15, 18, 35);       // 冷蓝暗紫
        m_groundColor = QColor(40, 42, 55);   // 金属地面
        m_flashbackColor = QColor(50, 40, 60); // 暖色繁华

        m_platforms << QRect(0, 620, 1280, 100);
        m_platforms << QRect(200, 540, 100, 16);  // 管道平台
        m_platforms << QRect(500, 500, 120, 16);  // 高台
        m_platforms << QRect(800, 560, 80, 16);   // 小平台

        // 道具
        m_props << Prop("dream_machine", 600, 420, 80, 80, QColor(60, 70, 90), "幻梦机器"); // 居中，大碰撞框便于交互
        m_props << Prop("neon_pipe_red", 150, 300, 16, 320, QColor(200, 50, 50), "霓虹管道");
        m_props << Prop("neon_pipe_blue", 400, 350, 16, 270, QColor(50, 80, 200), "霓虹管道");
        m_props << Prop("mech_debris1", 250, 590, 32, 30, QColor(100, 90, 70), "机械残骸");
        m_props << Prop("mech_debris2", 700, 595, 48, 25, QColor(90, 85, 75), "机械残骸");
        m_props << Prop("holo_screen", 900, 350, 64, 48, QColor(40, 60, 80), "全息光屏");

        // NPC
        m_npcs << Npc("lobster", 220, 510, 32, 30, QColor(200, 60, 40), "灵智龙虾");

        // 出口
        m_exits << SceneExit{QRect(1220, 560, 60, 60), 2, 100, 540, "沙漠通道"}; // 右侧→沙漠金字塔

        m_flashbackTrigger = "lobster";
        m_hint = "小心幻梦机器";
    }
    else if (sceneId == 2) {
        // ===== 场景三：沙漠金字塔 =====
        m_name = "沙漠金字塔";
        m_bgColor = QColor(60, 50, 30);       // 土黄
        m_groundColor = QColor(140, 120, 70); // 沙地
        m_flashbackColor = QColor(60, 80, 50); // 绿洲暖色

        m_platforms << QRect(0, 620, 1280, 100);
        m_platforms << QRect(350, 540, 200, 16);  // 金字塔入口台阶
        m_platforms << QRect(600, 480, 160, 16);  // 内部平台
        m_platforms << QRect(850, 520, 120, 16);  // 石棺台

        // 道具
        m_props << Prop("pyramid", 400, 380, 200, 240, QColor(160, 140, 90), "金字塔");
        m_props << Prop("wall_painting", 420, 400, 80, 60, QColor(140, 120, 80), "繁华壁画");
        m_props << Prop("stone_tablet", 620, 440, 32, 40, QColor(170, 150, 100), "铭文石碑");
        m_props << Prop("stone_coffin", 870, 440, 64, 80, QColor(150, 140, 110), "国王石棺");
        m_props << Prop("sand_drift", 100, 600, 1280, 20, QColor(160, 140, 80), ""); // 沙地装饰

        // 出口
        m_exits << SceneExit{QRect(1150, 580, 60, 40), 3, 100, 300, ""}; // 塔外出口→火山

        m_flashbackTrigger = "wall_painting";
        m_hint = "壁画似乎记录着什么...石碑上有铭文";
    }
    else if (sceneId == 3) {
        // ===== 场景四：末日火山 =====
        m_name = "末日火山";
        m_bgColor = QColor(35, 20, 15);       // 黑灰暗红
        m_groundColor = QColor(50, 35, 30);   // 焦黑
        m_flashbackColor = QColor(40, 60, 30); // 茂林暖色

        m_platforms << QRect(0, 620, 1280, 100);
        m_platforms << QRect(400, 540, 100, 16);  // 岩石平台
        m_platforms << QRect(600, 480, 80, 16);   // 山腰平台
        m_platforms << QRect(800, 520, 100, 16);  // 崖边平台

        // 道具
        m_props << Prop("broken_sword", 830, 440, 16, 48, QColor(180, 180, 200), "火山断剑");
        m_props << Prop("lava_rock1", 200, 595, 32, 25, QColor(60, 40, 35), "焦黑岩石");
        m_props << Prop("lava_rock2", 500, 590, 40, 30, QColor(55, 38, 32), "焦黑岩石");
        m_props << Prop("ember_ground", 0, 615, 1280, 5, QColor(80, 30, 20), ""); // 灰烬

        // NPC
        m_npcs << Npc("sword_ghost", 820, 460, 24, 60, QColor(150, 150, 160, 100), "守剑幽魂");

        // 出口
        m_exits << SceneExit{QRect(50, 580, 40, 40), 0, 100, 300, ""}; // 出口→教堂

        m_flashbackTrigger = "broken_sword";
        m_hint = "崖边似乎插着什么...";
    }
    else if (sceneId == 4) {
        // ===== 场景五：图书馆秘境 =====
        m_name = "图书馆秘境";
        m_bgColor = QColor(40, 35, 50);       // 淡紫米白
        m_groundColor = QColor(70, 65, 60);   // 朽木地板
        m_flashbackColor = QColor(60, 55, 40); // 暖色图书馆

        m_platforms << QRect(0, 620, 1280, 100);
        m_platforms << QRect(300, 540, 100, 16);  // 书架残骸
        m_platforms << QRect(550, 500, 80, 16);   // 石台台阶
        m_platforms << QRect(900, 540, 100, 16);  // 木门前

        // 道具
        m_props << Prop("time_corridor", 50, 300, 80, 320, QColor(180, 180, 200, 80), "时空通道");
        m_props << Prop("manuscript", 350, 510, 16, 16, QColor(200, 190, 160), "残缺手稿");
        m_props << Prop("seed", 580, 470, 16, 16, QColor(100, 220, 120), "文明种子");
        m_props << Prop("old_door", 920, 480, 48, 140, QColor(120, 90, 60), "古木门");
        m_props << Prop("bookshelf1", 200, 480, 40, 140, QColor(80, 60, 40), "倒塌书架");
        m_props << Prop("bookshelf2", 700, 500, 35, 120, QColor(75, 55, 35), "倒塌书架");

        // 出口
        m_exits << SceneExit{QRect(920, 480, 48, 140), 5, 100, 300, ""}; // 古木门→终局

        m_flashbackTrigger = "manuscript";
        m_hint = "石台上似乎有什么在发光...";
    }
    else if (sceneId == 5) {
        // ===== 场景六：时空闭环（终局教堂） =====
        m_name = "时空闭环";
        m_bgColor = QColor(30, 28, 35);       // 与破败教堂一致
        m_groundColor = QColor(60, 55, 50);
        m_flashbackColor = QColor(80, 70, 50);

        m_platforms << QRect(0, 620, 1280, 100);
        m_platforms << QRect(80, 560, 80, 16);
        m_platforms << QRect(300, 540, 120, 16);

        // 与初始教堂相同的道具（但法阵可使用种子）
        m_props << Prop("magic_circle", 450, 580, 128, 40, QColor(180, 160, 80), "古老法阵");
        m_props << Prop("stone_door", 1050, 480, 64, 140, QColor(80, 75, 85), "闭锁石门");
        m_props << Prop("broken_statue", 700, 540, 32, 80, QColor(100, 95, 90), "断裂女神像");

        // 无出口，只有结局选择
        m_hint = "种子承载三种力量，你的选择，便是世界的终局。";
    }
}
