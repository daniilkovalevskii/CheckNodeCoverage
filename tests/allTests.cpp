#include <QtTest>
#include "testParseDOT.cpp"
#include "testFindComponentRoot.cpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    int result = 0;

    testParseDot testParseDot;
    result |= QTest::qExec(&testParseDot, argc, argv);

    FindComponentRoot testFindComponentRoot;
    result |= QTest::qExec(&testFindComponentRoot, argc, argv);

    return result;
}
