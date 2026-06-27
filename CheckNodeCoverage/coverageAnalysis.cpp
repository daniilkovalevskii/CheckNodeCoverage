#include "coverageAnalysis.h"
#include <QSet>
#include <QDebug>

void dfsAbove(Node* node, QSet<Error>& errors, Result& result)
{
    // Если текущий узел является целевым, запускаем от него dfsSearching
    if (node->shape == NodeShape::DIAMOND)
    {
        dfsSearching(node, errors, result);
        return;
    }
    // Иначе Если текущий узел отмечен - добавляем ошибку
    else if (node->shape == NodeShape::RECTANGLE)
    {
        errors.insert(Error(ErrorType::NOT_DESCENDANT, node->name));
    }

    // Продолжаем поиск по всем детям
    for (Node* child : node->children)
    {
        dfsAbove(child, errors, result);
    }
}

CoverageStatus dfsSearching(Node* node, QSet<Error>& errors, Result& result)
{
    // Если текущий узел отмечен, добавляем его в корректные и ищем ошибки ниже
    if (node->shape == NodeShape::RECTANGLE)
    {
        result.valid.append(node);
        dfsCovered(node, node, errors);
        return FULLY_COVERED;
    }
    // Если текущий узел лист - возвращаем наверх информацию о том, что он непокрыт
    if (node->isLeaf())
    {
        return NOT_COVERED;
    }

    // Для всех остальных случаев собираем статусы детей
    QVector<CoverageStatus> status;
    for (Node* child : node->children)
    {
        status.append(dfsSearching(child, errors, result));
    }

    // Если все статусы FULLY_COVERED - возвращаем FULLY_COVERED
    if (!status.contains(NOT_COVERED) && !status.contains(PARTIALLY_COVERED))
    {
        return FULLY_COVERED;
    }

    // Если все статусы NOT_COVERED и текущий узел не является целевым - возвращаем NOT_COVERED
    if (!status.contains(FULLY_COVERED) && !status.contains(PARTIALLY_COVERED) && node->shape != NodeShape::DIAMOND)
    {
        return NOT_COVERED;
    }

    // Иначе(смешанные статусы или текущий узел цель и есть непокрытые потомки),
    // Добавляем в missing непокрытых детей
    for (int i = 0; i < node->children.size(); ++i)
    {
        if (status[i] == NOT_COVERED)
        {
            result.missing.append(node->children[i]);
        }
    }

    // Вернуть PARTIALLY_COVERED
    return PARTIALLY_COVERED;
}

void dfsCovered(const Node* node, const Node* activeMarked, QSet<Error>& errors)
{
    // Для каждого ребенка текущего узла
    for (const Node* child : node->children)
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
