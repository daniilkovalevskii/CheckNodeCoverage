#include "inputProcessing.h"

ParseResult parseDOT(const QStringList& lines, QSet<Error>& errors)
{
    ParseResult result;
    return result;
}

void dfsValidate(Node* node, QSet<Node*>& visited, QStack<Node*>& stack, QSet<Error>& errors)
{
    return;
}

bool findComponentRoot(Node* node, const QSet<Node*>& visited, Node*& outRoot)
{
    outRoot = nullptr;
    return false;
}

void validateStructure(Node* root, QSet<Error>& errors, const QMap<QString, Node*>& allNodes)
{
    return;
}
