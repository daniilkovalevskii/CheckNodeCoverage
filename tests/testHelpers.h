#ifndef TESTHELPERS_H
#define TESTHELPERS_H
#include "../CheckNodeCoverage/structures.h"
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include "QTest"

typedef QVector<QPair<QString, QString>> EdgeList;
Q_DECLARE_METATYPE(EdgeList)

// Функция для чтения строк из файла
QStringList readDotFile(const QString& relativePath);

// Конвертация ошибок в удобочитаемый формат
QString errorTypeToString(ErrorType type);

// Функция для сравнения ожидаемых и полученных ошибок
void showErrorLog(const QSet<Error>& actual, const QSet<Error>& expected);

// Функция для построения дерева программно
Node* buildTree(const QVector<QPair<QString, QString>>& edges, QMap<QString, Node*>& outNodes);

struct ExpectedNodeProfile
{
    QString name;
    NodeShape shape;
    QString parentName;
    QStringList childNames;
};

// Функция для проверки построенного дерева
bool treeMatchesExpectedProfile(const QMap<QString, Node*>& allNodes,
                                const QVector<ExpectedNodeProfile>& expectedProfiles,
                                QString& outError);

#endif // TESTHELPERS_H
