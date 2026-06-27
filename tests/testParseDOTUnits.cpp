#include <QtTest>
#include "testHelpers.h"
#include "../CheckNodeCoverage/inputProcessing.h"
#include "../CheckNodeCoverage/structures.h"

typedef QMap<QString, NodeShape> NodeShapeMap;
typedef QMap<QString, QStringList> ParentMap;

Q_DECLARE_METATYPE(NodeShapeMap)
Q_DECLARE_METATYPE(ParentMap)

class TestParseDOTUnits : public QObject
{
    Q_OBJECT

private slots:
    void testValidateHeader_data();
    void testValidateHeader();

    void testFindRoot_data();
    void testFindRoot();

    void testCountShapes_data();
    void testCountShapes();

    void testAppendShapeAnalysisErrors_data();
    void testAppendShapeAnalysisErrors();

    void testAppendMultiParentErrors_data();
    void testAppendMultiParentErrors();

    void testReadFile_data();
    void testReadFile();
};

void TestParseDOTUnits::testValidateHeader_data()
{
    QTest::addColumn<QStringList>("lines");
    QTest::addColumn<int>("expectedIndex");
    QTest::addColumn<QSet<Error>>("expectedErrors");

    QTest::newRow("Заголовок на первой строке")
        << QStringList{"digraph G {", "}"} << 0 << QSet<Error>{};

    QTest::newRow("Пустые строки перед заголовком")
        << QStringList{"", "   ", "digraph G {", "}"} << 2 << QSet<Error>{};

    QTest::newRow("Невалидный заголовок")
        << QStringList{"not a header", "}"} << -1 << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 1)};

    QTest::newRow("Невалидный заголовок после пустых строк")
        << QStringList{"", "", "graph G {"} << -1 << QSet<Error>{Error(ErrorType::SYNTAX_ERROR, "", "", "", 3)};

    QTest::newRow("Только пустые строки")
        << QStringList{"", "   ", ""} << -1 << QSet<Error>{};

    QTest::newRow("Заголовок с пробелами по краям строки")
        << QStringList{"   digraph G {   ", "}"} << 0 << QSet<Error>{};

    QTest::newRow("Заголовок с другим именем графа")
        << QStringList{"digraph MyGraph {", "}"} << 0 << QSet<Error>{};
}

void TestParseDOTUnits::testValidateHeader()
{
    QFETCH(QStringList, lines);
    QFETCH(int, expectedIndex);
    QFETCH(QSet<Error>, expectedErrors);

    QSet<Error> actualErrors;
    int actualIndex = validateHeader(lines, actualErrors);

    if (actualErrors != expectedErrors) {
        showErrorLog(actualErrors, expectedErrors);
    }

    QCOMPARE(actualIndex, expectedIndex);
    QCOMPARE(actualErrors, expectedErrors);
}

void TestParseDOTUnits::testFindRoot_data()
{
    QTest::addColumn<EdgeList>("edges");
    QTest::addColumn<QStringList>("fileOrder");
    QTest::addColumn<QString>("expectedRootName");

    QTest::newRow("Нет узлов")
        << EdgeList{} << QStringList{} << QString();

    QTest::newRow("Первый узел - корень")
        << EdgeList{{"a", "b"}} << QStringList{"a", "b"} << QString("a");

    QTest::newRow("Все узлы имеют родителя")
        << EdgeList{{"a", "b"}, {"b", "c"}, {"c", "a"}} << QStringList{"a", "b", "c"} << QString();

    QTest::newRow("Несколько корней - возвращается первый по порядку")
        << EdgeList{} << QStringList{"x", "y"} << QString("x");

    QTest::newRow("Корень не первый в fileOrderNodes")
        << EdgeList{{"root", "child1"}} << QStringList{"child1", "root"} << QString("root");
}

void TestParseDOTUnits::testFindRoot()
{
    QFETCH(EdgeList, edges);
    QFETCH(QStringList, fileOrder);
    QFETCH(QString, expectedRootName);

    QSet<Error> errors;
    ParseContext ctx(errors);
    ctx.fileOrderNodes = fileOrder;

    buildTree(edges, ctx.res.allNodes);

    if (QString(QTest::currentDataTag()) == "Все узлы имеют родителя")
    {
        Node* a = new Node("a", NodeShape::DEFAULT);
        Node* b = new Node("b", NodeShape::DEFAULT);
        a->parent = b;
        b->parent = a;
        ctx.res.allNodes.insert("a", a);
        ctx.res.allNodes.insert("b", b);
        ctx.fileOrderNodes = {"a", "b"};
    }
    else if (QString(QTest::currentDataTag()) == "Несколько корней - возвращается первый по порядку")
    {
        ctx.res.allNodes.insert("x", new Node("x", NodeShape::DEFAULT));
        ctx.res.allNodes.insert("y", new Node("y", NodeShape::DEFAULT));
    }

    Node* root = findRoot(ctx);

    if (expectedRootName.isEmpty())
    {
        QVERIFY(root == nullptr);
    }
    else
    {
        QVERIFY(root != nullptr);
        QCOMPARE(root->name, expectedRootName);
    }

    qDeleteAll(ctx.res.allNodes);
}

void TestParseDOTUnits::testCountShapes_data()
{
    QTest::addColumn<EdgeList>("edges");
    QTest::addColumn<NodeShapeMap>("shapes");
    QTest::addColumn<bool>("includeNullptr");
    QTest::addColumn<int>("expectedTargetCount");
    QTest::addColumn<int>("expectedMarkedCount");
    QTest::addColumn<QStringList>("expectedTargetNames");

    QTest::newRow("Пустая карта")
        << EdgeList{} << NodeShapeMap{} << false << 0 << 0 << QStringList{};

    QTest::newRow("Только DIAMOND узлы")
        << EdgeList{}
        << NodeShapeMap{{"a", NodeShape::DIAMOND}, {"b", NodeShape::DIAMOND}}
        << false << 2 << 0 << QStringList{"a", "b"};

    QTest::newRow("Только RECTANGLE узлы")
        << EdgeList{}
        << NodeShapeMap{{"a", NodeShape::RECTANGLE}, {"b", NodeShape::RECTANGLE}}
        << false << 0 << 2 << QStringList{};

    QTest::newRow("Смесь форм с DEFAULT")
        << EdgeList{}
        << NodeShapeMap{{"a", NodeShape::DIAMOND}, {"b", NodeShape::RECTANGLE}, {"c", NodeShape::DEFAULT}}
        << false << 1 << 1 << QStringList{"a"};

    QTest::newRow("Карта содержит nullptr")
        << EdgeList{} << NodeShapeMap{{"a", NodeShape::DIAMOND}} << true << 1 << 0 << QStringList{"a"};

    QTest::newRow("Несколько DIAMOND узлов, порядок по ключу")
        << EdgeList{}
        << NodeShapeMap{{"z", NodeShape::DIAMOND}, {"a", NodeShape::DIAMOND}}
        << false << 2 << 0 << QStringList{"a", "z"};
}

void TestParseDOTUnits::testCountShapes()
{
    QFETCH(EdgeList, edges);
    QFETCH(NodeShapeMap, shapes);
    QFETCH(bool, includeNullptr);
    QFETCH(int, expectedTargetCount);
    QFETCH(int, expectedMarkedCount);
    QFETCH(QStringList, expectedTargetNames);

    QMap<QString, Node*> allNodes;
    buildTree(edges, allNodes);

    for (auto it = shapes.constBegin(); it != shapes.constEnd(); ++it)
    {
        if (!allNodes.contains(it.key()))
        {
            allNodes.insert(it.key(), new Node(it.key(), it.value()));
        }
        else
        {
            allNodes[it.key()]->shape = it.value();
        }
    }

    if (includeNullptr)
    {
        allNodes.insert("__null__", nullptr);
    }

    ShapeCounts counts = countShapes(allNodes);

    QCOMPARE(counts.targetCount, expectedTargetCount);
    QCOMPARE(counts.markedCount, expectedMarkedCount);
    QCOMPARE(counts.targetNames, expectedTargetNames);
    QCOMPARE(counts.targetNodes.size(), expectedTargetNames.size());

    qDeleteAll(allNodes);
}

void TestParseDOTUnits::testAppendShapeAnalysisErrors_data()
{
    QTest::addColumn<int>("targetCount");
    QTest::addColumn<int>("markedCount");
    QTest::addColumn<QStringList>("targetNames");
    QTest::addColumn<QVector<bool>>("targetIsLeaf");
    QTest::addColumn<QSet<Error>>("expectedErrors");

    QTest::newRow("Нет целевых и нет отмеченных узлов")
        << 0 << 0 << QStringList{} << QVector<bool>{}
        << QSet<Error>{Error(ErrorType::NO_TARGET, "", "", "", -1),
                       Error(ErrorType::NO_MARKED_NODES, "", "", "", -1)};

    QTest::newRow("Полностью корректный граф")
        << 1 << 1 << QStringList{"a"} << QVector<bool>{false} << QSet<Error>{};

    QTest::newRow("Несколько целевых узлов")
        << 2 << 1 << QStringList{"z", "a"} << QVector<bool>{false, false}
        << QSet<Error>{Error(ErrorType::MULTIPLE_TARGETS, "a, z", "", "", -1)};

    QTest::newRow("Целевой узел - лист")
        << 1 << 1 << QStringList{"a"} << QVector<bool>{true}
        << QSet<Error>{Error(ErrorType::LEAF_TARGET, "a", "", "", -1)};

    QTest::newRow("Нет отмеченных узлов")
        << 1 << 0 << QStringList{"a"} << QVector<bool>{false} << QSet<Error>{Error(ErrorType::NO_MARKED_NODES, "", "", "", -1)};

    QTest::newRow("Несколько целевых-листьев без отмеченных узлов")
        << 2 << 0 << QStringList{"b", "a"} << QVector<bool>{true, true}
        << QSet<Error>{Error(ErrorType::MULTIPLE_TARGETS, "a, b", "", "", -1),
                       Error(ErrorType::NO_MARKED_NODES, "", "", "", -1),
                       Error(ErrorType::LEAF_TARGET, "b", "", "", -1),
                       Error(ErrorType::LEAF_TARGET, "a", "", "", -1)};
}

void TestParseDOTUnits::testAppendShapeAnalysisErrors()
{
    QFETCH(int, targetCount);
    QFETCH(int, markedCount);
    QFETCH(QStringList, targetNames);
    QFETCH(QVector<bool>, targetIsLeaf);
    QFETCH(QSet<Error>, expectedErrors);

    ShapeCounts counts;
    counts.targetCount = targetCount;
    counts.markedCount = markedCount;
    counts.targetNames = targetNames;

    for (int i = 0; i < targetNames.size(); ++i)
    {
        Node* n = new Node(targetNames[i], NodeShape::DIAMOND);
        if (!targetIsLeaf[i])
        {
            Node* fakeChild = new Node(targetNames[i] + "_child", NodeShape::DEFAULT);
            n->children.append(fakeChild);
        }
        counts.targetNodes.append(n);
    }

    QSet<Error> actualErrors;
    appendShapeAnalysisErrors(actualErrors, counts);

    if (actualErrors != expectedErrors)
    {
        showErrorLog(actualErrors, expectedErrors);
    }

    QCOMPARE(actualErrors, expectedErrors);

    for (Node* n : counts.targetNodes)
    {
        qDeleteAll(n->children);
        delete n;
    }
}

void TestParseDOTUnits::testAppendMultiParentErrors_data()
{
    QTest::addColumn<QStringList>("multiParentOrder");
    QTest::addColumn<ParentMap>("nodeAllParents");
    QTest::addColumn<QSet<Error>>("expectedErrors");

    QTest::newRow("Нет узлов с несколькими родителями")
        << QStringList{} << ParentMap{} << QSet<Error>{};

    QTest::newRow("Один узел с двумя родителями")
        << QStringList{"c"} << ParentMap{{"c", QStringList{"a", "b"}}}
        << QSet<Error>{Error(ErrorType::MULTI_PARENT, "c", "a, b", "", -1)};

    QTest::newRow("Несколько узлов с множественными родителями")
        << QStringList{"c", "f"}
        << ParentMap{{"c", QStringList{"a", "b"}}, {"f", QStringList{"d", "e", "g"}}}
        << QSet<Error>{Error(ErrorType::MULTI_PARENT, "c", "a, b", "", -1),
                       Error(ErrorType::MULTI_PARENT, "f", "d, e, g", "", -1)};
}

void TestParseDOTUnits::testAppendMultiParentErrors()
{
    QFETCH(QStringList, multiParentOrder);
    QFETCH(ParentMap, nodeAllParents);
    QFETCH(QSet<Error>, expectedErrors);

    QSet<Error> errors;
    ParseContext ctx(errors);
    ctx.multiParentOrder = multiParentOrder;
    ctx.nodeAllParents = nodeAllParents;

    appendMultiParentErrors(ctx);

    if (errors != expectedErrors)
    {
        showErrorLog(errors, expectedErrors);
    }

    QCOMPARE(errors, expectedErrors);
}

void TestParseDOTUnits::testReadFile_data()
{
    QTest::addColumn<QString>("filePath");
    QTest::addColumn<bool>("createFile");
    QTest::addColumn<QString>("fileContent");
    QTest::addColumn<int>("expectedLinesCount");
    QTest::addColumn<QSet<Error>>("expectedErrors");

    QTest::newRow("Файл не найден")
        << "ghost_file_404.dot" << false << "" << 0
        << QSet<Error>{Error(ErrorType::FILE_ERROR, "", "", "ghost_file_404.dot", -1)};

    QTest::newRow("Успешное чтение файла")
        << "temp_test_graph.dot" << true << "digraph G {\n"
                                            "  A -> B;\n"
                                            "}" << 3
        << QSet<Error>{};
}

void TestParseDOTUnits::testReadFile()
{
    QFETCH(QString, filePath);
    QFETCH(bool, createFile);
    QFETCH(QString, fileContent);
    QFETCH(int, expectedLinesCount);
    QFETCH(QSet<Error>, expectedErrors);

    if (createFile)
    {
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            out << fileContent;
            file.close();
        }
    }

    QSet<Error> actualErrors;
    QStringList actualLines = readFile(filePath, actualErrors);

    if (createFile)
    {
        QFile::remove(filePath);
    }

    if (actualErrors != expectedErrors)
    {
        showErrorLog(actualErrors, expectedErrors);
    }

    QCOMPARE(actualLines.size(), expectedLinesCount);
    QCOMPARE(actualErrors, expectedErrors);
}

#include "testParseDOTUnits.moc"
