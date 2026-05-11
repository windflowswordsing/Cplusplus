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
    churchStart.text = "……我来了。\n\n召唤阵的光芒消散，眼前是一片死寂废墟。\n她等了一千年，可我……来晚了。";
    churchStart.nextKey = "";
    (*m_dialogs)["church_start"] = churchStart;

    // 场景0进入时自动回溯（纯净背景+暗示对话）
    DialogData churchAutoFlashback;
    churchAutoFlashback.speaker = "???";
    churchAutoFlashback.speakerColor = "gray";
    churchAutoFlashback.text = "「……你终于来了。\n\n  我等了很久……很久。\n\n  这个世界已经面目全非了，\n  但请你……不要放弃希望。\n\n  去看看吧，看看千年后的世界……\n  看看我守护的一切，还剩下什么。」";
    churchAutoFlashback.nextKey = "";
    (*m_dialogs)["church_auto_flashback"] = churchAutoFlashback;

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
    stoneDoor.speaker = "???";
    stoneDoor.speakerColor = "gray";
    stoneDoor.text = "石门紧闭，门身刻有暗紫色圣纹结界。\n\n……门无法从此侧打开。";
    stoneDoor.nextKey = "";
    (*m_dialogs)["stone_door_locked"] = stoneDoor;

    // 古井疏通
    DialogData wellClear;
    wellClear.speaker = "勇者";
    wellClear.speakerColor = "white";
    wellClear.text = "古井被碎石堵住了……\n\n（移开石块后，幽深的井底传来阵阵凉意）\n\n……下面似乎通向某个地方。";
    wellClear.nextKey = "";
    (*m_dialogs)["well_clear"] = wellClear;

    // ===== 场景二：地下城 =====

    // 救下龙虾
    DialogData lobsterSave;
    lobsterSave.speaker = "勇者";
    lobsterSave.speakerColor = "white";
    lobsterSave.text = "……你会说话？";
    lobsterSave.nextKey = "lobster_reply";
    (*m_dialogs)["lobster"] = lobsterSave;

    DialogData lobsterReply;
    lobsterReply.speaker = "灵智龙虾";
    lobsterReply.speakerColor = "yellow";
    lobsterReply.text = "末日降临时，人类建造这座地下城，收纳文明火种。\n那位圣者暗中布下结界庇护我们，却始终不肯露面。\n\n这是「幻梦机器」——能编织没有末日、没有痛苦的完美世界。\n一旦进入，便再也无法醒来。";
    lobsterReply.nextKey = "lobster_hint";
    (*m_dialogs)["lobster_reply"] = lobsterReply;

    DialogData lobsterHint;
    lobsterHint.speaker = "灵智龙虾";
    lobsterHint.speakerColor = "yellow";
    lobsterHint.text = "……火山之巅，有圣者留下的信物。\n或许那就是打开那扇石门的关键。";
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

    // 幻梦机器（锁定状态）
    DialogData dreamMachineLocked;
    dreamMachineLocked.speaker = "???";
    dreamMachineLocked.speakerColor = "gray";
    dreamMachineLocked.text = "幻梦机器被某种力量封锁着……\n\n（需要先释放龙虾，才能解除封锁）";
    dreamMachineLocked.nextKey = "";
    (*m_dialogs)["dream_machine_locked"] = dreamMachineLocked;

    // 幻梦机器
    DialogData dreamMachine;
    dreamMachine.speaker = "幻梦机器";
    dreamMachine.speakerColor = "blue";
    dreamMachine.text = "幻梦机器发出低沉的嗡鸣……\n\n法阵中央浮现一道空间裂隙，通往未知的远方。\n另一侧似乎……是沙漠。";
    DialogChoice choice1;
    choice1.text = "启动幻梦机器（沉沦）";
    choice1.action = "dream_enter";
    DialogChoice choice2;
    choice2.text = "穿过空间裂隙（传送）";
    choice2.action = "dream_teleport";
    DialogChoice choice3;
    choice3.text = "离开";
    choice3.action = "dream_leave";
    dreamMachine.choices << choice1 << choice2 << choice3;
    dreamMachine.isEnding = true;
    dreamMachine.endingType = 4;
    (*m_dialogs)["dream_machine"] = dreamMachine;

    // 幻梦机器（发现种子后，可回到原来世界）
    DialogData dreamMachineWithSeed;
    dreamMachineWithSeed.speaker = "幻梦机器";
    dreamMachineWithSeed.speakerColor = "blue";
    dreamMachineWithSeed.text = "幻梦机器发出低沉的嗡鸣……\n\n法阵中央浮现一道空间裂隙，\n你手中的种子散发着温暖的光芒……\n\n「时空通道已开启。」\n「你可以选择……回到原来的世界。」";
    DialogChoice dmChoice1;
    dmChoice1.text = "启动幻梦机器（沉沦）";
    dmChoice1.action = "dream_enter";
    DialogChoice dmChoice2;
    dmChoice2.text = "穿过空间裂隙（传送至金字塔）";
    dmChoice2.action = "dream_teleport";
    DialogChoice dmChoice3;
    dmChoice3.text = "回到原来的世界（如同大梦一场）";
    dmChoice3.action = "return_original_world";
    DialogChoice dmChoice4;
    dmChoice4.text = "离开";
    dmChoice4.action = "dream_leave";
    dreamMachineWithSeed.choices << dmChoice1 << dmChoice2 << dmChoice3 << dmChoice4;
    (*m_dialogs)["dream_machine_with_seed"] = dreamMachineWithSeed;

    // 全息光屏（场景1地下城）
    DialogData holoScreen;
    holoScreen.speaker = "全息光屏";
    holoScreen.speakerColor = "cyan";
    holoScreen.text = "【滋滋……沙沙……】\n\n现在是……末日纪元……1000年……\n【滋滋……】\n\n文明……已覆灭……圣女……千年前……\n【沙沙……滋滋……】\n\n信号……中断……";
    holoScreen.nextKey = "";
    (*m_dialogs)["holo_screen"] = holoScreen;

    // ===== 场景三：金字塔 =====

    DialogData pyramidPainting;
    pyramidPainting.speaker = "勇者";
    pyramidPainting.speakerColor = "white";
    pyramidPainting.text = "这里似乎曾经……是一座辉煌的城邦。\n\n壁画上描绘着绿洲环绕的都城，\n百姓耕种、商旅往来、孩童嬉戏于喷泉之畔……\n\n然而画面渐变——天空裂开，黄沙吞没一切，\n繁华的街巷被荒漠掩埋，\n曾经鲜活的面容，如今只剩岩壁上模糊的轮廓。\n\n千年的文明，就这样……消散在风沙里了。";
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
    stoneTablet.text = "铭文亮起金色光芒——\n\n「圣石成棺，秘藏永生，可避灾厄。\n  王命筑之，未及启封，王已陨灭。」\n\n（石棺上的封印似乎……松动了。）";
    stoneTablet.nextKey = "";
    (*m_dialogs)["stone_tablet"] = stoneTablet;

    // 石棺（解锁后）
    DialogData stoneCoffin;
    stoneCoffin.speaker = "国王石棺";
    stoneCoffin.speakerColor = "gold";
    stoneCoffin.text = "石棺的封印已经解除，\n棺盖缓缓滑开，露出幽蓝色的光芒……\n\n躺入其中，据说可以获得永生，\n永远躲避末日的灾厄。\n\n但代价是——永远沉眠于此。";
    DialogChoice coffinEnter;
    coffinEnter.text = "躺入石棺（永眠）";
    coffinEnter.action = "coffin_enter";
    DialogChoice coffinTeleport;
    coffinTeleport.text = "穿过石棺旁的暗道（传送）";
    coffinTeleport.action = "coffin_teleport";
    DialogChoice coffinLeave;
    coffinLeave.text = "离开";
    coffinLeave.action = "leave_coffin";
    stoneCoffin.choices << coffinEnter << coffinTeleport << coffinLeave;
    (*m_dialogs)["stone_coffin"] = stoneCoffin;

    // 石棺未解锁提示
    DialogData coffinLocked;
    coffinLocked.speaker = "勇者";
    coffinLocked.speakerColor = "white";
    coffinLocked.text = "石棺被古老的金色封印锁住了……\n\n（也许铭文石碑上记载着解开封印的方法。）";
    coffinLocked.nextKey = "";
    (*m_dialogs)["stone_coffin_locked"] = coffinLocked;

    // ===== 场景四：火山 =====

    // 守剑幽魂：第一次交互（喃喃自语）
    DialogData ghostTalk;
    ghostTalk.speaker = "守剑幽魂";
    ghostTalk.speakerColor = "gray";
    ghostTalk.text = "「……嗯……？\n\n  ……有人……来了……\n\n  ……千年了……终于……\n\n  ……守着……这把剑……\n\n  ……等一个……有缘人……」";
    ghostTalk.nextKey = "";
    (*m_dialogs)["sword_ghost"] = ghostTalk;

    // 守剑幽魂：第二次交互（选择取走/离开）
    DialogData ghostChoice;
    ghostChoice.speaker = "守剑幽魂";
    ghostChoice.speakerColor = "gray";
    ghostChoice.text = "「……你……就是那个人吗……\n\n  ……这把断剑……是圣女留下的……\n\n  ……能打开……那扇被封锁的门……\n\n  ……拿走吧……我……守够了……」";
    DialogChoice takeSword;
    takeSword.text = "取走断剑";
    takeSword.action = "take_sword";
    DialogChoice leaveGhost;
    leaveGhost.text = "离开";
    leaveGhost.action = "leave_ghost";
    ghostChoice.choices << takeSword << leaveGhost;
    (*m_dialogs)["sword_ghost_choice"] = ghostChoice;

    // 取走断剑后的对话
    DialogData takeSwordDialog;
    takeSwordDialog.speaker = "守剑幽魂";
    takeSwordDialog.speakerColor = "gray";
    takeSwordDialog.text = "「……嗯……去吧……\n\n  ……替我……看看……门后面的世界……\n\n  ……是不是……还有希望……」\n\n（幽魂的身形逐渐化为银色光点，融入断剑之中……）";
    takeSwordDialog.nextKey = "";
    (*m_dialogs)["take_sword_dialog"] = takeSwordDialog;

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
    libraryEnter.text = "结界解开了……\n\n门后竟然是图书馆。\n她……一直在这里布局。";
    libraryEnter.nextKey = "";
    (*m_dialogs)["library_enter"] = libraryEnter;

    DialogData manuscript;
    manuscript.speaker = "手稿";
    manuscript.speakerColor = "beige";
    manuscript.text = "「末日不可逆，勇者不可逝，\n  以千年为约，留迹于世，待君择路。」";
    manuscript.nextKey = "";
    (*m_dialogs)["manuscript"] = manuscript;

    // 手稿（未发现种子时）
    DialogData manuscriptLocked;
    manuscriptLocked.speaker = "勇者";
    manuscriptLocked.speakerColor = "white";
    manuscriptLocked.text = "这似乎是魔法书的残页……\n\n记录着时间魔法的推演……\n\n「末日不可逆，勇者不可逝，\n  以千年为约，留迹于世，待君择路。」\n\n（魔法发动还需要什么……）";
    manuscriptLocked.nextKey = "";
    (*m_dialogs)["manuscript_locked"] = manuscriptLocked;

    // 手稿选择（研究触发结局）
    DialogData manuscriptChoice;
    manuscriptChoice.speaker = "残缺手稿";
    manuscriptChoice.speakerColor = "beige";
    manuscriptChoice.text = "手稿上密密麻麻写满了时空魔法的推演……\n\n「末日不可逆，勇者不可逝，\n  以千年为约，留迹于世，待君择路。」\n\n研究这份手稿，可以逆流千年，回到过去，\n在末日降临之前，与圣女并肩，抵抗那场浩劫。\n\n但代价是——你将永远留在那个时空，\n再也无法回到你原本的世界。";
    DialogChoice studyManuscript;
    studyManuscript.text = "研究手稿（回到过去，抵抗末日）";
    studyManuscript.action = "study_manuscript";
    DialogChoice leaveManuscript;
    leaveManuscript.text = "离开";
    leaveManuscript.action = "leave_manuscript";
    manuscriptChoice.choices << studyManuscript << leaveManuscript;
    (*m_dialogs)["manuscript_choice"] = manuscriptChoice;

    // 发现种子
    DialogData seedDiscovered;
    seedDiscovered.speaker = "勇者";
    seedDiscovered.speakerColor = "white";
    seedDiscovered.text = "容器中散发着柔和的光芒……\n\n是一颗种子！\n\n「文明种子——蕴含着生命与希望的种子，\n  可在废土中生根发芽，重建文明。」\n\n（这或许就是圣女留下的希望……）";
    seedDiscovered.nextKey = "";
    (*m_dialogs)["seed_discovered"] = seedDiscovered;

    // 种子选择
    DialogData seedChoice;
    seedChoice.speaker = "文明种子";
    seedChoice.speakerColor = "green";
    seedChoice.text = "种子散发着温暖的光芒……\n\n使用它，可以在废土中播种希望，\n重建文明，开启新的纪元。\n\n但代价是——你将永远留在这个世界。";
    DialogChoice useSeed;
    useSeed.text = "使用种子（文明新生）";
    useSeed.action = "use_seed";
    DialogChoice leaveSeed;
    leaveSeed.text = "离开";
    leaveSeed.action = "leave_seed";
    seedChoice.choices << useSeed << leaveSeed;
    (*m_dialogs)["seed_choice"] = seedChoice;

    // 场景4自动回溯：圣女内心矛盾独白
    DialogData libraryAutoFlashback;
    libraryAutoFlashback.speaker = "圣女";
    libraryAutoFlashback.speakerColor = "white";
    libraryAutoFlashback.text = "「……千年了。\n\n  我在时空的裂隙中等待，看着世界一点点崩塌……\n\n  末日……真的无法逆转吗？\n\n  我推演了无数种可能，\n  每一条路都通向毁灭，\n  除了……那一个选择。\n\n  可那个选择……代价太大了。\n\n  我该让勇者承担这份重负吗？\n  还是……让他永远沉睡在幻梦之中？\n\n  ……不，他有权利知道真相，\n  也有权利……做出自己的选择。\n\n  我在图书馆留下了种子和手稿，\n  在法阵中刻下了最后的抉择……\n\n  等待千年，只为这一刻。」";
    libraryAutoFlashback.isFlashback = true;
    libraryAutoFlashback.nextKey = "";
    (*m_dialogs)["library_auto_flashback"] = libraryAutoFlashback;

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
    loopShock.text = "……竟然回到了最初的地方。\n\n是时空闭环。\n她瞒过了所有人，等了我……一千年。";
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
