#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/inputProcessing.h"
#include "../CheckNodeCoverage/structures.h"

class TestValidateStructure : public QObject
{
    Q_OBJECT;

private slots:
    void testValidateStructure_data()
    {
        QTest::addColumn<EdgeList>("edges");
        QTest::addColumn<QSet<Error>>("expErrors");
        QTest::addColumn<QString>("rootName");

        QSet<Error> noErrors;

        QTest::newRow("Тест 1: Задано корректное дерево") << EdgeList{{"a", "b"},
                                                                      {"b", "c"},
                                                                      {"b", "d"}}
                                                          << noErrors
                                                          << "a";

        QTest::newRow("Тест 2: Есть цикл в поддереве") << EdgeList{{"a", "b"},
                                                                  {"b", "c"},
                                                                  {"b", "d"},
                                                                  {"d", "e"},
                                                                  {"e", "a"}}
                                                      << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->b->d->e->a", -1),
                                                                     Error(ErrorType::NO_ROOT)}
                                                      << "";

        QTest::newRow("Тест 3: Граф содержит несвязную компоненту без цикла") << EdgeList{{"a", "b"},
                                                                                          {"b", "c"},
                                                                                          {"b", "d"},
                                                                                          {"e", "f"}}
                                                                              << QSet<Error>{Error(ErrorType::CONNECTIVITY_ERROR, "a", "e", "", -1)}
                                                                              << "a";

        QTest::newRow("Тест 4: Граф содержит несвязную компоненту, состоящую из цикла") << EdgeList{{"a", "b"},
                                                                                                    {"b", "c"},
                                                                                                    {"b", "d"},
                                                                                                    {"e", "f"},
                                                                                                    {"f", "g"},
                                                                                                    {"g", "e"}}
                                                                                        << QSet<Error>{Error(ErrorType::CONNECTIVITY_ERROR, "a", "e", "", -1),
                                                                                                       Error(ErrorType::CYCLE_ERROR, "", "", "e->f->g->e", -1)}
                                                                                        << "a";

        QTest::newRow("Тест 5: Граф состоит из цикла") << EdgeList{{"a", "b"},
                                                                   {"b", "c"},
                                                                   {"c", "d"},
                                                                   {"d", "a"}}
                                                       << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->b->c->d->a", -1),
                                                                      Error(ErrorType::NO_ROOT)}
                                                       << "";

        QTest::newRow("Тест 6: Граф состоит из двух несвязных циклов") << EdgeList{{"a", "b"},
                                                                                   {"b", "c"},
                                                                                   {"c", "a"},
                                                                                   {"d", "e"},
                                                                                   {"e", "f"},
                                                                                   {"f", "d"}}
                                                                       << QSet<Error>{Error(ErrorType::CYCLE_ERROR, "", "", "a->b->c->a", -1),
                                                                                      Error(ErrorType::CYCLE_ERROR, "", "", "d->e->f->d", -1),
                                                                                      Error(ErrorType::NO_ROOT)}
                                                                       << "";

        QTest::newRow("Тест 7: Граф состоит из двух несвязных компонент, в одной из которых нарушен алфавитный порядок имен узлов") << EdgeList{{"a", "b"},
                                                                                                                                               {"b", "c"},
                                                                                                                                               {"c", "d"},
                                                                                                                                               {"h", "g"},
                                                                                                                                               {"g", "f"},
                                                                                                                                               {"f", "e"}}
                                                                                                                                   << QSet<Error>{Error(ErrorType::CONNECTIVITY_ERROR, "a", "h", "", -1)}
                                                                                                                                   << "a";

        QTest::newRow("Тест 8: Граф состоит из двух несвязных компонент, в одной из которых содержится цикл") << EdgeList{{"a", "b"},
                                                                                                                          {"b", "c"},
                                                                                                                          {"c", "d"},
                                                                                                                          {"h", "g"},
                                                                                                                          {"g", "f"},
                                                                                                                          {"f", "h"},
                                                                                                                          {"f", "e"}}
                                                                                                              << QSet<Error>{Error(ErrorType::CONNECTIVITY_ERROR, "a", "f", "", -1),
                                                                                                                             Error(ErrorType::CYCLE_ERROR, "", "", "f->h->g->f", -1)}
                                                                                                              << "a";

        QTest::newRow("Тест 9: Граф состоит из двух несвязных компонент, сожержащих цикл") << EdgeList{{"a", "b"},
                                                                                                       {"b", "c"},
                                                                                                       {"c", "d"},
                                                                                                       {"c", "a"},
                                                                                                       {"d", "i"},
                                                                                                       {"h", "g"},
                                                                                                       {"g", "f"},
                                                                                                       {"f", "h"},
                                                                                                       {"f", "e"}}
                                                                                           << QSet<Error>{Error(ErrorType::NO_ROOT),
                                                                                                          Error(ErrorType::CYCLE_ERROR, "", "", "f->h->g->f", -1),
                                                                                                          Error(ErrorType::CYCLE_ERROR, "", "", "a->b->c->a", -1)}
                                                                                           << "";

        QTest::newRow("Тест 10: Граф состоит из трех независимых корректных деревьев") << EdgeList{{"a", "b"},
                                                                                                   {"c", "d"},
                                                                                                   {"e", "f"}}
                                                                                       << QSet<Error>{Error(ErrorType::CONNECTIVITY_ERROR, "a", "c", "", -1),
                                                                                                      Error(ErrorType::CONNECTIVITY_ERROR, "a", "e", "", -1)}
                                                                                       << "a";
    }

    void testValidateStructure()
    {
        QFETCH(EdgeList, edges);
        QFETCH(QSet<Error>, expErrors);
        QFETCH(QString, rootName);

        // Строим дерево из заданных связей
        QMap<QString, Node*> nodes;
        buildTree(edges, nodes);

        // Ищем указатель на корень
        Node* rootPtr = nullptr;
        if (!rootName.isEmpty())
            rootPtr = nodes.value(rootName);

        QSet<Error> actualErrors;
        validateStructure(rootPtr, actualErrors, nodes);

        bool testPassed = true;
        QStringList msgs;

        // Проверяем ошибки
        if (actualErrors != expErrors)
        {
            showErrorLog(actualErrors, expErrors);
            msgs << "Полученный набор ошибок не совпадает с ожидаемым";
            testPassed = false;
        }

        // Очищаем память
        qDeleteAll(nodes);

        // Выводим ошибки
        if (!testPassed)
        {
            QFAIL(qPrintable(msgs.join("\n")));
        }
    }
};

#include "testValidateStructure.moc"
