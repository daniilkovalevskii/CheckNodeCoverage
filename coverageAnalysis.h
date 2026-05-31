#ifndef COVERAGEANALYSIS_H
#define COVERAGEANALYSIS_H
#include "structures.h"


// Функция для обработки узлов выше целевого
void dfsAbove(Node* node, QSet<Error>& errors, Result& result);

// Функция для обработки узлов между целевым и листьями
CoverageStatus dfsSearching(Node* node, QSet<Error>& errors, Result& result);

// Функция для обработки узлов ниже уже отмеченных
void dfsCovered(Node* node, Node* activeMarked, QSet<Error>& errors);

#endif // COVERAGEANALYSIS_H
