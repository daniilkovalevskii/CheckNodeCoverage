#include "outputGeneration.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSet>

bool generateReport(const QString& outPath, const QSet<Error>& errors, const Result& result)
{
    return false;
}

bool generateOutputDOT(const QString& outPath, QSet<Error>& errors, QStringList& lines, const Result& result)
{
    // Открыть выходной файл для записи
    QFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Пишем в консоль
        qCritical().noquote() << "Неверно указан файл для выходных данных. Возможно указанного расположения не существует или нет прав на запись.\n"
                                 "Путь:" << outPath;

        // Заносим в массив ошибок для последующей передачи в репорт
        errors.insert(Error(ErrorType::OUTPUT_FILE_ERROR, "", "", outPath, -1));
        return false;
    }

    QTextStream out(&file);

    // Записываем все строки исходного файла кроме последней закрывающей скобки
    for (int i = 0; i < lines.size() - 1; ++i)
    {
        out << lines.at(i) << "\n";
    }

    out << "\n";

    // Для каждого узла из списка недостающих дописать объявление с оранжевым цветом
    for (Node* node : result.missing)
    {
        if (node != nullptr)
        {
            out << QString("    %1 [color=\"orange\", style=\"filled\"];\n").arg(node->name);
        }
    }

    // Для каждой ошибки NOT_DESCENDANT и REDUNDANT_NODE дописать объявление с красным цветом
    for (const Error& err : errors)
    {
        if (err.type == ErrorType::NOT_DESCENDANT || err.type == ErrorType::REDUNDANT_NODE)
        {
            out << QString("    %1 [color=\"red\", style=\"filled\"];\n").arg(err.nodeName);
        }
    }

    // Для каждого узла из списка корректных дописать объявление с зелёным цветом
    for (Node* node : result.valid)
    {
        if (node != nullptr)
        {
            out << QString("    %1 [color=\"green\", style=\"filled\"];\n").arg(node->name);
        }
    }

    // Записать закрывающую скобку
    out << "}\n";

    // Закрыть файл
    file.close();
}
