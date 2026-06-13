#include "outputGeneration.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSet>

bool generateReport(const QString& outPath, const QSet<Error>& errors, const Result& result)
{
    // Открыть выходной файл для записи
    QFile file(outPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        // Если открыть не удалось - вывести ошибку в консоль как критический системный сбой
        qCritical().noquote() << "Неверно указан файл для выходных данных(отчет). Возможно указанного расположения не существует или нет прав на запись.\n"
                                 "Путь к файлу отчета:" << outPath;
        return false;
    }

    QTextStream out(&file);

    // Если result.valid пуст и result.missing пуст(это значит, что из-за критических синтаксических ошибок граф вообще не был построен и проанализирован)
    if (result.valid.isEmpty() && result.missing.isEmpty())
    {
        // Для каждой ошибки создать сообщение
        for (const Error& err : errors)
        {
            out << err.generateMessage() << "\n";
        }
    }
    // Иначе
    else
    {
        // Если result.missing пуст:
        if (result.missing.isEmpty())
        {
            out << "Целевой узел ПОКРЫТ\n";
        }
        // Иначе:
        else
        {
            out << "Целевой узел НЕ ПОКРЫТ\n";

            // Собираем имена недостающих узлов через запятую
            QStringList missingNames;
            for (Node* node : result.missing)
            {
                if (node != nullptr)
                {
                    missingNames.append(node->name);
                }
            }
            // Записываем недостающие узлы
            out << "Недостающие узлы: " << missingNames.join(", ") << "\n";
        }

        // Если среди errors есть ошибки NOT_DESCENDANT или REDUNDANT_NODE тоже их записываем
        for (const Error& err : errors)
        {
            if (err.type == ErrorType::NOT_DESCENDANT || err.type == ErrorType::REDUNDANT_NODE)
            {
                out << err.generateMessage() << "\n";
            }
        }
    }

    // Закрыть файл
    file.close();
    return true;
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
    return true;
}
