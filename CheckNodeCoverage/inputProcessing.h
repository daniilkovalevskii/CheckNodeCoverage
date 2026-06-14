#ifndef INPUTPROCESSING_H
#define INPUTPROCESSING_H
#include "structures.h"

/*!
 * \file inputProcessing.h
 * \brief Функции для чтения, парсинга и проверки дерева DOT.
 */

/*!
 * \brief Построчно считывает файл в массив строк.
 * \param[in] filepath Путь к входному файлу
 * \param[in,out] errors Набор ошибок
 * \return Массив строк с содержимым файла
 */
QStringList readFile(const QString& filepath, QSet<Error>& errors);

/*!
 * \brief Программно строит дерево, описанное в массиве строк.
 * \param[in] lines Массив строк входного файла
 * \param[in,out] errors Набор ошибок
 * \return Структура с результатами парсинга (указатель на корень и словарь узлов дерева)
 */
ParseResult parseDOT(const QStringList& lines, QSet<Error>& errors);

/*!
 * \brief Рекурсивная функция для поиска циклов в графе.
 * \param[in] node Узел, с которого начинается проверка
 * \param[in,out] visited Множество уже посещенных узлов в ходе проверки на циклы
 * \param[in,out] stack Стек для хранения пути цикла
 * \param[in,out] errors Набор ошибок
 */
void dfsValidate(Node* node, QSet<Node*>& visited, QStack<Node*>& stack, QSet<Error>& errors);

/*!
 * \brief Ищет локальный корень компоненты.
 * \param[in] node Узел, с которого начинается поиск корня компоненты
 * \param[in] visited Множество посещенных узлов в ходе проверки на циклы
 * \param[out] outRoot Указатель на корень локальной компоненты
 * \return true, если найден корень компоненты; если компонента состоит из цикла - false
 */
bool findComponentRoot(Node* node, const QSet<Node*>& visited, Node*& outRoot);

/*!
 * \brief Проверяет граф на циклы и связность.
 * \param[in] root Указатель на корень (nullptr, если в ходе парсинга не было найдено корня)
 * \param[in,out] errors Набор ошибок
 * \param[in] allNodes Словарь всех узлов
 */
void validateStructure(Node* root, QSet<Error>& errors, const QMap<QString, Node*>& allNodes);

#endif // INPUTPROCESSING_H
