#ifndef TESTHELPERS_H
#define TESTHELPERS_H
#include "../CheckNodeCoverage/structures.h"
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include "QTest"

// Функция для чтения строк из файла
QStringList readDotFile(const QString& relativePath)
{
    QString fullPath = QFINDTESTDATA(relativePath);
    if (fullPath.isEmpty()) {
        fullPath = QCoreApplication::applicationDirPath() + "/" + relativePath;
    }

    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть тестовый файл:" << fullPath;
        return QStringList();
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    return lines;
}

#endif // TESTHELPERS_H
