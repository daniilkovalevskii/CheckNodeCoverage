QT += core
CONFIG += console c++17
CONFIG -= app_bundle

TARGET = CheckNodeCoverage
TEMPLATE = app

HEADERS += structures.h inputProcessing.h coverageAnalysis.h outputGeneration.h
SOURCES += main.cpp inputProcessing.cpp coverageAnalysis.cpp outputGeneration.cpp
