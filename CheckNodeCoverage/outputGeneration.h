#ifndef OUTPUTGENERATION_H
#define OUTPUTGENERATION_H
#include "structures.h"


// Функция для создания файла с отчетом
bool generateReport(const QString& outPath, const QSet<Error>& errors, const Result& result);

// Функция для создания выходного файла .dot
bool generateOutputDOT(const QString& outPath, QSet<Error>& errors, QStringList& lines, const Result& result);

#endif // OUTPUTGENERATION_H
