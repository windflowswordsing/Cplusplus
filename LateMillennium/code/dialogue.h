#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

// 对话类型枚举
enum class DialogType {
    Talk,        // 普通对话（与NPC交谈）
    Investigate, // 调查道具（触发回溯）
    Choice,      // 选项选择
    Ending       // 结局
};

// 选项结构
struct DialogChoice {
    QString text;       // 显示文字
    QString action;     // 动作标识
    QString desc;       // 选项说明
};

// 对话数据
struct DialogData {
    QString key;               // 对话标识
    DialogType type;           // 对话类型
    QString speaker;           // 说话人
    QString speakerColor;      // 颜色标识
    QString text;              // 对话内容
    QList<DialogChoice> choices; // 选项列表
    QString nextKey;           // 下一段对话key
    bool isFlashback;          // 是否回溯对话
    bool isEnding;             // 是否触发结局
    int endingType;            // 结局类型

    DialogData() : type(DialogType::Talk), isFlashback(false), isEnding(false), endingType(0) {}

    // 是否处于回溯状态（用于渲染回溯背景）
    bool inFlashbackState() const {
        return isFlashback || type == DialogType::Investigate;
    }
};

class Dialogue
{
public:
    Dialogue();
    ~Dialogue();

    // 获取对话数据
    DialogData getDialog(const QString &key);

    // 检查对话是否存在
    bool hasDialog(const QString &key);

    // 获取提示文本
    static QString getHint(const QString &key);

private:
    void initDialogs();
    void loadDefaultDialogs();
    void parseDialogBlock(const QString &block);
    DialogType parseType(const QString &typeStr);

    QMap<QString, DialogData> m_dialogs;
};

#endif // DIALOGUE_H
