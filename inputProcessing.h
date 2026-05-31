#ifndef INPUTPROCESSING_H
#define INPUTPROCESSING_H
#include "structures.h"


// Функция для парсинга входного файла .dot
ParseResult parseDOT(const QStringList& lines, QSet<Error>& errors);

// Рекурсивная функция для проверки циклов в переданном графе
void dfsValidate(Node* node, QSet<Node*>& visited, QStack<Node*>& stack, QSet<Error>& errors);

// Функция для поиска локального корня компоненты
bool findComponentRoot(Node* node, const QSet<Node*>& visited, Node*& outRoot);

// Функция для проверки корректности переданного графа
void validateStructure(Node* root, QSet<Error>& errors, const QMap<QString, Node*>& allNodes);

#endif // INPUTPROCESSING_H
