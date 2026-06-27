#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/inputProcessing.h"
#include "../CheckNodeCoverage/structures.h"

class TestParseDot : public QObject
{
    Q_OBJECT

private slots:
    void testParseDOT_data()
    {
        QTest::addColumn<QString>("filePath");
        QTest::addColumn<QSet<Error>>("expectedErrors");
        QTest::addColumn<QVector<ExpectedNodeProfile>>("expectedTree");
        QTest::addColumn<QString>("expectedRootName");

        QSet<Error> noErrors = {};
        QTest::newRow("Тест 1: Нет открывающей скобки")
            << "testdata/parseTest01.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 1)} << QVector<ExpectedNodeProfile>{}
            << "";

        QTest::newRow("Тест 2: Нет закрывающей скобки")
            << "testdata/parseTest02.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 15)} << QVector<ExpectedNodeProfile>{}
            << "";

        QTest::newRow("Тест 3: В описании графа используются неподдерживаемые ключевые слова")
            << "testdata/parseTest03.dot" << QSet<Error>{Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "", "", "", 2),
                                                         Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "", "", "", 7),
                                                         Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "", "", "", 8)}
            << QVector<ExpectedNodeProfile>{{"b", NodeShape::DIAMOND, "", {"g", "h", "i"}},
                                            {"c", NodeShape::DEFAULT, "", {"e"}},
                                            {"d", NodeShape::DEFAULT, "", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::RECTANGLE, "d", {}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::RECTANGLE, "j", {}}}
            << "c";

        QTest::newRow("Тест 4: У узла несколько родителей")
            << "testdata/parseTest04.dot" << QSet<Error>{Error(ErrorType::MULTI_PARENT, "k", "j, i, g", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b", "c"}},
                                            {"b", NodeShape::DIAMOND, "a", {"g", "h", "i"}},
                                            {"c", NodeShape::DEFAULT, "a", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::RECTANGLE, "d", {}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::RECTANGLE, "j", {}}}
            << "a";

        QTest::newRow("Тест 5: Узел имеет неподдерживаемую форму")
            << "testdata/parseTest05.dot" << QSet<Error>{Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "a", "", "", 2), Error(ErrorType::NO_TARGET, "", "", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b"}},
                                            {"b", NodeShape::RECTANGLE, "a", {}}}
            << "a";

        QTest::newRow("Тест 6: В описанном графе есть несколько корней")
            << "testdata/parseTest06.dot" << noErrors
            << QVector<ExpectedNodeProfile>{{"c", NodeShape::DEFAULT, "", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::RECTANGLE, "d", {}},
                                            {"b", NodeShape::DIAMOND, "", {"g", "h", "i"}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::RECTANGLE, "j", {}}}
            << "c";

        QTest::newRow("Тест 7: Целевой узел не отмечен")
            << "testdata/parseTest07.dot" << QSet<Error>{Error(ErrorType::NO_TARGET, "", "", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b", "c"}},
                                            {"b", NodeShape::DEFAULT, "a", {"g", "h", "i"}},
                                            {"c", NodeShape::DEFAULT, "a", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::RECTANGLE, "d", {}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::RECTANGLE, "j", {}}}
            << "a";

        QTest::newRow("Тест 8: Отмечено несколько целевых узлов")
            << "testdata/parseTest08.dot" << QSet<Error>{Error(ErrorType::MULTIPLE_TARGETS, "b, c", "", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b", "c"}},
                                            {"b", NodeShape::DIAMOND, "a", {"g", "h", "i"}},
                                            {"c", NodeShape::DIAMOND, "a", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::RECTANGLE, "d", {}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::RECTANGLE, "j", {}}}
            << "a";

        QTest::newRow("Тест 9: Создание ребер цепочкой")
            << "testdata/parseTest09.dot" << noErrors
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b", "c"}},
                                            {"b", NodeShape::DIAMOND, "a", {"g", "h", "i"}},
                                            {"c", NodeShape::DEFAULT, "a", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::RECTANGLE, "d", {}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::RECTANGLE, "j", {}}}
            << "a";

        QTest::newRow("Тест 10: Создается пустой узел, а потом добавляется связь")
            << "testdata/parseTest10.dot" << noErrors
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b", "c"}},
                                            {"b", NodeShape::DIAMOND, "a", {"g", "h", "i"}},
                                            {"c", NodeShape::DEFAULT, "a", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::RECTANGLE, "d", {}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::RECTANGLE, "j", {}}}
            << "a";

        QTest::newRow("Тест 11: Нет отмеченных узлов (rectangle)")
            << "testdata/parseTest11.dot" << QSet<Error>{Error(ErrorType::NO_MARKED_NODES, "", "", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b", "c"}},
                                            {"b", NodeShape::DIAMOND, "a", {"g", "h", "i"}},
                                            {"c", NodeShape::DEFAULT, "a", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::DEFAULT, "d", {}},
                                            {"g", NodeShape::DEFAULT, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::DEFAULT, "b", {}},
                                            {"j", NodeShape::DEFAULT, "h", {"k"}},
                                            {"k", NodeShape::DEFAULT, "j", {}}}
            << "a";

        QTest::newRow("Тест 12: Отсутствует точка с запятой в конце строки с объявлением ребра")
            << "testdata/parseTest12.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 2)} << QVector<ExpectedNodeProfile>{} << "";

        QTest::newRow("Тест 13: Отсутствует точка с запятой в конце строки с объявлением узла")
            << "testdata/parseTest13.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 2)}
            << QVector<ExpectedNodeProfile>{} << "";

        QTest::newRow("Тест 14: Есть пробел между частями стрелки при объявлении связи")
            << "testdata/parseTest14.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 2)} << QVector<ExpectedNodeProfile>{} << "";

        QTest::newRow("Тест 15: Несколько ошибок разного типа")
            << "testdata/parseTest15.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 4),
                                                         Error(ErrorType::SYNTAX_ERROR, "", "", "", 15)}
            << QVector<ExpectedNodeProfile>{} << "";

        QTest::newRow("Тест 16: Целевой узел — лист")
            << "testdata/parseTest16.dot" << QSet<Error>{Error(ErrorType::LEAF_TARGET, "a", "", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DIAMOND, "b", {}},
                                            {"b", NodeShape::RECTANGLE, "", {"a"}}}
            << "b";;

        QTest::newRow("Тест 17: Пустой граф")
            << "testdata/parseTest17.dot" << QSet<Error>{Error(ErrorType::NO_TARGET, "", "", "", -1),
                                                         Error(ErrorType::NO_MARKED_NODES, "", "", "", -1)}
            << QVector<ExpectedNodeProfile>{} << "";

        QTest::newRow("Тест 18: Форма узла объявляется после объявления ребра с его участием")
            << "testdata/parseTest18.dot" << noErrors
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DIAMOND, "", {"b"}},
                                            {"b", NodeShape::RECTANGLE, "a", {}}}
            << "a";

        QTest::newRow("Тест 19: Цикл")
            << "testdata/parseTest19.dot" << noErrors
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DIAMOND, "c", {"b"}},
                                            {"b", NodeShape::RECTANGLE, "a", {"c"}},
                                            {"c", NodeShape::RECTANGLE, "b", {"a"}}}
            << "";

        QTest::newRow("Тест 20: Корректное дерево")
            << "testdata/parseTest20.dot" << noErrors
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b", "c"}},
                                            {"b", NodeShape::DIAMOND, "a", {"g", "h", "i"}},
                                            {"c", NodeShape::DEFAULT, "a", {"d", "e"}},
                                            {"d", NodeShape::DEFAULT, "c", {"f"}},
                                            {"e", NodeShape::DEFAULT, "c", {}},
                                            {"f", NodeShape::DEFAULT, "d", {}},
                                            {"g", NodeShape::RECTANGLE, "b", {}},
                                            {"h", NodeShape::DEFAULT, "b", {"j"}},
                                            {"i", NodeShape::RECTANGLE, "b", {}},
                                            {"j", NodeShape::RECTANGLE, "h", {"k"}},
                                            {"k", NodeShape::DEFAULT, "j", {}}}
            << "a";

        QTest::newRow("Тест 21: Открывающая фигурная скобка находится не на одной строке с digraph")
            << "testdata/parseTest21.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 1)}
            << QVector<ExpectedNodeProfile>{}
            << "";

        QTest::newRow("Тест 22: Дублирование ребра связи")
            << "testdata/parseTest22.dot" << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 3),
                                                         Error(ErrorType::NO_TARGET, "", "", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b"}},
                                            {"b", NodeShape::RECTANGLE, "a", {}}}
            << "a";

        QTest::newRow("Тест 23: Есть квадратные скобки, но нет атрибута shape")
            << "testdata/parseTest23.dot" << QSet<Error>{Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "a", "", "", 2),
                                                         Error(ErrorType::NO_TARGET, "", "", "", -1)}
            << QVector<ExpectedNodeProfile>{{"a", NodeShape::DEFAULT, "", {"b"}},
                                            {"b", NodeShape::RECTANGLE, "a", {}}}
            << "a";

        QTest::newRow("Тест 24: Абсолютно пустой файл")
            << "testdata/parseTest24.dot" << QSet<Error>{}
            << QVector<ExpectedNodeProfile>{}
            << "";

        QTest::newRow("Тест 25: Избыточные закрывающие скобки подграфа")
            << "testdata/parseTest25.dot" << QSet<Error>{Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "", "", "", 2)}
            << QVector<ExpectedNodeProfile>{}
            << "";
    }

    void testParseDOT()
    {
        QFETCH(QString, filePath);
        QFETCH(QSet<Error>, expectedErrors);
        QFETCH(QVector<ExpectedNodeProfile>, expectedTree);
        QFETCH(QString, expectedRootName);

        QStringList lines = readDotFile(filePath);
        QSet<Error> actualErrors;
        ParseResult result = parseDOT(lines, actualErrors);

        bool testPassed = true;
        QStringList finalFailureMessages;

        // Сверяем ошибки
        if (actualErrors != expectedErrors)
        {
            showErrorLog(actualErrors, expectedErrors);
            finalFailureMessages << "Ошибка: Набор ошибок не совпадает с ожидаемым";
            testPassed = false;
        }

        // Сверяем структуру дерева
        if (result.root != nullptr)
        {
            // Проверяем, совпадает ли имя возвращённого корня с эталоном
            if (result.root->name != expectedRootName)
            {
                finalFailureMessages << QString("Ошибка корня: Ожидался корень '%1', но парсер вернул '%2'")
                                            .arg(expectedRootName)
                                            .arg(result.root->name);
                testPassed = false;
            }

            // Проверяем идентичность всей остальной структуры узлов
            QString treeError;
            bool matches = treeMatchesExpectedProfile(result.allNodes, expectedTree, treeError);
            if (!matches)
            {
                qWarning().noquote() << "--- ОШИБКА СТРУКТУРЫ ДЕРЕВА ---";
                qWarning().noquote() << treeError;
                finalFailureMessages << "Ошибка структуры: " + treeError;
                testPassed = false;
            }
        }
        else
        {
            // Если корень отсутствует, проверяем, ожидался ли он
            if (!expectedRootName.isEmpty())
            {
                finalFailureMessages << "Ошибка: Ожидался корень '" + expectedRootName + "'";
                testPassed = false;
            }
        }

        // Если дерево не построено, хотя ожидалось
        if (!expectedTree.isEmpty() && result.allNodes.isEmpty())
        {
            finalFailureMessages << "Ошибка: Парсер не построил дерево, хотя ожидалось";
            testPassed = false;
        }

        // Проверка на мусор в allNodes при пустом корне
        if (result.root == nullptr && !result.allNodes.isEmpty() && !expectedRootName.isEmpty())
        {
            finalFailureMessages << QString("Ошибка: root равен nullptr, но карта allNodes содержит %1 узлов")
                                        .arg(result.allNodes.size());
            testPassed = false;
        }

        // Очистка памяти
        qDeleteAll(result.allNodes);

        // Выводим все собранные ошибки
        if (!testPassed)
        {
            QFAIL(qPrintable(finalFailureMessages.join("\n")));
        }
    }
};

#include "testParseDOT.moc"
