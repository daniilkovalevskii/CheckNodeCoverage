#include <QCoreApplication>
#include <coverageAnalysis.h>
#include <inputProcessing.h>
#include <outputGeneration.h>
#include <structures.h>
#include <QSet>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Проверка аргументов командной строки
    // Ожидаем: ./program <путь_к_входному_dot> <путь_к_выходному_dot> <путь_к_отчету_txt>
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
