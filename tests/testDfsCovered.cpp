#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/coverageAnalysis.h"
#include "../CheckNodeCoverage/inputProcessing.h"
#include "../CheckNodeCoverage/structures.h"

class TestDfsCovered : public QObject
{
    Q_OBJECT;

private slots:
    void testDfsCovered_data()
    {
        QTest::addColumn<QString>("filePath");
        QTest::addColumn<QString>("activeMarkedNode");
        QTest::addColumn<QSet<Error>>("expErrors");

        QSet<Error> noErrors;
        QTest::newRow("Тест 1: Переданный отмеченный узел не имеет детей") << QString("testData/dfsCoveredTest01.dot")
                                                                           << "a"
                                                                           << noErrors;

        QTest::newRow("Тест 2: У отмеченного узла есть неотмеченные дети") << QString("testData/dfsCoveredTest02.dot")
                                                                           << "a"
                                                                           << noErrors;

        QTest::newRow("Тест 3: У отмеченного узла есть отмеченный ребенок") << QString("testData/dfsCoveredTest03.dot")
                                                                            << "a"
                                                                            << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "b", "a", "", -1)};

        QTest::newRow("Тест 4: У отмеченного узла есть несколько отмеченных детей") << QString("testData/dfsCoveredTest04.dot")
                                                                                    << "a"
                                                                                    << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "b", "a", "", -1),
                                                                                                   Error(ErrorType::REDUNDANT_NODE, "c", "a", "", -1)};

        QTest::newRow("Тест 5: Отмеченный узел глубоко в поддереве") << QString("testData/dfsCoveredTest05.dot")
                                                                     << "a"
                                                                     << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "g", "a", "", -1)};

        QTest::newRow("Тест 6: Цепочка отмеченных узлов") << QString("testData/dfsCoveredTest06.dot")
                                                          << "a"
                                                          << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "c", "a", "", -1),
                                                                         Error(ErrorType::REDUNDANT_NODE, "d", "a", "", -1),
                                                                         Error(ErrorType::REDUNDANT_NODE, "e", "a", "", -1)};

        QTest::newRow("Тест 7: Цепочка отмеченных узлов") << QString("testData/dfsCoveredTest07.dot")
                                                          << "a"
                                                          << QSet<Error>{Error(ErrorType::REDUNDANT_NODE, "m", "a", "", -1),
                                                                         Error(ErrorType::REDUNDANT_NODE, "j", "a", "", -1)};
    }

    void testDfsCovered()
    {
        QFETCH(QString, filePath);
        QFETCH(QString, activeMarkedNode);
        QFETCH(QSet<Error>,expErrors);

        QStringList lines = readDotFile(filePath);
        QSet<Error> actualErrors;
        ParseResult parsed = parseDOT(lines, actualErrors);

        bool testPassed = true;
        QStringList msgs;

        if (actualErrors.isEmpty())
        {
            Node* activeMarkedPtr = parsed.allNodes.value(activeMarkedNode);

            if (activeMarkedPtr != nullptr)
            {
                dfsCovered(activeMarkedPtr, activeMarkedPtr, actualErrors);
            }

            else
            {
                msgs << QString("Не найдено переданного отмеченного узла %1 в графе").arg(activeMarkedNode);
                testPassed = false;
            }
        }

        if (actualErrors != expErrors)
        {
            showErrorLog(actualErrors, expErrors);
            msgs << "Полученный набор ошибок не совпадает с ожидаемым";
            testPassed = false;
        }

        for (Node* node : parsed.allNodes)
        {
            if (node)
            {
                node->children.clear();
                node->parent = nullptr;
            }
        }
        qDeleteAll(parsed.allNodes);

        if (!testPassed)
        {
            QFAIL(qPrintable(msgs.join("\n")));
        }
    }
};

#include "testDfsCovered.moc"
