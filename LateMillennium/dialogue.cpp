#include "dialogue.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>

Dialogue::Dialogue()
{
    m_dialogs = new QMap<QString, DialogData>();
    initDialogs();
}

void Dialogue::initDialogs()
{
    m_dialogs->clear();

    // 查找对话文本文件
    QString filePath = "dialogues.txt";
    QString appDir = QCoreApplication::applicationDirPath();
    if (QFile::exists(appDir + "/dialogues.txt")) {
        filePath = appDir + "/dialogues.txt";
    } else if (QFile::exists("C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium/dialogues.txt")) {
        filePath = "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/69ff33d94039cdfcb3becb60/LateMillennium/dialogues.txt";
    } else if (QFile::exists(QDir::currentPath() + "/dialogues.txt")) {
        filePath = QDir::currentPath() + "/dialogues.txt";
    }

    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 文件不存在，使用内置默认对话
        loadDefaultDialogs();
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QString currentKey;
    DialogData current;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // 跳过空行和注释
        if (line.isEmpty() || line.startsWith("#")) continue;

        // 新对话块
        if (line.startsWith("[") && line.endsWith("]")) {
            // 保存上一个对话
            if (!currentKey.isEmpty()) {
                (*m_dialogs)[currentKey] = current;
            }
            currentKey = line.mid(1, line.length() - 2);
            current = DialogData();
            current.isFlashback = false;
            current.isEnding = false;
            current.endingType = 0;
            continue;
        }

        // 解析键值对
        int eqPos = line.indexOf(" = ");
        if (eqPos < 0) eqPos = line.indexOf("=");
        if (eqPos < 0) continue;

        QString key = line.left(eqPos).trimmed();
        QString value = line.mid(eqPos + 1).trimmed();

        if (key == "speaker") {
            current.speaker = value;
        } else if (key == "color") {
            current.speakerColor = value;
        } else if (key == "text") {
            // 将 \n 转换为实际换行
            current.text = value.replace("\\n", "\n");
        } else if (key == "next") {
            current.nextKey = value;
        } else if (key == "flashback") {
            current.isFlashback = (value.toLower() == "true");
        } else if (key == "ending") {
            current.isEnding = true;
            current.endingType = value.toInt();
        } else if (key == "choice") {
            // 格式: 选项文字|action
            int pipePos = value.indexOf("|");
            if (pipePos >= 0) {
                DialogChoice c;
                c.text = value.left(pipePos).trimmed();
                c.action = value.mid(pipePos + 1).trimmed();
                current.choices.append(c);
            }
        }
    }

    // 保存最后一个对话
    if (!currentKey.isEmpty()) {
        (*m_dialogs)[currentKey] = current;
    }

    file.close();
}

void Dialogue::loadDefaultDialogs()
{
    // 内置默认对话（当文本文件不存在时使用）
    DialogData d;
    d.speaker = "系统";
    d.speakerColor = "white";
    d.text = "对话文件 dialogues.txt 未找到。\n请确保该文件与程序在同一目录下。";
    d.nextKey = "";
    (*m_dialogs)["default"] = d;
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

    DialogData fb;
    fb.speaker = "不知名人";
    fb.speakerColor = "gray";
    fb.text = "......一抹白色剪影一闪而过......";
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
