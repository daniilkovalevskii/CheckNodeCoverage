TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += coreApp unitTests

coreApp.file = CheckNodeCoverage/core.pro
unitTests.file = tests/tests.pro
