#include "dialogue.h"
#include <QMap>

Dialogue::Dialogue()
{
    m_dialogs = new QMap<QString, DialogData>();
    initDialogs();
}

void Dialogue::initDialogs()
{
    m_dialogs->clear();

    // ===== 场景一：破败教堂 =====

    // 初始独白
    DialogData churchStart;
    churchStart.speaker = "勇者";
    churchStart.speakerColor = "white";
    churchStart.text = "这里就是被召唤的世界？没有生机，没有战火，只有废墟……\n召唤阵还在，可一切都太晚了。";
    churchStart.nextKey = "";
    (*m_dialogs)["church_start"] = churchStart;

    // 法阵回溯后的残影对话
    DialogData churchFlashback;
    churchFlashback.speaker = "残影";
    churchFlashback.speakerColor = "gray";
    churchFlashback.text = "民众1：「圣女大人还在法阵前守着，她一定要等到勇者。」\n\n民众2：「末日越来越近，她已经很久没有休息了，到底在盘算什么……」";
    churchFlashback.isFlashback = true;
    churchFlashback.nextKey = "";
    (*m_dialogs)["church_flashback"] = churchFlashback;

    // 石门提示
    DialogData stoneDoor;
    stoneDoor.speaker = "系统";
    stoneDoor.speakerColor = "yellow";
    stoneDoor.text = "石门被强大魔法封印，无法打开。\n门身刻有暗紫色圣纹结界，似乎需要特殊信物才能解除。";
    stoneDoor.nextKey = "";
    (*m_dialogs)["stone_door_locked"] = stoneDoor;

    // 石堆/古井
    DialogData wellEntry;
    wellEntry.speaker = "系统";
    wellEntry.speakerColor = "cyan";
    wellEntry.text = "移开石堆，露出了一口古井……\n井底似乎通向某个地方。\n\n按 E 进入古井通道 → 前往地下城";
    wellEntry.nextKey = "";
    (*m_dialogs)["well_entrance"] = wellEntry;

    // ===== 场景二：地下城 =====

    // 救下龙虾
    DialogData lobsterSave;
    lobsterSave.speaker = "勇者";
    lobsterSave.speakerColor = "white";
    lobsterSave.text = "你能说话？这里是什么地方，这台机器又是做什么的？";
    lobsterSave.nextKey = "lobster_reply";
    (*m_dialogs)["lobster"] = lobsterSave;

    DialogData lobsterReply;
    lobsterReply.speaker = "灵智龙虾";
    lobsterReply.speakerColor = "yellow";
    lobsterReply.text = "末日降临时，人类建造这座地下城，收纳文明火种以求存续。\n那位圣者暗中布下结界庇护我们，却始终不肯露面。\n\n这是「幻梦机器」，能编织没有末日、没有痛苦的完美世界，\n只是一旦进入，就再也无法醒来。";
    lobsterReply.nextKey = "lobster_hint";
    (*m_dialogs)["lobster_reply"] = lobsterReply;

    DialogData lobsterHint;
    lobsterHint.speaker = "灵智龙虾";
    lobsterHint.speakerColor = "yellow";
    lobsterHint.text = "对了……火山之巅，有圣者留下的信物。\n或许那就是打开那扇石门的关键。";
    lobsterHint.nextKey = "";
    (*m_dialogs)["lobster_hint"] = lobsterHint;

    // 地下城回溯
    DialogData dungeonFlashback;
    dungeonFlashback.speaker = "残影";
    dungeonFlashback.speakerColor = "gray";
    dungeonFlashback.text = "居民：「圣者从不来这里，却一直默默护着我们。」\n\n管理者：「这机器是她授意建造的，像是留给世人最后的慰藉。」";
    dungeonFlashback.isFlashback = true;
    dungeonFlashback.nextKey = "";
    (*m_dialogs)["dungeon_flashback"] = dungeonFlashback;

    // 幻梦机器
    DialogData dreamMachine;
    dreamMachine.speaker = "系统";
    dreamMachine.speakerColor = "blue";
    dreamMachine.text = "幻梦机器的屏幕微微亮起……\n\n启动后将进入完美幻梦，无法回头。";
    DialogChoice choice1;
    choice1.text = "启动幻梦机器";
    choice1.action = "dream_enter";
    DialogChoice choice2;
    choice2.text = "离开";
    choice2.action = "dream_leave";
    dreamMachine.choices << choice1 << choice2;
    dreamMachine.isEnding = true;
    dreamMachine.endingType = 4;
    (*m_dialogs)["dream_machine"] = dreamMachine;

    // ===== 场景三：金字塔 =====

    DialogData pyramidPainting;
    pyramidPainting.speaker = "勇者";
    pyramidPainting.speakerColor = "white";
    pyramidPainting.text = "曾经如此繁盛的文明，如今只剩黄沙掩埋。\n壁画上描绘着昔日的绿洲城邦、农耕与庆典盛景……";
    pyramidPainting.nextKey = "";
    (*m_dialogs)["wall_painting"] = pyramidPainting;

    DialogData pyramidFlashback;
    pyramidFlashback.speaker = "残影";
    pyramidFlashback.speakerColor = "gray";
    pyramidFlashback.text = "百姓：「圣女走遍所有绿洲留下圣痕，却从不说未来如何。」\n\n侍从：「国王为求永生建造此棺，可城破人亡，一切都来不及了。」";
    pyramidFlashback.isFlashback = true;
    pyramidFlashback.nextKey = "";
    (*m_dialogs)["pyramid_flashback"] = pyramidFlashback;

    DialogData stoneTablet;
    stoneTablet.speaker = "铭文";
    stoneTablet.speakerColor = "gold";
    stoneTablet.text = "「此棺以圣石铸就，入之可获永生，躲避末日劫难。\n  王国尚在时奉王令建造，未及启用，王已死于叛军。」";
    stoneTablet.nextKey = "";
    (*m_dialogs)["stone_tablet"] = stoneTablet;

    // 石棺
    DialogData stoneCoffin;
    stoneCoffin.speaker = "系统";
    stoneCoffin.speakerColor = "darkYellow";
    stoneCoffin.text = "石棺的棺盖半掩着，内部散发着幽幽光芒……\n\n躺入石棺，寻求永生（不可逆）。";
    DialogChoice choice3;
    choice3.text = "躺入石棺";
    choice3.action = "coffin_enter";
    DialogChoice choice4;
    choice4.text = "离开";
    choice4.action = "coffin_leave";
    stoneCoffin.choices << choice3 << choice4;
    stoneCoffin.isEnding = true;
    stoneCoffin.endingType = 5;
    (*m_dialogs)["stone_coffin"] = stoneCoffin;

    // ===== 场景四：火山 =====

    DialogData ghostTalk;
    ghostTalk.speaker = "勇者";
    ghostTalk.speakerColor = "white";
    ghostTalk.text = "你在这里守着什么？";
    ghostTalk.nextKey = "ghost_reply";
    (*m_dialogs)["sword_ghost"] = ghostTalk;

    DialogData ghostReply;
    ghostReply.speaker = "守剑幽魂";
    ghostReply.speakerColor = "gray";
    ghostReply.text = "「反抗末日。」";
    ghostReply.nextKey = "";
    (*m_dialogs)["ghost_reply"] = ghostReply;

    DialogData volcanoFlashback;
    volcanoFlashback.speaker = "残影";
    volcanoFlashback.speakerColor = "gray";
    volcanoFlashback.text = "老者：「圣女曾登临此山，亲手折断佩剑，说要留给后世勇者。」\n\n勇士：「我们要去反抗末日，她拦不住我们！」";
    volcanoFlashback.isFlashback = true;
    volcanoFlashback.nextKey = "";
    (*m_dialogs)["volcano_flashback"] = volcanoFlashback;

    DialogData pickSword;
    pickSword.speaker = "系统";
    pickSword.speakerColor = "white";
    pickSword.text = "你拔起了断剑——\n银灰剑身锈迹斑驳，却仍能感受到圣力萦绕。\n幽魂缓缓消散，化为点点银光……\n\n获得关键道具：【火山断剑（结界钥匙）】";
    pickSword.nextKey = "";
    (*m_dialogs)["pick_sword"] = pickSword;

    // ===== 场景五：图书馆 =====

    DialogData libraryEnter;
    libraryEnter.speaker = "勇者";
    libraryEnter.speakerColor = "white";
    libraryEnter.text = "结界解开了……门后竟然是图书馆。\n她一直在这里布局。";
    libraryEnter.nextKey = "";
    (*m_dialogs)["library_enter"] = libraryEnter;

    DialogData manuscript;
    manuscript.speaker = "手稿";
    manuscript.speakerColor = "beige";
    manuscript.text = "「末日不可逆，勇者不可逝，\n  以千年为约，留迹于世，待君择路。」";
    manuscript.nextKey = "";
    (*m_dialogs)["manuscript"] = manuscript;

    DialogData libraryFlashback;
    libraryFlashback.speaker = "残影";
    libraryFlashback.speakerColor = "gray";
    libraryFlashback.text = "法师：「她整日研究时空禁术，从不与人言说。」\n\n学者：「她好像早已看透结局，只是独自在修改什么。」";
    libraryFlashback.isFlashback = true;
    libraryFlashback.nextKey = "";
    (*m_dialogs)["library_flashback"] = libraryFlashback;

    DialogData pickSeed;
    pickSeed.speaker = "系统";
    pickSeed.speakerColor = "green";
    pickSeed.text = "你从石台上取下了文明种子——\n晶莹淡绿，微光闪烁，蕴含着新生的力量。\n\n获得关键道具：【文明种子】";
    pickSeed.nextKey = "";
    (*m_dialogs)["pick_seed"] = pickSeed;

    // ===== 场景六：终局 =====

    DialogData loopShock;
    loopShock.speaker = "勇者";
    loopShock.speakerColor = "white";
    loopShock.text = "竟然回到了最初的地方……是时空闭环。\n她瞒过了所有人，等了我一千年。";
    loopShock.nextKey = "";
    (*m_dialogs)["loop_shock"] = loopShock;

    DialogData finalChoice;
    finalChoice.speaker = "系统";
    finalChoice.speakerColor = "gold";
    finalChoice.text = "种子承载三种力量，你的选择，便是世界的终局。\n\n你站在法阵前，手中握着文明种子……";
    DialogChoice c1;
    c1.text = "在法阵中使用种子 → 文明新生";
    c1.action = "ending_newborn";
    DialogChoice c2;
    c2.text = "使用种子的时空之力 → 归世幻梦";
    c2.action = "ending_dream";
    DialogChoice c3;
    c3.text = "留在图书馆参悟手稿 → 逆时赴约";
    c3.action = "ending_time";
    finalChoice.choices << c1 << c2 << c3;
    finalChoice.isEnding = true;
    finalChoice.endingType = 0; // 由action决定
    (*m_dialogs)["final_choice"] = finalChoice;
}

DialogData Dialogue::getDialog(const QString &key)
{
    if (m_dialogs->contains(key))
        return (*m_dialogs)[key];
    DialogData empty;
    empty.text = "";
    return empty;
}

DialogData Dialogue::getFlashback(const QString &sceneTrigger)
{
    QString key = sceneTrigger + "_flashback";
    if (m_dialogs->contains(key))
        return (*m_dialogs)[key];

    // 通用回溯
    DialogData fb;
    fb.speaker = "残影";
    fb.speakerColor = "gray";
    fb.text = "……圣女的白色剪影一闪而过……";
    fb.isFlashback = true;
    return fb;
}

QString Dialogue::getHint(const QString &key)
{
    if (key == "stone_door") return "石门被强大魔法封印，无法打开";
    if (key == "dream_machine") return "幻梦机器 - 启动后将进入幻梦，无法回头";
    if (key == "stone_coffin") return "国王石棺 - 躺入可获永生（不可逆）";
    return "";
}
