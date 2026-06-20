#ifndef COVERAGEANALYSIS_H
#define COVERAGEANALYSIS_H
#include "structures.h"

/*!
 * \file coverageAnalysis.h
 * \brief Функции для проверки покрытия целевого узла набором отмеченных узлов.
 */

/*!
 * \brief Рекурсивно обрабатывает узлы выше целевого.
 *
 * Ищет ошибочно отмеченные узлы вне поддерева целевого узла и ищет сам целевой узел.
 *
 * \param[in] node Текущий анализируемый узел.
 * \param[in,out] errors Набор ошибок.
 * \param[in,out] result Структура с результатами анализа покрытия целевого узла.
 *
 * \pre Узел \c node не должен быть равен \c nullptr. Структура графа гарантированно должна быть валидным деревом без циклов.
 * \post Спускается по дереву. При нахождении целевого узла передает управление в функцию \c dfsSearching. Отмеченные узлы выше цели заносит в \c errors как \c ErrorType::NOT_DESCENDANT.
 */
void dfsAbove(Node* node, QSet<Error>& errors, Result& result);

/*!
 * \brief Рекурсивно проверяет покрытие целевого узла набором отмеченных узлов.
 * \param[in] node Текущий анализируемый узел.
 * \param[in,out] errors Набор ошибок.
 * \param[in,out] result Структура с результатами анализа покрытия целевого узла.
 * \return Статус покрытия для узла.
 *
 * \pre Функция должна вызываться от целевого узла графа или его прямых потомков. Узел \c node не равен \c nullptr.
 * \post Вычисляет статус покрытия поддерева. Корректные вершины сохраняет в список \c result.valid, а недостающие для покрытия узлы — в список \c result.missing.
 */
CoverageStatus dfsSearching(Node* node, QSet<Error>& errors, Result& result);

/*!
 * \brief Рекурсивно ищет ошибочно отмеченные узлы (избыточные) ниже целевого.
 * \param[in] node Указатель на текущий узел.
 * \param[in] activeMarked Указатель на отмеченный узел, поддерево которого анализируется.
 * \param[in,out] errors Набор ошибок.
 *
 * \pre Переданный узел \c node валиден и не равен \c nullptr. Параметр \c activeMarked указывает на вышестоящий отмеченный узел.
 * \post Сканирует поддерево ниже \c activeMarked. Любой встреченный прямоугольный узел регистрирует как избыточный и добавляет в \c errors запись \c ErrorType::REDUNDANT_NODE.
 */
void dfsCovered(Node* node, Node* activeMarked, QSet<Error>& errors);

#endif // COVERAGEANALYSIS_H
