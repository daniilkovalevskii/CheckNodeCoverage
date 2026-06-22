#include <QCoreApplication>
#include <coverageAnalysis.h>
#include <inputProcessing.h>
#include <outputGeneration.h>
#include <structures.h>
#include <QSet>
#include <QDebug>
#include <clocale>

/*!
 * \mainpage Программа для проверки покрытия набором узлов целевого узла
 *
 * \section intro_sec Введение
 * Программа предназначена для проверки покрытия набором узлов вышележащего узла дерева.
 * Дерево - это связный граф без циклов. Вышележащий узел - это любой узел, который находится на пути от текущего узла вверх к корню.
 * Узел дерева считается покрытым, если на каждом пути от этого узла до любого листа выбран хотя бы один узел.
 *
 * \section features_sec Основные возможности
 * - Валидация структуры: Обнаружение скрытых петель, циклов и изолированных компонент.
 * - Проверка покрытия: Рекурсивное выявление избыточных узлов (REDUNDANT_NODE и NOT_DESCENDANT) и точечный сбор дыр в покрытии.
 * - Визуализация: Генерация модифицированного графа с цветовой индикацией
 * (Зелёный - корректно выбранные узлы, выполняющие работу по покрытию целевого узла
 * /Оранжевый - недостающие узлы для полного покрытия
 * /Красный - избыточные узлы).
 *
 * \section usage_sec Инструкция по запуску
 * Запуск утилиты осуществляется через командную строку с передачей трёх обязательных аргументов:
 *
 * - Для ОС Windows (Командная строка или PowerShell):
 *   \code{.cmd}
 *   CheckNodeCoverage.exe <путь_к_входному_файлу.dot> <путь_к_выходному_файлу.dot> <путь_к_отчету.txt>
 *   \endcode
 *
 * - Для ОС macOS / Linux (Терминал):
 *   \code{.bash}
 *   ./CheckNodeCoverage <путь_к_входному_файлу.dot> <путь_к_выходному_файлу.dot> <путь_к_отчету.txt>
 *   \endcode
 *
 * \author Даниил Ковалевский
 * \sa Node, Error, Result, parseDOT(), validateStructure(), dfsAbove()
 */

int main(int argc, char *argv[])
{
    std::setlocale(LC_ALL, "Russian");

    QCoreApplication a(argc, argv);

    // Проверка аргументов командной строки
    if (argc < 4)
    {
        qCritical().noquote() << "Неверное количество аргументов.\n"
                                 "Использование: program <input.dot> <output.dot> <report.txt>";
        return 1;
    }

    QString inputPath  = argv[1];
    QString outputPath = argv[2];
    QString reportPath = argv[3];

    QSet<Error> errors;
    Result result;

    // Прочитать входной файл в список строк
    QStringList lines = readFile(inputPath, errors);

    // Если ошибок не обнаружено
    if (errors.isEmpty() && !lines.isEmpty())
    {
        // Выполнить парсинг
        ParseResult parsed = parseDOT(lines, errors);

        // Выполнить валидацию структуры графа
        // Запускаем проверку структуры только если парсер не стер граф из-за синтаксического сбоя
        if (parsed.root != nullptr || !parsed.allNodes.isEmpty())
        {
            validateStructure(parsed.root, errors, parsed.allNodes);
        }

        // Если нет ошибок, анализируем покрытие и формируем выходной .dot
        if (errors.isEmpty())
        {
            dfsAbove(parsed.root, errors, result);
            generateOutputDOT(outputPath, errors, lines, result);
        }

        // Формируем отчет
        generateReport(reportPath, errors, result);

        // Освобождаем память дерева
        qDeleteAll(parsed.allNodes);
        parsed.allNodes.clear();
        parsed.root = nullptr;
    }
    else
    {
        generateReport(reportPath, errors, result);
    }

    return 0;
}
