#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/coverageAnalysis.h"
#include "../CheckNodeCoverage/inputProcessing.h"
#include "../CheckNodeCoverage/structures.h"

class TestDfsAbove : public QObject
{
    Q_OBJECT;

private slots:
    void testDfsAbove_data()
    {
        QTest::addColumn<QString>("filePath");
        QTest::addColumn<QString>("startNodeName");
        QTest::addColumn<QStringList>("expValidNodeNames");
        QTest::addColumn<QStringList>("expMissingNodeNames");
        QTest::addColumn<QSet<Error>>("expErrors");

        QSet<Error> noErrors;
        QStringList noValid;
        QStringList noMissing;

        QTest::newRow("Тест 1: Целевой узел является корнем") << "testData/dfsAboveTest01.dot" << "target"
                                                                 << QStringList{"c"} << QStringList{"b"} << noErrors;

        QTest::newRow("Тест 2: Целевой узел не является корнем, между ним и корнем нет отмеченных")
            << "testData/dfsAboveTest02.dot" << "root"
            << QStringList{"c"} << QStringList{"b"} << noErrors;

        QTest::newRow("Тест 3: Есть отмеченный узел между корнем и целевым узлом")
            << "testData/dfsAboveTest03.dot" << "root"
            << QStringList{"c"} << QStringList{"b"} << QSet<Error>{Error(ErrorType::NOT_DESCENDANT, "a")};

        QTest::newRow("Тест 4: Есть несколько отмеченных узлов между корнем и целевым узлом")
            << "testData/dfsAboveTest04.dot" << "root"
            << QStringList{"c"} << QStringList{"b"} << QSet<Error>{Error(ErrorType::NOT_DESCENDANT, "a"),
                                                                   Error(ErrorType::NOT_DESCENDANT, "d"),
                                                                   Error(ErrorType::NOT_DESCENDANT, "e")};

        QTest::newRow("Тест 5: Есть отмеченные узлы вне поддерева, где содержится целевой")
            << "testData/dfsAboveTest05.dot" << "root"
            << QStringList{"c"} << QStringList{"b"} << QSet<Error>{Error(ErrorType::NOT_DESCENDANT, "a"),
                                                                   Error(ErrorType::NOT_DESCENDANT, "d"),
                                                                   Error(ErrorType::NOT_DESCENDANT, "e")};

        QTest::newRow("Тест 6: Комплексный тест 1")
            << "testData/dfsAboveTest06.dot" << "root"
            << QStringList{"b", "d"} << noMissing << QSet<Error>{Error(ErrorType::NOT_DESCENDANT, "a"),
                                                                 Error(ErrorType::REDUNDANT_NODE, "g", "b"),
                                                                 Error(ErrorType::REDUNDANT_NODE, "j", "d")};

        QTest::newRow("Тест 7: Комплексный тест 2")
            << "testData/dfsAboveTest07.dot" << "root"
            << QStringList{"c", "n"} << QStringList{"d", "e", "m"} << QSet<Error>{Error(ErrorType::NOT_DESCENDANT, "q")};

        QTest::newRow("Тест 8: Комплексный тест 3")
            << "testData/dfsAboveTest08.dot" << "root"
            << QStringList{"c", "n", "j", "k"} << QStringList{"d", "m"} << QSet<Error>{Error(ErrorType::NOT_DESCENDANT, "q")};
    }

    void testDfsAbove()
    {
        QFETCH(QString, filePath);
        QFETCH(QString, startNodeName);
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

        Node* startNodePtr = res.allNodes.value(startNodeName);
        if (!startNodePtr)
        {
            msgs << QString("Не найдено переданного узла %1 в графе").arg(startNodeName);
            testPassed = false;
        }
        else
        {
            Result ans;
            dfsAbove(startNodePtr, actualErrors, ans);

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

#include "testDfsAbove.moc"
