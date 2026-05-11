#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QList>

// 对话选项
struct DialogChoice {
    QString text;
    QString action;  // 动作标识
};

// 对话数据
struct DialogData {
    QString speaker;           // 说话人
    QString speakerColor;      // 说话人颜色标识
    QString text;              // 对话内容
    QList<DialogChoice> choices; // 选项（空=无选项，按E继续）
    QString nextKey;           // 下一段对话key（空=结束）
    bool isFlashback;          // 是否是回溯残影对话
    bool isEnding;             // 是否触发结局
    int endingType;            // 结局类型
};

class Dialogue
{
public:
    Dialogue();

    // 获取对话数据
    DialogData getDialog(const QString &key);

    // 获取回溯对话（圣女剪影场景）
    DialogData getFlashback(const QString &sceneTrigger);

    // 获取提示文本
    static QString getHint(const QString &key);

private:
    void initDialogs();
    void loadDefaultDialogs();
    QMap<QString, DialogData> *m_dialogs;
};

#endif // DIALOGUE_H
