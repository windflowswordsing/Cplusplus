#include "dialogue.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

Dialogue::Dialogue()
{
    initDialogs();
}

Dialogue::~Dialogue()
{
}

void Dialogue::initDialogs()
{
    m_dialogs.clear();

    // 查找对话文本文件
    QString filePath;
    QStringList candidates = {
        QCoreApplication::applicationDirPath() + "/dialogues.txt",
        QDir::currentPath() + "/dialogues.txt",
        QDir::currentPath() + "/../dialogues.txt",
        QDir::currentPath() + "/code/dialogues.txt",
        "C:/Users/29742/AppData/Roaming/TRAE SOLO CN/ModularData/ai-agent/work-mode-projects/6a02c580c6009f364917cb50/LateMillennium/code/dialogues.txt"
    };

    for (const QString &c : candidates) {
        if (QFile::exists(c)) {
            filePath = c;
            break;
        }
    }

    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Dialogue file not found, loading defaults";
        loadDefaultDialogs();
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QString currentBlock;
    QString currentKey;

    while (!in.atEnd()) {
        QString line = in.readLine();

        // 跳过空行和注释
        if (line.trimmed().isEmpty() || line.trimmed().startsWith("#")) {
            continue;
        }

        // 新对话块开始
        if (line.startsWith("[") && line.contains("]")) {
            // 解析之前的块
            if (!currentBlock.isEmpty() && !currentKey.isEmpty()) {
                parseDialogBlock(currentBlock);
            }
            // 开始新块
            currentKey = line.mid(1, line.indexOf("]") - 1);
            currentBlock = line + "\n";
        } else {
            currentBlock += line + "\n";
        }
    }

    // 解析最后一个块
    if (!currentBlock.isEmpty() && !currentKey.isEmpty()) {
        parseDialogBlock(currentBlock);
    }

    file.close();
    qDebug() << "Loaded" << m_dialogs.size() << "dialogs from" << filePath;
}

void Dialogue::parseDialogBlock(const QString &block)
{
    QStringList lines = block.split("\n", Qt::SkipEmptyParts);
    if (lines.isEmpty()) return;

    // 第一行是 [key]
    QString key = lines[0].mid(1, lines[0].indexOf("]") - 1);
    if (key.isEmpty()) return;

    DialogData data;
    data.key = key;

    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty() || line.startsWith("#")) continue;

        int eqPos = line.indexOf("=");
        if (eqPos < 0) continue;

        QString field = line.left(eqPos).trimmed();
        QString value = line.mid(eqPos + 1).trimmed();

        if (field == "type") {
            data.type = parseType(value);
        } else if (field == "speaker") {
            data.speaker = value;
        } else if (field == "color") {
            data.speakerColor = value;
        } else if (field == "text") {
            // 处理多行文本，将 \n\n 替换为实际换行
            data.text = value.replace("\\n\\n", "\n\n").replace("\\n", "\n");
        } else if (field == "next") {
            data.nextKey = value;
        } else if (field == "flashback") {
            data.isFlashback = (value.toLower() == "true");
        } else if (field == "narration") {
            data.isNarration = (value.toLower() == "true");
        } else if (field == "tip") {
            data.tipText = value;
        } else if (field == "ending") {
            data.isEnding = true;
            data.endingType = value.toInt();
        } else if (field == "choice") {
            // 格式: 选项文字|动作标识|说明
            QStringList parts = value.split("|");
            if (parts.size() >= 2) {
                DialogChoice choice;
                choice.text = parts[0].trimmed();
                choice.action = parts[1].trimmed();
                if (parts.size() >= 3) {
                    choice.desc = parts[2].trimmed();
                }
                data.choices.append(choice);
            }
        }
    }

    // 如果没有指定类型，根据内容推断
    if (data.type == DialogType::Talk) {
        if (!data.choices.isEmpty()) {
            data.type = DialogType::Choice;
        } else if (data.isEnding) {
            data.type = DialogType::Ending;
        }
    }

    m_dialogs[key] = data;
}

DialogType Dialogue::parseType(const QString &typeStr)
{
    QString lower = typeStr.toLower();
    if (lower == "investigate") return DialogType::Investigate;
    if (lower == "talk") return DialogType::Talk;
    if (lower == "choice") return DialogType::Choice;
    if (lower == "ending") return DialogType::Ending;
    return DialogType::Talk;
}

void Dialogue::loadDefaultDialogs()
{
    DialogData d;
    d.key = "default";
    d.type = DialogType::Talk;
    d.speaker = "系统";
    d.speakerColor = "white";
    d.text = "对话文件未找到。";
    m_dialogs["default"] = d;
}

DialogData Dialogue::getDialog(const QString &key)
{
    if (m_dialogs.contains(key)) {
        return m_dialogs[key];
    }
    // 返回空对话
    DialogData empty;
    empty.key = key;
    empty.text = "";
    return empty;
}

bool Dialogue::hasDialog(const QString &key)
{
    return m_dialogs.contains(key);
}

QString Dialogue::getHint(const QString &key)
{
    if (key == "stone_door") return "石门被强大魔法封印，无法打开";
    if (key == "dream_machine") return "幻梦机器 - 启动后将进入幻梦，无法回头";
    if (key == "stone_coffin") return "国王石棺 - 躺入可获永生（不可逆）";
    return "";
}
