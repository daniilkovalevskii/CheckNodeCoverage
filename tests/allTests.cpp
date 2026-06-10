#include <QtTest>
#include "testParseDOT.cpp"
#include "testFindComponentRoot.cpp"
#include "testDfsValidate.cpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    int result = 0;

    testParseDot testParseDot;
    result |= QTest::qExec(&testParseDot, argc, argv);

    FindComponentRoot testFindComponentRoot;
    result |= QTest::qExec(&testFindComponentRoot, argc, argv);

    TestDfsValidate testDfsValidate;
    result |= QTest::qExec(&testDfsValidate, argc, argv);

    return result;
}
