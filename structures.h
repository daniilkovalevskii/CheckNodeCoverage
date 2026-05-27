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
    Node();
    Node(QString newName, NodeShape newShape);

    QString name;
    NodeShape shape;
    Node* parent;
    QVector<Node*> children;

    bool isLeaf() const;
    bool operator== (Node &other);
};

enum ErrorType {
    FILE_ERROR,
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
    Error();
    Error(ErrorType type, QString nodeName1, QString nodeName2, QString path, int line);

    ErrorType type;
    QString nodeName;
    QString relatedNodeName;
    QString path;
    int line;

    QString generateMessage() const;
    bool operator==(const Error& other) const;
};

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
