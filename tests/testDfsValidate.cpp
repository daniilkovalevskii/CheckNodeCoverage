#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/inputProcessing.h"
#include "../CheckNodeCoverage/structures.h"

class TestDfsValidate : public QObject
{
    Q_OBJECT;

private slots:
    void testDfsValidate_data()
    {
        QTest::addColumn<EdgeList>("edges");
        QTest::addColumn<QString>("startNode");
        QTest::addColumn<QSet<Error>>("expErrors");

        QSet<Error> noErrors;

        QTest::newRow("Тест 1: Дерево состоит из одного узла") << EdgeList{} << "a" << noErrors;

        QTest::newRow("Тест 2: Дерево состоит из длинной цепочки") << EdgeList{{"a", "b"},
                                                                               {"b", "c"},
                                                                               {"c", "d"},
                                                                               {"d", "e"},
                                                                               {"e", "f"}}
                                                                   << "a"
                                                                   << noErrors;

        QTest::newRow("Тест 3: Дерево с ветвлением") << EdgeList{{"a", "b"},
                                                                 {"b", "c"},
                                                                 {"a", "d"},
                                                                 {"d", "e"},
                                                                 {"d", "f"}}
                                                     << "a"
                                                     << noErrors;

        QTest::newRow("Тест 4: Есть цикл из двух узлов") << EdgeList{{"a", "b"},
                                                                     {"b", "a"}}
                                                         << "a"
                                                         << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->b->a", -1)};

        QTest::newRow("Тест 5: Есть цикл из трех узлов") << EdgeList{{"a", "b"},
                                                                     {"b", "c"},
                                                                     {"c", "a"}}
                                                         << "a"
                                                         << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->b->c->a", -1)};

        QTest::newRow("Тест 6: Узел ссылается сам на себя") << EdgeList{{"a", "a"}}
                                                            << "a"
                                                            << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->a", -1)};

        QTest::newRow("Тест 7: Длинный цикл") << EdgeList{{"a", "b"},
                                                          {"b", "c"},
                                                          {"c", "d"},
                                                          {"d", "e"},
                                                          {"e", "f"},
                                                          {"f", "g"},
                                                          {"g", "h"},
                                                          {"h", "i"},
                                                          {"i", "j"},
                                                          {"j", "a"}}
                                              << "a"
                                              << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->b->c->d->e->f->g->h->i->j->a", -1)};

        QTest::newRow("Тест 8: Граф с ветвлением содержит цикл") << EdgeList{{"a", "b"},
                                                                               {"b", "c"},
                                                                               {"a", "d"},
                                                                               {"d", "e"},
                                                                               {"d", "f"},
                                                                               {"f", "g"},
                                                                               {"g", "h"},
                                                                               {"h", "i"},
                                                                               {"h", "j"},
                                                                               {"h", "k"},
                                                                               {"j", "l"},
                                                                               {"l", "a"}}
                                                     << "a"
                                                     << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->d->f->g->h->j->l->a", -1)};
    }

    void testDfsValidate()
    {
        QFETCH(EdgeList, edges);
        QFETCH(QString, startNode);
        QFETCH(QSet<Error>,expErrors);

        // Строим дерево
        QMap<QString, Node*> nodes;
        if (edges.isEmpty())
            nodes.insert(startNode, new Node(startNode));
        else
            buildTree(edges, nodes);

        Node* startNodePtr = nodes.value(startNode);
        bool testPassed = true;
        QStringList msgs;

        if (startNodePtr == nullptr)
        {
            testPassed = false;
            msgs << QString("Ошибка в тесте: startNode = nullptr, не удалось найти узел %1 в графе, построенном из заданных связей").arg(startNode);
        }
        else
        {
            QSet<Node*> visited;
            QStack<Node*> stack;
            QSet<Error> actualErrors;

            dfsValidate(startNodePtr, visited, stack, actualErrors);
            if (actualErrors != expErrors)
            {
                showErrorLog(actualErrors, expErrors);
                msgs << "Набор полученных ошибок не совпадает с ожидаемым";
                testPassed = false;
            }
        }

        for (Node* node : nodes)
        {
            if (node)
            {
                node -> children.clear();
                node -> parent = nullptr;
            }
        }
        qDeleteAll(nodes);

        if (!testPassed)
            QFAIL(qPrintable(msgs.join("\n")));
    }
};

#include "testDfsValidate.moc"
