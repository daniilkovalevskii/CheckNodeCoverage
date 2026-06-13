#include "coverageAnalysis.h"
#include <QSet>
#include <QDebug>

void dfsAbove(Node* node, QSet<Error>& errors, Result& result)
{
    return;
}

CoverageStatus dfsSearching(Node* node, QSet<Error>& errors, Result& result)
{
    return CoverageStatus::NOT_COVERED;
}

void dfsCovered(Node* node, Node* activeMarked, QSet<Error>& errors)
{
    // Для каждого ребенка текущего узла
    for (Node* child : node->children)
    {
        // Если ребенок отмечен(имеет форму rectangle), добавляем ошибку
        if (child->shape == NodeShape::RECTANGLE)
        {
            errors.insert(Error(ErrorType::REDUNDANT_NODE, QString("%1").arg(child->name), QString("%1").arg(activeMarked->name)));
        }

        // Запускаем рекурсию от ребенка, сохранив указатель на уже отмеченный узел
        dfsCovered(child, activeMarked, errors);
    }
}
