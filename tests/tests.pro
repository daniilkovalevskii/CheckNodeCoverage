QT += core testlib
CONFIG += console c++17
CONFIG -= app_bundle

TARGET = all_tests
TEMPLATE = app

SOURCES += allTests.cpp

HEADERS += ../CheckNodeCoverage/inputProcessing.h \
           ../CheckNodeCoverage/coverageAnalysis.h \
           ../CheckNodeCoverage/outputGeneration.h

SOURCES += ../CheckNodeCoverage/inputProcessing.cpp \
           ../CheckNodeCoverage/coverageAnalysis.cpp \
           ../CheckNodeCoverage/outputGeneration.cpp
