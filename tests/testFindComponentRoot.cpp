#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/structures.h"
#include "../CheckNodeCoverage/inputProcessing.h"

typedef QVector<QPair<QString, QString>> EdgeList;
Q_DECLARE_METATYPE(EdgeList)

class FindComponentRoot : public QObject
{
    Q_OBJECT;

private slots:
    void testFindComponentRoot_data()
    {
        QTest::addColumn<EdgeList>("edges");
        QTest::addColumn<QString>("startNodeName");
        QTest::addColumn<QString>("expectedRootName");
        QTest::addColumn<bool>("result");

        QTest::newRow("Тест 1: Переданный узел является листом компоненты") << EdgeList{{"a", "b"}, {"b", "c"}}
                                                                            << "c" << "a" << true;

        QTest::newRow("Тест 2: Переданный узел не является ни листом, ни корнем компоненты") << EdgeList{{"a", "b"}, {"b", "c"}}
                                                                                             << "b" << "a" << true;

        QTest::newRow("Тест 3: Переданный узел является корнем компоненты") << EdgeList{{"a", "b"}, {"b", "c"}}
                                                                            << "a" << "a" << true;

        QTest::newRow("Тест 4: Компонента состоит из цикла") << EdgeList{{"a", "b"}, {"b", "c"}, {"c", "a"}}
                                                             << "a" << "a" << false;

        QTest::newRow("Тест 5: Компонента состоит из длинной цепочки") << EdgeList{{"a", "b"}, {"b", "c"}, {"c", "d"},
                                                                                                           {"d", "e"}, {"e", "f"}, {"f", "g"}, {"g", "h"}}
                                                                       << "g" << "a" << true;
        QTest::newRow("Тест 6: Разветвленное дерево (запуск из глубокого листа)") << EdgeList{{"root", "left"}, {"root", "right"}, {"left", "leaf1"}, {"left", "leaf2"}}
                                                                                  << "leaf2" << "root" << true;
        QTest::newRow("Тест 7: Дерево переходящее в цикл (вход в петлю)") << EdgeList{{"a", "b"}, {"b", "c"}, {"c", "d"}, {"d", "b"}}
                                                                          << "a" << "a" << true;
    }

    void testFindComponentRoot()
    {
        QFETCH(EdgeList, edges);
        QFETCH(QString, startNodeName);
        QFETCH(QString, expectedRootName);
        QFETCH(bool, result); // True — ожидаем корень, False — ожидаем узел в цикле

        QMap<QString, Node*> nodes;
        buildTree(edges, nodes);

        Node* startNode = nodes.value(startNodeName);

        // Флаги для накопления результатов проверки
        bool testPassed = true;
        QString failureMessage;

        if (startNode == nullptr)
        {
            testPassed = false;
            failureMessage = "Стартовый узел '" + startNodeName + "' не найден в построенном графе!";
        }
        else
        {
            QSet<Node*> visited;
            Node* actualRoot = nullptr;

            // Запускаем алгоритм
            bool actualResult = findComponentRoot(startNode, visited, actualRoot);

            if (actualRoot == nullptr)
            {
                testPassed = false;
                failureMessage = "Алгоритм вернул actualRoot == nullptr!";
            }
            else if (actualRoot->name != expectedRootName)
            {
                testPassed = false;
                failureMessage = QString("Неверный корень! Ожидался '%1', получен '%2'")
                                     .arg(expectedRootName).arg(actualRoot->name);
            }
            else if (actualResult != result)
            {
                testPassed = false;
                failureMessage = QString("Возвращенный флаг (bool) не совпадает! Ожидался %1, получен %2")
                                     .arg(result).arg(actualResult);
            }
        }

        // Очистка памяти
        for (Node* node : nodes)
        {
            if (node)
            {
                node->children.clear();
                node->parent = nullptr;
            }
        }
        qDeleteAll(nodes);

        if (!testPassed)
        {
            QFAIL(qPrintable(failureMessage));
        }
    }
};

#include "testFindComponentRoot.moc"
