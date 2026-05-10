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

    // 法阵回溯后的残影对话（第一章：破败教堂）
    DialogData churchFlashback;
    churchFlashback.speaker = "不知名人";
    churchFlashback.speakerColor = "gray";
    churchFlashback.text = "光阴倒流，尘埃散尽，教堂重回千年前盛景……\n\n圣歌缭绕，钟声悠远，祭司、贵族、平民、法师挤满殿堂，\n人人面色惶然，却都望向祭坛方向——那道素白身影。";
    churchFlashback.isFlashback = true;
    churchFlashback.nextKey = "church_flashback_2";
    (*m_dialogs)["church_flashback"] = churchFlashback;

    DialogData churchFb2;
    churchFb2.speaker = "不知名人";
    churchFb2.speakerColor = "gray";
    churchFb2.text = "民众：「全凭圣女撑住召唤阵，维系时空通路，\n  不然我们连一丝希望都没有。」\n\n民众：「末日阴霾日重一日，族中长老劝她分担重任，\n  她却都婉拒了，把所有担子都揽在自己身上。」";
    churchFb2.isFlashback = true;
    churchFb2.nextKey = "church_flashback_3";
    (*m_dialogs)["church_flashback_2"] = churchFb2;

    DialogData churchFb3;
    churchFb3.speaker = "不知名人";
    churchFb3.speakerColor = "gray";
    churchFb3.text = "民众：「近来她常独自守在法阵旁，闭门不语，\n  烛火彻夜不熄，仿佛在独自斟酌什么重大抉择。」\n\n祭坛圣光缭绕，一抹素白身影静立其上，\n衣袂轻扬，被万人仰赖，却又透着难以言说的孤绝……";
    churchFb3.isFlashback = true;
    churchFb3.nextKey = "church_flashback_4";
    (*m_dialogs)["church_flashback_3"] = churchFb3;

    DialogData churchFb4;
    churchFb4.speaker = "不知名人";
    churchFb4.speakerColor = "gray";
    churchFb4.text = "身影渐渐转过身来，轮廓愈发清晰，似要露出眉眼——\n\n回溯光芒骤然崩碎，银白流光化作细碎像素……\n\n重回死寂教堂。勇者心底隐隐生出异样：\n那份沉默独处、暗自斟酌的模样，\n总透着几分不为人知的隐秘。";
    churchFb4.isFlashback = true;
    churchFb4.nextKey = "";
    (*m_dialogs)["church_flashback_4"] = churchFb4;

    // 石门提示
    DialogData stoneDoor;
    stoneDoor.speaker = "系统";
    stoneDoor.speakerColor = "yellow";
    stoneDoor.text = "石门被强大魔法封印，无法打开。\n门身刻有暗紫色圣纹结界，似乎需要特殊信物才能解除。";
    stoneDoor.nextKey = "";
    (*m_dialogs)["stone_door_locked"] = stoneDoor;

    // 古井疏通
    DialogData wellClear;
    wellClear.speaker = "勇者";
    wellClear.speakerColor = "white";
    wellClear.text = "这些石块堵住了古井……让我把它们移开。\n\n（经过一番努力，古井终于疏通了）\n幽深的井底似乎通向某个地方。";
    wellClear.nextKey = "";
    (*m_dialogs)["well_clear"] = wellClear;

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

    // 地下城回溯（第二章：赛博地下城）
    DialogData dungeonFlashback;
    dungeonFlashback.speaker = "不知名人";
    dungeonFlashback.speakerColor = "gray";
    dungeonFlashback.text = "地下城瞬间恢复繁华，霓虹流光溢彩……\n\n孩童在巷道间奔走嬉闹，笑声清脆；\n学者们围在书架旁整理万卷藏书；\n工匠们调试机械，忙碌而有序。\n地底自成一方安稳桃源。";
    dungeonFlashback.isFlashback = true;
    dungeonFlashback.nextKey = "dungeon_flashback_2";
    (*m_dialogs)["dungeon_flashback"] = dungeonFlashback;

    DialogData dungeonFb2;
    dungeonFb2.speaker = "不知名人";
    dungeonFb2.speakerColor = "gray";
    dungeonFb2.text = "居民：「多亏那位圣者的结界庇护，我们才能躲过浩劫，\n  可我们连她的名字都不知道。」\n\n居民：「她从不现身与人相见，有人说曾在巷道尽头\n  见过她的身影，白衣胜雪，却转瞬即逝。」";
    dungeonFb2.isFlashback = true;
    dungeonFb2.nextKey = "dungeon_flashback_3";
    (*m_dialogs)["dungeon_flashback_2"] = dungeonFb2;

    DialogData dungeonFb3;
    dungeonFb3.speaker = "不知名人";
    dungeonFb3.speakerColor = "gray";
    dungeonFb3.text = "居民：「还有人说，这幻梦机器，\n  是她留给那些撑不住绝望的人，最后的慰藉。」\n\n巷道远处，一道浅白衣影静静伫立在霓虹光影中，\n似在凝望人间烟火，眼神温柔却又疏离……";
    dungeonFb3.isFlashback = true;
    dungeonFb3.nextKey = "dungeon_flashback_4";
    (*m_dialogs)["dungeon_flashback_3"] = dungeonFb3;

    DialogData dungeonFb4;
    dungeonFb4.speaker = "不知名人";
    dungeonFb4.speakerColor = "gray";
    dungeonFb4.text = "勇者凝神望去，刚要辨清身形眉目——\n\n回溯戛然而止，银白流光消散，繁华褪去……\n\n眼前重回锈铁空城。勇者隐约察觉：\n那位圣女始终刻意疏离、不露行踪，\n仿佛在隐藏什么，不愿被任何人知晓。";
    dungeonFb4.isFlashback = true;
    dungeonFb4.nextKey = "";
    (*m_dialogs)["dungeon_flashback_4"] = dungeonFb4;

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

    // 金字塔回溯（第三章：沙漠金字塔）
    DialogData pyramidFlashback;
    pyramidFlashback.speaker = "不知名人";
    pyramidFlashback.speakerColor = "gray";
    pyramidFlashback.text = "黄沙褪去，满目青绿，千里绿洲连绵不绝……\n\n城邦林立，楼宇错落，商旅络绎不绝，\n百姓安居乐业，一派盛世景象。\n可这份繁华之下，却藏着末日将至的阴影。";
    pyramidFlashback.isFlashback = true;
    pyramidFlashback.nextKey = "pyramid_flashback_2";
    (*m_dialogs)["pyramid_flashback"] = pyramidFlashback;

    DialogData pyramidFb2;
    pyramidFb2.speaker = "不知名人";
    pyramidFb2.speakerColor = "gray";
    pyramidFb2.text = "百姓：「圣女走遍每一片绿洲，在水源、古石上留下圣痕，\n  有人说那些圣痕能护佑我们躲过浩劫，可她从不明说。」\n\n百姓：「她从不说末日能否逆转，只是沉默走过大地，\n  四处留迹，眼神里藏着我们读不懂的沉重。」";
    pyramidFb2.isFlashback = true;
    pyramidFb2.nextKey = "pyramid_flashback_3";
    (*m_dialogs)["pyramid_flashback_2"] = pyramidFb2;

    DialogData pyramidFb3;
    pyramidFb3.speaker = "不知名人";
    pyramidFb3.speakerColor = "gray";
    pyramidFb3.text = "远处沙丘顶端，一道孤影迎风静立，\n白衣与风沙共舞，望着连片绿洲城邦，久久凝望……\n\n身影孤寂而坚定，似在与这片山河作别，\n又似在默默守护。勇者凝神望去——";
    pyramidFb3.isFlashback = true;
    pyramidFb3.nextKey = "pyramid_flashback_4";
    (*m_dialogs)["pyramid_flashback_3"] = pyramidFb3;

    DialogData pyramidFb4;
    pyramidFb4.speaker = "不知名人";
    pyramidFb4.speakerColor = "gray";
    pyramidFb4.text = "面容将近显形——\n\n回溯光影骤然碎裂，银白流光消散……\n\n重回昏暗金字塔。石碑旁石壁上，\n一行浅浅刻字，无署名、无落款：\n「山河有尽，文明有痕，有缘之人，自循迹而来。」";
    pyramidFb4.isFlashback = true;
    pyramidFb4.nextKey = "";
    (*m_dialogs)["pyramid_flashback_4"] = pyramidFb4;

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

    // 火山回溯（第四章：末日火山）
    DialogData volcanoFlashback;
    volcanoFlashback.speaker = "不知名人";
    volcanoFlashback.speakerColor = "gray";
    volcanoFlashback.text = "焦土重归万顷林海，古木参天，枝叶繁茂……\n\n山脚下村落炊烟袅袅，百姓日出而作、日落而息。\n可安宁终究只是短暂假象——\n黑雾铺天盖地从天际压落，吞噬草木，侵蚀土地。";
    volcanoFlashback.isFlashback = true;
    volcanoFlashback.nextKey = "volcano_flashback_2";
    (*m_dialogs)["volcano_flashback"] = volcanoFlashback;

    DialogData volcanoFb2;
    volcanoFb2.speaker = "不知名人";
    volcanoFb2.speakerColor = "gray";
    volcanoFb2.text = "村里最勇敢的农夫、猎手、匠人，\n不愿坐以待毙，纷纷拿起农具、锈刃，\n结伴向火山山顶走去——\n明知以凡人之躯抗衡末日毫无胜算，却依旧义无反顾。";
    volcanoFb2.isFlashback = true;
    volcanoFb2.nextKey = "volcano_flashback_3";
    (*m_dialogs)["volcano_flashback_2"] = volcanoFb2;

    DialogData volcanoFb3;
    volcanoFb3.speaker = "不知名人";
    volcanoFb3.speakerColor = "gray";
    volcanoFb3.text = "老者：「昔日圣女曾登临此山，驻足良久，\n  望着山下村落，沉默了许久，\n  随后亲手折去佩剑一半，将半截断剑留在这崖边——\n  此物待后世有缘勇者，可启尘封之境。」";
    volcanoFb3.isFlashback = true;
    volcanoFb3.nextKey = "volcano_flashback_4";
    (*m_dialogs)["volcano_flashback_3"] = volcanoFb3;

    DialogData volcanoFb4;
    volcanoFb4.speaker = "不知名人";
    volcanoFb4.speakerColor = "gray";
    volcanoFb4.text = "山巅崖边，圣女孤身立在风里，白衣猎猎，\n低头凝望那柄断剑，身影孤寂淡然……\n\n回溯之力彻底耗尽，画面瞬间消散。\n勇者拔出断剑，幽魂化为银光融入剑身——\n这柄断剑，是解开结界、打开秘境之门的钥匙。";
    volcanoFb4.isFlashback = true;
    volcanoFb4.nextKey = "";
    (*m_dialogs)["volcano_flashback_4"] = volcanoFb4;

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

    // 图书馆回溯（第五章：图书馆秘境）
    DialogData libraryFlashback;
    libraryFlashback.speaker = "不知名人";
    libraryFlashback.speakerColor = "gray";
    libraryFlashback.text = "图书馆灯火长明，万卷书架林立……\n\n学者与法师往来论学，低声交谈，文风鼎盛。\n圣女常独自静坐书案前，埋首古籍，\n手中执笔，在纸上推演秘术与时空法理。";
    libraryFlashback.isFlashback = true;
    libraryFlashback.nextKey = "library_flashback_2";
    (*m_dialogs)["library_flashback"] = libraryFlashback;

    DialogData libraryFb2;
    libraryFb2.speaker = "不知名人";
    libraryFb2.speakerColor = "gray";
    libraryFb2.text = "法师：「她一人背负世间救世之责，心思深沉，\n  从来都是独来独往，\n  仿佛早已习惯了独自承担一切。」\n\n法师：「她推演的时空秘术太过诡异，不似寻常救世之法，\n  仿佛早已看透前路宿命，只是不愿明说。」";
    libraryFb2.isFlashback = true;
    libraryFb2.nextKey = "library_flashback_3";
    (*m_dialogs)["library_flashback_2"] = libraryFb2;

    DialogData libraryFb3;
    libraryFb3.speaker = "不知名人";
    libraryFb3.speakerColor = "gray";
    libraryFb3.text = "圣女垂眸执笔，神情沉静孤寂，\n笔尖在纸上飞速游走，留下密密麻麻的推演……\n\n忽而若有所感，目光轻轻投向那扇紧闭的内门，\n眼神复杂，似有牵挂，似有决绝——";
    libraryFb3.isFlashback = true;
    libraryFb3.nextKey = "library_flashback_4";
    (*m_dialogs)["library_flashback_3"] = libraryFb3;

    DialogData libraryFb4;
    libraryFb4.speaker = "不知名人";
    libraryFb4.speakerColor = "gray";
    libraryFb4.text = "容颜将要清晰显露一瞬——\n\n回溯最后一次中断，光影彻底散去，\n银白流光消失无踪，再也无法催动。\n\n满地残稿散落，纸上尽是时间魔法的推演与注释，\n却没有任何一行文字直白写下她的心思。\n一切抉择与隐忍，只待勇者自行领悟。";
    libraryFb4.isFlashback = true;
    libraryFb4.nextKey = "";
    (*m_dialogs)["library_flashback_4"] = libraryFb4;

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
    fb.speaker = "不知名人";
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
