#ifndef TESTHELPERS_H
#define TESTHELPERS_H
#include "../CheckNodeCoverage/structures.h"
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include "QTest"

// Функция для чтения строк из файла
QStringList readDotFile(const QString& relativePath)
{
    QString fullPath = QFINDTESTDATA(relativePath);
    if (fullPath.isEmpty()) {
        fullPath = QCoreApplication::applicationDirPath() + "/" + relativePath;
    }

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть тестовый файл:" << fullPath;
        return QStringList();
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    return lines;
}

// Конвертация ошибок в удобочитаемый формат
QString errorTypeToString(ErrorType type)
{
    switch(type)
    {
    case ErrorType::FILE_ERROR:
        return "FILE_ERROR";
    case ErrorType::OUTPUT_FILE_ERROR:
        return "OUTPUT_FILE_ERROR";
    case ErrorType::CONNECTIVITY_ERROR:
        return "CONNECTIVITY_ERROR";
    case ErrorType::NO_MARKED_NODES:
        return "NO_MARKED_NODES";
    case ErrorType::LEAF_TARGET:
        return "LEAF_TARGET";
    case ErrorType::NO_TARGET:
        return "NO_TARGET";
    case ErrorType::MULTIPLE_TARGETS:
        return "MULTIPLE_TARGETS";
    case ErrorType::CYCLE_ERROR:
        return "CYCLE_ERROR";
    case ErrorType::NOT_DESCENDANT:
        return "NOT_DESCENDANT";
    case ErrorType::MULTI_PARENT:
        return "MULTI_PARENT";
    case ErrorType::REDUNDANT_NODE:
        return "REDUNDANT_NODE";
    case ErrorType::NO_ROOT:
        return "NO_ROOT";
    case ErrorType::SYNTAX_ERROR:
        return "SYNTAX_ERROR";
    case ErrorType::FORBIDDEN_STRUCTURE_OR_FORM:
        return "FORBIDDEN_STRUCTURE_OR_FORM";
    default:
        return "UNKNOWN_ERROR";
    }
}

// Функция для сравнения ожидаемых и полученных ошибок
void showErrorLog(const QSet<Error>& actual, const QSet<Error>& expected)
{
    QStringList missing;
    QStringList unexpected;

    // Сбор недостающих ошибок
    for (const Error& e : expected)
    {
        if (!actual.contains(e))
        {
            QString desc = QString("Тип: %1, Строка: %2")
                               .arg(errorTypeToString(e.type))
                               .arg(e.line);
            if (!e.nodeName.isEmpty())
                desc += ", Узел: " + e.nodeName;
            if (!e.relatedNodeName.isEmpty())
                desc += " -> " + e.relatedNodeName;
            if (!e.path.isEmpty())
                desc += ", Путь: " + e.path;

            missing << desc;
        }
    }

    // Сбор лишних ошибок
    for (const Error& e : actual)
    {
        if (!expected.contains(e))
        {
            QString desc = QString("Тип: %1, Строка: %2")
                               .arg(errorTypeToString(e.type))
                               .arg(e.line);
            if (!e.nodeName.isEmpty()) desc += ", Узел: " + e.nodeName;
            if (!e.relatedNodeName.isEmpty()) desc += " -> " + e.relatedNodeName;
            if (!e.path.isEmpty()) desc += ", Путь: " + e.path;

            unexpected << desc;
        }
    }

    // Вывод отчета
    if (!missing.isEmpty())
    {
        qWarning().noquote() << "--- НЕ ХВАТАЕТ ОШИБОК ---";
        for (int i = 0; i < missing.size(); ++i)
        {
            qWarning().noquote() << missing.at(i);
        }
    }
    if (!unexpected.isEmpty())
    {
        qWarning().noquote() << "--- НЕОЖИДАННЫЕ ОШИБКИ ---";
        for (int i = 0; i < unexpected.size(); ++i)
        {
            qWarning().noquote() << unexpected.at(i);
        }
    }
}

#endif // TESTHELPERS_H
