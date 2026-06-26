#ifndef TESTHELPERS_H
#define TESTHELPERS_H
#include "../CheckNodeCoverage/structures.h"
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include "QTest"

typedef QVector<QPair<QString, QString>> EdgeList;
Q_DECLARE_METATYPE(EdgeList)

// Функция для чтения строк из файла
QStringList readDotFile(const QString& relativePath)
{
    QString fullPath = QFINDTESTDATA(relativePath);
    if (fullPath.isEmpty())
    {
        fullPath = QCoreApplication::applicationDirPath() + "/" + relativePath;
    }

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Не удалось открыть тестовый файл:" << fullPath;
        return QStringList();
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd())
    {
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
            {
                desc += ", Узел: " + e.nodeName;
            }
            if (!e.relatedNodeName.isEmpty())
            {
                desc += " -> " + e.relatedNodeName;
            }
            if (!e.path.isEmpty())
            {
                desc += ", Путь: " + e.path;
            }

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
            if (!e.nodeName.isEmpty())
            {
                desc += ", Узел: " + e.nodeName;
            }
            if (!e.relatedNodeName.isEmpty())
            {
                desc += " -> " + e.relatedNodeName;
            }
            if (!e.path.isEmpty())
            {
                desc += ", Путь: " + e.path;
            }

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

// Функция для построения дерева программно
Node* buildTree(const QVector<QPair<QString, QString>>& edges, QMap<QString, Node*>& outNodes)
{
    outNodes.clear();

    for (const auto& edge : edges)
    {
        if (!outNodes.contains(edge.first))
        {
            outNodes[edge.first] = new Node(edge.first);
        }
        if (!outNodes.contains(edge.second))
        {
            outNodes[edge.second] = new Node(edge.second);
        }

        Node* parent = outNodes[edge.first];
        Node* child = outNodes[edge.second];

        if (child->parent == nullptr)
        {
            child->parent = parent;
            parent->children.append(child);
        }
    }

    for (Node* node : outNodes)
    {
        if (node->parent == nullptr)
        {
            return node;
        }
    }

    return nullptr;
}

struct ExpectedNodeProfile
{
    QString name;
    NodeShape shape;
    QString parentName;
    QStringList childNames;
};

// Функция для проверки построенного дерева
bool treeMatchesExpectedProfile(const QMap<QString, Node*>& allNodes,
                                const QVector<ExpectedNodeProfile>& expectedProfiles,
                                QString& outError)
{
    QStringList errorList;

    // Проверяем состав узлов
    QSet<QString> actualNames;
    for (auto it = allNodes.constBegin(); it != allNodes.constEnd(); ++it)
    {
        actualNames.insert(it.key());
    }

    QSet<QString> expectedNames;
    for (const auto& p : expectedProfiles)
    {
        expectedNames.insert(p.name);
    }

    if (actualNames != expectedNames)
    {
        QStringList missing = (expectedNames - actualNames).values();
        QStringList unexpected = (actualNames - expectedNames).values();
        if (!missing.isEmpty())
        {
            errorList << "Отсутствуют: " + missing.join(", ");
        }
        if (!unexpected.isEmpty())
        {
            errorList << "Лишние: " + unexpected.join(", ");
        }
    }

    // Проверяем свойства каждого ожидаемого узла
    for (const ExpectedNodeProfile& profile : expectedProfiles)
    {
        if (allNodes.contains(profile.name))
        {
            Node* n = allNodes[profile.name];

            // Проверка формы узла
            if (n->shape != profile.shape)
            {
                errorList << QString("Узел '%1': неверная форма (ожид. %2, пол. %3)")
                                 .arg(profile.name)
                                 .arg(static_cast<int>(profile.shape))
                                 .arg(static_cast<int>(n->shape));
            }

            // Проверка родителя
            QString actualParentName = n->parent ? n->parent->name : "";
            if (actualParentName != profile.parentName)
            {
                errorList << QString("Узел '%1': неверный родитель (ожид. '%2', пол. '%3')")
                                 .arg(profile.name, profile.parentName, actualParentName);
            }

            // Проверка детей
            QStringList actualChildrenNames;
            for (Node* child : n->children)
            {
                if (child)
                {
                    actualChildrenNames << child->name;
                }
            }

            // Создаем копию для сортировки
            QStringList expectedChildrenNames = profile.childNames;
            actualChildrenNames.sort();
            expectedChildrenNames.sort();

            if (actualChildrenNames != expectedChildrenNames)
            {
                errorList << QString("Узел '%1': списки детей не совпадают (ожид. [%2], пол. [%3])")
                                 .arg(profile.name)
                                 .arg(expectedChildrenNames.join(", "))
                                 .arg(actualChildrenNames.join(", "));
            }
        }
    }

    // Формируем итоговый результат
    if (!errorList.isEmpty())
    {
        outError = errorList.join("\n");
        return false;
    }

    return true;
}

#endif // TESTHELPERS_H
