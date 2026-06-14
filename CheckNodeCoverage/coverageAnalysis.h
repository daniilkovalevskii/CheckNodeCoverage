#ifndef COVERAGEANALYSIS_H
#define COVERAGEANALYSIS_H
#include "structures.h"

/*!
 * \file coverageAnalysis.h
 * \brief Функции для проверки покрытия целевого узла набором отмеченных узлов
 */

/*!
 * \brief Рекурсивно обрабатывает узлы выше целевого.
 *
 * Ищет ошибочно отмеченные узлы вне поддерева целевого узла и ищет сам целевой узел.
 *
 * \param[in] node Текущий анализируемый узел
 * \param[in,out] errors Набор ошибок
 * \param[in,out] result Структура с результатами анализа покрытия целевого узла
 */
void dfsAbove(Node* node, QSet<Error>& errors, Result& result);

/*!
 * \brief Рекурсивно проверяет покрытие целевого узла набором отмеченных узлов.
 * \param[in] node Текущий анализируемый узел
 * \param[in,out] errors Набор ошибок
 * \param[in,out] result Структура с результатами анализа покрытия целевого узла
 * \return Статус покрытия для узла
 */
CoverageStatus dfsSearching(Node* node, QSet<Error>& errors, Result& result);

/*!
 * \brief Рекурсивно ищет ошибочно отмеченные узлы (избыточные) ниже целевого.
 * \param[in] node Указатель на текущий узел
 * \param[in] activeMarked Указатель на отмеченный узел, поддерево которого анализируется
 * \param[in,out] errors Набор ошибок
 */
void dfsCovered(Node* node, Node* activeMarked, QSet<Error>& errors);

#endif // COVERAGEANALYSIS_H
