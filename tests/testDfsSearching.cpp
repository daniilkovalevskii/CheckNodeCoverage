#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/coverageAnalysis.h"
#include "../CheckNodeCoverage/inputProcessing.h"
#include "../CheckNodeCoverage/structures.h"

class TestDfsSearching : public QObject
{
    Q_OBJECT;

private slots:
    void testDfsSearching_data()
    {
        QTest::addColumn<QString>("filePath");
        QTest::addColumn<QString>("targetNodeName");
        QTest::addColumn<QStringList>("expValidNodeNames");
        QTest::addColumn<QStringList>("expMissingNodeNames");
        QTest::addColumn<QSet<Error>>("expErrors");

        QSet<Error> noErrors;
        QStringList noValid;
        QStringList noMissing;
        QTest::newRow("Тест 1: Все дети целевого узла отмечены") << "testData/dfsSearchingTest01.dot" << "target"
                                                                 << QStringList{"b", "c"} << noMissing << noErrors;

        QTest::newRow("Тест 2: Минимальное покрытие обеспечивается добавлением одного узла") << "testData/dfsSearchingTest02.dot" << "target"
                                                                                             << noValid << QStringList{"b"}
                                                                                             << QSet<Error>{Error(ErrorType::NO_MARKED_NODES)};

        QTest::newRow("Тест 3: Несколько вариантов для минимального покрытия(выбираем ближайший к целевому)") << "testData/dfsSearchingTest03.dot" << "target"
                                                                                                              << noValid << QStringList{"b"} << QSet<Error>{Error(ErrorType::NO_MARKED_NODES)};

        QTest::newRow("Тест 4: Целевой узел частично покрыт") << "testData/dfsSearchingTest04.dot" << "target"
                                                              << QStringList{"c"} << QStringList{"b"} << noErrors;

        QTest::newRow("Тест 5: Отмеченный узел - лист") << "testData/dfsSearchingTest05.dot" << "target"
                                                        << QStringList{"c"} << noMissing << noErrors;

        QTest::newRow("Тест 6: Целевой узел частично покрыт, минимальное покрытие обеспечивается добавлением одного узла")
            << "testData/dfsSearchingTest06.dot" << "target"
            << QStringList{"c"} << QStringList{"b"} << noErrors;

        QTest::newRow("Тест 7: В поддереве есть несколько частично покрытых узлов") << "testData/dfsSearchingTest07.dot" << "target"
                                                                                    << QStringList{"c", "e", "g"}
                                                                                    << QStringList{"f"} << noErrors;

        QTest::newRow("Тест 8: Непокрыто большое поддерево, минимальное покрытие обеспечивается добавлением одного узла")
            << "testData/dfsSearchingTest08.dot" << "target"
            << QStringList{"c"}
            << QStringList{"b"} << noErrors;

        QTest::newRow("Тест 9: Поддерево содержит частично покрытый узел, у которого есть полностью непокрытые дети и один покрытый ребенок")
            << "testData/dfsSearchingTest09.dot" << "target"
            << QStringList{"c", "n"}
            << QStringList{"d", "e", "m"} << noErrors;

        QTest::newRow("Тест 10: Поддерево содержит частично покрытый узел, у которого есть полностью непокрытые дети и несколько покрытых детей")
            << "testData/dfsSearchingTest10.dot" << "target"
            << QStringList{"c", "n", "j", "k"}
            << QStringList{"d", "m"} << noErrors;

        QTest::newRow("Тест 11: Есть отмеченный узел в поддереве уже отмеченного")
            << "testData/dfsSearchingTest11.dot" << "target"
            << QStringList{"d"}
            << QStringList{"b"} << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "e", "d", "", -1)};

        QTest::newRow("Тест 12: Есть несколько отмеченных узлов в поддереве уже отмеченного")
            << "testData/dfsSearchingTest12.dot" << "target"
            << QStringList{"d"}
            << QStringList{"b"} << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "e", "d", "", -1),
                                               Error(ErrorType::REDUNDANT_NODE, "f", "d", "", -1),
                                               Error(ErrorType::REDUNDANT_NODE, "g", "d", "", -1)};

        QTest::newRow("Тест 13: Есть отмеченные узлы в поддеревьях разных уже отмеченных узлов")
            << "testData/dfsSearchingTest13.dot" << "target"
            << QStringList{"b", "d"}
            << noMissing << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "g", "b", "", -1),
                                        Error(ErrorType::REDUNDANT_NODE, "j", "d", "", -1)};
    }

    void testDfsSearching()
    {
        QFETCH(QString, filePath);
        QFETCH(QString, targetNodeName);
        QFETCH(QStringList, expValidNodeNames);
        QFETCH(QStringList, expMissingNodeNames);
        QFETCH(QSet<Error>, expErrors);

        bool testPassed = true;
        QStringList msgs;

        QStringList lines = readDotFile(filePath);
        if (lines.isEmpty())
        {
            QFAIL(qPrintable(QString("Не удалось прочитать или файл пуст: %1").arg(filePath)));
            return;
        }

        QSet<Error> actualErrors;
        ParseResult res = parseDOT(lines, actualErrors);

        Node* targetPtr = res.allNodes.value(targetNodeName);
        if (!targetPtr)
        {
            msgs << QString("Не найдено переданного отмеченного узла %1 в графе").arg(targetNodeName);
            testPassed = false;
        }
        else
        {
            Result ans;
            dfsSearching(targetPtr, actualErrors, ans);

            QStringList actualValidNames;
            for (Node* n : ans.valid)
            {
                if (n)
                    actualValidNames.append(n->name);
            }

            QStringList actualMissingNames;
            for (Node* n : ans.missing)
            {
                if (n)
                    actualMissingNames.append(n->name);
            }

            actualValidNames.sort();
            actualMissingNames.sort();
            expValidNodeNames.sort();
            expMissingNodeNames.sort();

            // Проверяем валидные узлы
            if (actualValidNames != expValidNodeNames)
            {
                msgs << QString("Несовпадение валидных узлов. Ожидалось: [%1], Получено: [%2]")
                            .arg(expValidNodeNames.join(", "), actualValidNames.join(", "));
                testPassed = false;
            }

            // Проверяем недостающие узлы для покрытия
            if (actualMissingNames != expMissingNodeNames)
            {
                msgs << QString("Несовпадение недостающих узлов покрытия. Ожидалось: [%1], Получено: [%2]")
                            .arg(expMissingNodeNames.join(", "), actualMissingNames.join(", "));
                testPassed = false;
            }

            // Проверяем наборы ошибок
            if (actualErrors != expErrors)
            {
                showErrorLog(actualErrors, expErrors);
                msgs << "Полученный набор ошибок избыточности покрытия не совпадает с ожидаемым";
                testPassed = false;
            }
        }

        // Очистка памяти
        for (Node* node : res.allNodes)
        {
            if (node)
            {
                node->children.clear();
                node->parent = nullptr;
            }
        }
        qDeleteAll(res.allNodes);

        if (!testPassed)
        {
            QFAIL(qPrintable(msgs.join("\n")));
        }
    }
};

#include "testDfsSearching.moc"
