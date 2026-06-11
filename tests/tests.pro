QT += core testlib
CONFIG += console c++17
CONFIG -= app_bundle

TARGET = all_tests
TEMPLATE = app

HEADERS += testHelpers.h
SOURCES += allTests.cpp testParseDOT.cpp testFindComponentRoot.cpp \
    testDfsCovered.cpp \
    testDfsSearching.cpp \
    testDfsValidate.cpp \
    testValidateStructure.cpp

HEADERS += ../CheckNodeCoverage/inputProcessing.h \
           ../CheckNodeCoverage/coverageAnalysis.h \
           ../CheckNodeCoverage/outputGeneration.h

SOURCES += ../CheckNodeCoverage/inputProcessing.cpp \
           ../CheckNodeCoverage/coverageAnalysis.cpp \
           ../CheckNodeCoverage/outputGeneration.cpp
