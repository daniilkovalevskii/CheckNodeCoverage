#ifndef INPUTPROCESSING_H
#define INPUTPROCESSING_H
#include "structures.h"

/*!
 * \file inputProcessing.h
 * \brief Функции для чтения, парсинга и проверки дерева \c DOT.
 */

/*!
 * \brief Построчно считывает файл в массив строк.
 * \param[in] filepath Путь к входному файлу.
 * \param[out] errors Набор ошибок.
 * \return Массив строк с содержимым файла.
 *
 * \pre Путь к файлу \c filepath не должен быть пустым.
 * \post Возвращает считанные строки в формате \c QStringList. При ошибке доступа к файлу в контейнер \c errors записывается \c FILE_ERROR.
 */
QStringList readFile(const QString& filepath, QSet<Error>& errors);

/*!
 * \brief Программно строит дерево, описанное в массиве строк.
 * \param[in] lines Массив строк входного файла.
 * \param[out] errors Набор ошибок.
 * \return Структура с результатами парсинга (указатель на корень и словарь узлов дерева).
 *
 * \pre Контейнер \c lines должен содержать корректно считанные строки.
 * \post Возвращает заполненную структуру \c ParseResult. Если обнаружены критические синтаксические ошибки или запрещенные структуры, возвращаемая структура будет пустой, а в \c errors запишутся соответствующие типы ошибок.
 */
ParseResult parseDOT(const QStringList& lines, QSet<Error>& errors);

/*!
 * \brief Рекурсивная функция для поиска циклов в графе.
 * \param[in] node Узел, с которого начинается проверка.
 * \param[in,out] visited Множество уже посещенных узлов в ходе проверки на циклы.
 * \param[in,out] stack Стек для хранения пути цикла.
 * \param[out] errors Набор ошибок.
 *
 * \pre Указатель \c node должен быть валидным и не равным \c nullptr.
 * \post Обходит граф в глубину. При обнаружении цикла записывает ошибку \c CYCLE_ERROR в набор \c errors и формирует путь в \c stack.
 */
void dfsValidate(Node* node, QSet<Node*>& visited, QStack<Node*>& stack, QSet<Error>& errors);

/*!
 * \brief Ищет локальный корень компоненты.
 * \param[in] node Узел, с которого начинается поиск корня компоненты.
 * \param[in] visited Множество посещенных узлов в ходе проверки на циклы.
 * \param[out] outRoot Указатель на корень локальной компоненты.
 * \return \c true, если найден корень компоненты; если компонента состоит из цикла - \c false.
 *
 * \pre Указатель \c node не должен быть равен \c nullptr.
 * \post Записывает адрес найденного корня компоненты в \c outRoot, возвращая результат успешности поиска.
 */
bool findComponentRoot(Node* node, const QSet<Node*>& visited, Node*& outRoot);

/*!
 * \brief Проверяет граф на циклы и связность.
 * \param[in] root Указатель на корень (\c nullptr, если в ходе парсинга не было найдено корня).
 * \param[out] errors Набор ошибок.
 * \param[in] allNodes Словарь всех узлов.
 *
 * \pre Контейнер \c allNodes должен содержать все распарсенные узлы графа для проверки связности.
 * \post Граф полностью проверен на ошибки топологии (циклы, множественные родители, отсутствие корня). Все найденные несоответствия занесены в \c errors.
 */
void validateStructure(Node* root, QSet<Error>& errors, const QMap<QString, Node*>& allNodes);

#endif // INPUTPROCESSING_H
