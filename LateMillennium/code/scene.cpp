#include "scene.h"

Scene::Scene()
{
    m_bgColor = QColor(20, 20, 30);
    m_groundColor = QColor(60, 55, 50);
    m_flashbackColor = QColor(255, 245, 230);
}

void Scene::load(int sceneId)
{
    m_platforms.clear();
    m_props.clear();
    m_npcs.clear();
    m_exits.clear();

    // 地面平台（所有场景通用）
    m_platforms << QRect(0, 620, 1280, 100);

    switch (sceneId) {
    case 0: // 破败教堂
        m_name = "破败教堂";
        m_hint = "调查神像、法阵、烛台了解过去";
        m_bgColor = QColor(25, 20, 30);
        m_groundColor = QColor(50, 45, 55);
        m_flashbackColor = QColor(255, 240, 220);
        m_bgImagePath = "bg/bg_church.png";
        m_bgFlashbackPath = "bg/bg_church_fb.png";

        // 场景道具：法阵（两种状态）
        m_props << Prop("magic_circle", 576, 580, 128, 40, QColor(100, 80, 120), "法阵",
                       "prop/prop_magic_circle.png", "prop/prop_magic_circle_glow.png");

        // 场景道具：神像（两种状态）
        m_props << Prop("church_statue", 300, 480, 100, 140, QColor(180, 160, 140), "神像",
                       "prop/prop_church_statue.png", "");

        // 场景道具：烛台（两种状态）
        m_props << Prop("church_candle", 150, 520, 80, 100, QColor(180, 160, 100), "烛台",
                       "prop/prop_church_candle.jpg", "");

        // NPC：井（两种状态）
        m_npcs << Npc("well", 1000, 500, 80, 120, QColor(100, 100, 120), "古井",
                     "prop/prop_well_blocked.png", "prop/prop_well_open.png");

        // NPC：石门（两种状态）
        m_npcs << Npc("stone_door", 1180, 480, 60, 140, QColor(80, 80, 90), "石门",
                     "npc/npc_stone_door.png", "prop/prop_portal_door.png");

        // 出口
        m_exits << SceneExit{QRect(1280, 300, 50, 320), 1, 100, 300, ""};
        break;

    case 1: // 井下地下城
        m_name = "井下地下城";
        m_hint = "调查幻梦机器、全息光屏";
        m_bgColor = QColor(15, 20, 35);
        m_groundColor = QColor(40, 45, 60);
        m_flashbackColor = QColor(230, 235, 250);
        m_bgImagePath = "bg/bg_dungeon.png";
        m_bgFlashbackPath = "bg/bg_dungeon_fb.jpg";

        // 场景道具：幻梦机器（两种状态）
        m_props << Prop("dream_machine", 500, 440, 200, 200, QColor(80, 70, 100), "幻梦机器",
                       "prop/prop_dream_machine.png", "");

        // 场景道具：全息光屏（两种状态）
        m_props << Prop("holo_screen", 840, 495, 120, 150, QColor(100, 120, 150), "光屏",
                       "prop/prop_holo_screen.png", "");

        // NPC：灵智龙虾（两种状态）
        m_npcs << Npc("lobster", 200, 520, 100, 100, QColor(200, 80, 60), "灵智龙虾",
                     "npc/npc_lobster_trapped.png", "npc/npc_lobster_saved.png");

        // 出口
        m_exits << SceneExit{QRect(-50, 300, 50, 320), 0, 1200, 300, ""};
        m_exits << SceneExit{QRect(1280, 300, 50, 320), 2, 100, 300, ""};
        break;

    case 2: // 沙漠金字塔
        m_name = "沙漠金字塔";
        m_hint = "调查壁画、石碑、石棺";
        m_bgColor = QColor(45, 35, 25);
        m_groundColor = QColor(80, 70, 50);
        m_flashbackColor = QColor(255, 245, 220);
        m_bgImagePath = "bg/bg_pyramid.png";
        m_bgFlashbackPath = "bg/bg_pyramid_fb.png";

        // 场景道具：壁画（两种状态）
        m_props << Prop("pyramid_wall", 550, 490, 200, 120, QColor(140, 120, 80), "壁画",
                       "prop/prop_wall_painting.png", "");

        // 场景道具：石碑（两种状态）
        m_props << Prop("stone_tablet", 350, 490, 80, 100, QColor(120, 110, 90), "石碑",
                       "prop/prop_stone_tablet.png", "");

        // 场景道具：石棺（两种状态）
        m_props << Prop("stone_coffin", 830, 540, 120, 80, QColor(100, 90, 80), "石棺",
                       "prop/prop_stone_coffin.png", "");

        // 出口
        m_exits << SceneExit{QRect(-50, 300, 50, 320), 1, 1200, 300, ""};
        m_exits << SceneExit{QRect(1280, 300, 50, 320), 3, 100, 300, ""};
        break;

    case 3: // 末日火山
        m_name = "末日火山";
        m_hint = "调查熔岩石、断剑，与幽魂对话";
        m_bgColor = QColor(40, 20, 15);
        m_groundColor = QColor(70, 45, 35);
        m_flashbackColor = QColor(255, 220, 200);
        m_bgImagePath = "bg/bg_volcano.jpg";
        m_bgFlashbackPath = "bg/bg_volcano_fb.jpg";

        // 场景道具：熔岩石（两种状态）
        m_props << Prop("volcano_rock", 300, 550, 100, 70, QColor(150, 60, 40), "熔岩石",
                       "", "");

        // 场景道具：断剑（两种状态：未掉落/已掉落）
        m_props << Prop("broken_sword", 830, 500, 60, 120, QColor(180, 180, 190), "断剑",
                       "", "prop/prop_broken_sword.png");

        // NPC：守剑幽魂（两种状态）
        m_npcs << Npc("sword_ghost", 830, 480, 80, 140, QColor(150, 150, 180), "幽魂",
                     "npc/npc_sword_ghost.png", "");

        // 出口
        m_exits << SceneExit{QRect(-50, 300, 50, 320), 2, 1200, 300, ""};
        m_exits << SceneExit{QRect(1280, 300, 50, 320), 4, 100, 300, "broken_sword"};
        break;

    case 4: // 图书馆秘境
        m_name = "图书馆秘境";
        m_hint = "调查书架、手稿、种子容器";
        m_bgColor = QColor(25, 25, 40);
        m_groundColor = QColor(50, 50, 65);
        m_flashbackColor = QColor(230, 235, 255);
        m_bgImagePath = "bg/bg_library.jpg";
        m_bgFlashbackPath = "bg/bg_library_fb.jpg";

        // 场景道具：书架（两种状态）
        m_props << Prop("bookshelf", 300, 480, 150, 140, QColor(80, 70, 60), "书架",
                       "prop/prop_bookshelf.png", "");

        // 场景道具：手稿（两种状态）
        m_props << Prop("manuscript", 500, 580, 60, 40, QColor(120, 110, 90), "手稿",
                       "prop/prop_manuscript.png", "");

        // 场景道具：种子容器（两种状态：未开启/已开启）
        m_props << Prop("seed_vessel", 700, 570, 60, 50, QColor(100, 120, 100), "种子容器",
                       "prop/prop_seed_vessel.png", "prop/prop_seed.png");

        // 场景道具：传送门
        m_props << Prop("portal", 1050, 420, 100, 200, QColor(150, 100, 200), "传送门",
                       "prop/prop_portal_door.png", "");

        // 出口
        m_exits << SceneExit{QRect(-50, 300, 50, 320), 3, 1200, 300, ""};
        m_exits << SceneExit{QRect(1280, 300, 50, 320), 5, 100, 540, "seed"};
        break;

    case 5: // 时空闭环（终局）
        m_name = "时空闭环";
        m_hint = "最终选择";
        m_bgColor = QColor(30, 25, 40);
        m_groundColor = QColor(55, 50, 65);
        m_flashbackColor = QColor(240, 235, 255);
        m_bgImagePath = "bg/bg_church.png";
        m_bgFlashbackPath = "bg/bg_church_fb.png";

        // 场景道具：发光法阵
        m_props << Prop("magic_circle_glow", 576, 580, 128, 40, QColor(150, 120, 200), "法阵",
                       "prop/prop_magic_circle_glow.png", "");

        // 出口
        m_exits << SceneExit{QRect(-50, 300, 50, 320), 4, 1200, 300, ""};
        break;
    }
}
