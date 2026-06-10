#include <QtTest>
#include "testParseDOT.cpp"
#include "testFindComponentRoot.cpp"
#include "testDfsValidate.cpp"
#include "testValidateStructure.cpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    int result = 0;

    testParseDot testParseDot;
    result |= QTest::qExec(&testParseDot, argc, argv);

    FindComponentRoot testFindComponentRoot;
    result |= QTest::qExec(&testFindComponentRoot, argc, argv);

    TestDfsValidate testDfsValidate;
    result |= QTest::qExec(&testDfsValidate, argc, argv);

    TestValidateStructure testValidateStructure;
    result |= QTest::qExec(&testValidateStructure, argc, argv);

    return result;
}
