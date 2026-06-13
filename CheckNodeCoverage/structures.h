#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <QString>
#include <QVector>
#include <QMap>

enum NodeShape
{
    DIAMOND,
    RECTANGLE,
    DEFAULT
};

class Node
{
public:
    Node(QString newName, NodeShape newShape = NodeShape::DEFAULT)
    {
        name = newName;
        shape = newShape;

        parent = nullptr;
    };

    ~Node()
    {}

    QString name;
    NodeShape shape;
    Node* parent;
    QVector<Node*> children;

    bool isLeaf() const
    {
        return children.isEmpty();
    };
};

enum ErrorType
{
    FILE_ERROR,
    OUTPUT_FILE_ERROR,
    CONNECTIVITY_ERROR,
    NO_MARKED_NODES,
    LEAF_TARGET,
    NO_TARGET,
    MULTIPLE_TARGETS,
    CYCLE_ERROR,
    NOT_DESCENDANT,
    MULTI_PARENT,
    REDUNDANT_NODE,
    NO_ROOT,
    SYNTAX_ERROR,
    FORBIDDEN_STRUCTURE_OR_FORM
};

class Error
{
    public:
        Error()
        {
        };
        Error(ErrorType newType, QString nodeName1 = "", QString nodeName2 = "", QString newPath = "", int newLine = -1)
        {
            type = newType;
            nodeName = nodeName1;
            relatedNodeName = nodeName2;
            path = newPath;
            line = newLine;
        };

        ErrorType type;
        QString nodeName;
        QString relatedNodeName;
        QString path;
        int line;

        QString generateMessage() const
        {
            switch (type)
            {
                case FILE_ERROR:
                    return "Входной файл не найден или недоступен для чтения.";

                case OUTPUT_FILE_ERROR:
                    return "Неверно указан файл для выходных данных. Возможно указанного расположения не существует или нет прав на запись.";

                case CYCLE_ERROR:
                    return QString("Обнаружен цикл %1. Граф не является деревом.").arg(path);

                case FORBIDDEN_STRUCTURE_OR_FORM:
                    return QString("Обнаружена запрещенная структура или сложный тип узла в строке %1. Используйте только плоский digraph и простые формы.").arg(line);

                case NO_TARGET:
                    return "Не найдено целевого узла.";

                case CONNECTIVITY_ERROR:
                    return QString("Нарушена связность. Нет связи между узлами %1 и %2.").arg(nodeName, relatedNodeName);

                case SYNTAX_ERROR:
                    return QString("Синтаксическая ошибка в описании графа DOT в строке %1.").arg(line);

                case MULTIPLE_TARGETS:
                    return QString("Обнаружено более одного целевого узла (Узлы %1). Допустим только один целевой узел.").arg(nodeName);

                case NOT_DESCENDANT:
                    return QString("Узел %1 не является потомком целевого узла.").arg(nodeName);

                case REDUNDANT_NODE:
                    return QString("Узел %1 избыточен (находится в поддереве уже отмеченного узла %2).").arg(nodeName, relatedNodeName);

                case LEAF_TARGET:
                    return QString("Целевой узел %1 не может быть листом.").arg(nodeName);

                case MULTI_PARENT:
                    return QString("Узел %1 имеет несколько родителей.").arg(nodeName);

                case NO_ROOT:
                    return "Не удалось определить корень дерева.";

                case NO_MARKED_NODES:
                    return "Нет отмеченных узлов покрытия.";
            }

            return "";
        }
        bool operator==(const Error& other) const
        {
            return type == other.type &&
                   nodeName == other.nodeName &&
                   relatedNodeName == other.relatedNodeName &&
                   path == other.path &&
                   line == other.line;
        };
};

inline size_t qHash(const Error& key, size_t seed = 0)
{
    return qHash(static_cast<int>(key.type), seed) ^
           qHash(key.nodeName, seed) ^
           qHash(key.relatedNodeName, seed) ^
           qHash(key.path, seed) ^
           qHash(key.line, seed);
}

enum CoverageStatus
{
    NOT_COVERED,
    PARTIALLY_COVERED,
    FULLY_COVERED
};

struct ParseResult
{
    Node* root;
    QMap<QString, Node*> allNodes;
};

struct Result
{
    QVector<Node*> valid;
    QVector<Node*> missing;
};

#endif // STRUCTURES_H
