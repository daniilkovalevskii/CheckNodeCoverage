#include <QtTest>
#include "testParseDOT.cpp"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    int result = 0;

    testParseDot testParseDot;
    result |= QTest::qExec(&testParseDot, argc, argv);

    return result;
}
