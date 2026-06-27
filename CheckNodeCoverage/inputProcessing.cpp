#include "inputProcessing.h"
#include <QSet>
#include <QRegularExpression>
#include <QStack>
#include <QFile>

QStringList readFile(const QString& filepath, QSet<Error>& errors)
{
    QStringList lines;

    // Открываем входной файл для чтения в текстовом режиме
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // Выводим критическую ошибку в консоль
        qCritical().noquote() << "Входной файл не найден или недоступен для чтения.\n"
                                 "Проверьте правильность пути:" << filepath;

        // Заносим ошибку в массив ошибок для отчета
        errors.insert(Error(ErrorType::FILE_ERROR, "", "", filepath, -1));
        return lines;
    }

    QTextStream in(&file);

    // Построчно считываем весь файл до самого конца
    while (!in.atEnd())
    {
        QString line = in.readLine();
        lines.append(line);
    }

    // Закрываем файл после успешного чтения
    file.close();
    return lines;
}

ParseContext::ParseContext(QSet<Error>& errs) : errors(errs)
{
    res.root = nullptr;
}

int validateHeader(const QStringList& lines, QSet<Error>& errors)
{
    for (int i = 0; i < lines.size(); ++i)
    {
        QString line = lines.at(i).trimmed();
        // Пропуск пустых строк
        if (line.isEmpty())
        {
            continue;
        }

        // Проверяем наличие объявления графа
        if (header.match(line).hasMatch())
        {
            return i;
        }

        // Если нет корректного объявления графа - фиксируем ошибку
        errors.insert(Error(ErrorType::SYNTAX_ERROR, "", "", "", i + 1));
        return -1;
    }
    return -1;
}

void linkParentChild(ParseContext& ctx, const QString& parentName, const QString& childName, int lineNumber)
{
    // Фиксируем порядок появления узлов в файле
    if (!ctx.fileOrderNodes.contains(parentName))
    {
        ctx.fileOrderNodes.append(parentName);
    }
    if (!ctx.fileOrderNodes.contains(childName))
    {
        ctx.fileOrderNodes.append(childName);
    }

    // Если узлов еще нет в общем словаре - создаем их
    if (!ctx.res.allNodes.contains(parentName))
    {
        ctx.res.allNodes.insert(parentName, new Node(parentName, NodeShape::DEFAULT));
    }
    if (!ctx.res.allNodes.contains(childName))
    {
        ctx.res.allNodes.insert(childName, new Node(childName, NodeShape::DEFAULT));
    }

    Node* parentNode = ctx.res.allNodes[parentName];
    Node* childNode = ctx.res.allNodes[childName];

    // Если уже есть такой ребенок в списке детей - описана множественная связь, граф - не дерево
    if (parentNode->children.contains(childNode))
    {
        ctx.errors.insert(Error(ErrorType::SYNTAX_ERROR, "", "", "", lineNumber));
    }

    if (!ctx.nodeAllParents[childName].contains(parentName))
    {
        // Если текущий родитель не зафиксирован в списке родителей узла - добавляем
        ctx.nodeAllParents[childName].append(parentName);
        // Добавляем узел в список узлов с несколькими родителями только тогда, когда появляется второй родитель
        if (ctx.nodeAllParents[childName].size() == 2)
        {
            ctx.multiParentOrder.append(childName);
        }
    }

    // Если у текущего узла нет родителя - добавляем его
    if (childNode->parent == nullptr)
    {
        childNode->parent = parentNode;
        parentNode->children.append(childNode);
    }
}

void processEdgeChain(ParseContext& ctx, const QString& line, int lineNumber)
{
    // Очищаем строку от точки с запятой и пробелов
    QString cleanLine = line;
    cleanLine.remove(";").remove(" ");
    // Разбиваем цепочку на узлы
    QStringList nodes = cleanLine.split("->");

    // Проходим по парам родитель->ребенок
    for (int k = 0; k < nodes.size() - 1; ++k)
    {
        linkParentChild(ctx, nodes[k], nodes[k + 1], lineNumber);
    }
}

void parseShapeAttribute(ParseContext& ctx, Node* node, const QString& nodeName, const QString& line, int lineNumber)
{
    // Проверяем паттерн объявления атрибутов
    QRegularExpressionMatch shapeMatch = shapeValue.match(line);
    if (!shapeMatch.hasMatch())
    {
        // Если есть квадратные скобки, но нет атрибута формы - присваиваем форму по умолчанию
        node->shape = NodeShape::DEFAULT;
        ctx.errors.insert(Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, nodeName, "", "", lineNumber));
        return;
    }

    // Извлекаем форму узла
    QString shapeType = shapeMatch.captured(1).toLower();
    // Присваиваем узлу форму
    if (shapeType == "diamond")
    {
        node->shape = NodeShape::DIAMOND;
    }
    else if (shapeType == "rectangle")
    {
        node->shape = NodeShape::RECTANGLE;
    }
    // Если форма запрещена - фиксируем соответствующую ошибку
    else
    {
        node->shape = NodeShape::DEFAULT;
        ctx.errors.insert(Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, nodeName, "", "", lineNumber));
    }
}

void processSingleNode(ParseContext& ctx, const QRegularExpressionMatch& nodeMatch, const QString& line, int lineNumber)
{
    // Извлекаем имя узла
    QString nodeName = nodeMatch.captured(1);
    // Фиксируем появление в файле, если раньше не видели такого узла
    if (!ctx.fileOrderNodes.contains(nodeName))
    {
        ctx.fileOrderNodes.append(nodeName);
    }

    // Добавляем в словарь, если такого узла не было
    if (!ctx.res.allNodes.contains(nodeName))
    {
        ctx.res.allNodes.insert(nodeName, new Node(nodeName, NodeShape::DEFAULT));
    }

    Node* singleNode = ctx.res.allNodes[nodeName];

    // Если в строке есть квадратные скобки - разбираем атрибуты узла
    if (line.contains("[") || line.contains("]"))
    {
        parseShapeAttribute(ctx, singleNode, nodeName, line, lineNumber);
    }
}

bool tryProcessForbiddenOrNested(ParseContext& ctx, const QString& line, int lineNumber)
{
    QRegularExpressionMatch forbiddenMatch = forbidden.match(line);
    int opens = line.count('{');
    int closes = line.count('}');

    // Обработка запрещенных структур
    if (forbiddenMatch.hasMatch())
    {
        ctx.errors.insert(Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "", "", "", lineNumber));
        ctx.hasForbiddenError = true;
        ctx.subgraphDepth += opens - closes;
        return true;
    }

    // Пропускаем вложенные строки, если внутри запрещенной структуры
    if (ctx.subgraphDepth > 0)
    {
        ctx.subgraphDepth += opens - closes;
        if (ctx.subgraphDepth < 0)
        {
            ctx.subgraphDepth = 0;
        }
        return true;
    }

    return false;
}

void processBodyLine(ParseContext& ctx, const QString& line, int lineNumber)
{
    // Проверяем закрывающую скобку графа, если не во вложенной структуре
    if (closing.match(line).hasMatch() && ctx.subgraphDepth == 0)
    {
        ctx.hasClosing = true;
        return;
    }

    // Запрещенная структура или продолжение вложенности
    if (tryProcessForbiddenOrNested(ctx, line, lineNumber))
    {
        return;
    }

    // Проверяем совпадение с объявлением одного узла (или его атрибутов) / ребра (цепочки ребер)
    QRegularExpressionMatch edgeChainMatch = edgeChain.match(line);
    QRegularExpressionMatch nodeMatch = singleNode.match(line);

    // Если найдено объявление ребер
    if (edgeChainMatch.hasMatch())
    {
        processEdgeChain(ctx, line, lineNumber);
    }
    // Если обнаружено объявление узла
    else if (nodeMatch.hasMatch())
    {
        processSingleNode(ctx, nodeMatch, line, lineNumber);
    }
    // Если строка не подошла ни под один шаблон - добавляем синтаксическую ошибку
    else
    {
        ctx.errors.insert(Error(ErrorType::SYNTAX_ERROR, "", "", "", lineNumber));
        ctx.hasSyntaxError = true;
    }
}

void parseBody(ParseContext& ctx, const QStringList& lines, int headerLineIdx)
{
    // Построчный разбор тела графа
    for (int i = headerLineIdx + 1; i < lines.size(); ++i)
    {
        QString line = lines.at(i).trimmed();
        if (line.isEmpty())
        {
            continue;
        }

        processBodyLine(ctx, line, i + 1);
    }
}

Node* findRoot(const ParseContext& ctx)
{
    // Находим корень - первый по порядку появления в файле узел без родителя
    for (const QString& name : ctx.fileOrderNodes)
    {
        Node* n = ctx.res.allNodes.value(name, nullptr);
        if (n != nullptr && n->parent == nullptr)
        {
            return n;
        }
    }
    return nullptr;
}

ShapeCounts countShapes(const QMap<QString, Node*>& allNodes)
{
    ShapeCounts counts;
    // Подсчитываем количество целевых и отмеченных узлов
    for (auto it = allNodes.constBegin(); it != allNodes.constEnd(); ++it)
    {
        Node* n = it.value();
        if (n == nullptr)
        {
            continue;
        }

        // Подсчитываем целевые
        if (n->shape == NodeShape::DIAMOND)
        {
            counts.targetCount++;
            counts.targetNodes.append(n);
            counts.targetNames.append(n->name);
        }
        // Подсчитываем отмеченные
        if (n->shape == NodeShape::RECTANGLE)
        {
            counts.markedCount++;
        }
    }
    return counts;
}

void appendMultiParentErrors(ParseContext& ctx)
{
    // Добавляем ошибки для всех узлов с несколькими родителями
    for (const QString& childName : ctx.multiParentOrder)
    {
        QString parentsStr = ctx.nodeAllParents.value(childName).join(", ");
        ctx.errors.insert(Error(ErrorType::MULTI_PARENT, childName, parentsStr, "", -1));
    }
}

void appendShapeAnalysisErrors(QSet<Error>& errors, const ShapeCounts& counts)
{
    // Если нет целевого узла, добавляем ошибку
    if (counts.targetCount == 0)
    {
        errors.insert(Error(ErrorType::NO_TARGET, "", "", "", -1));
    }
    // Если больше одного целевого узла - добавляем ошибку
    if (counts.targetCount > 1)
    {
        QStringList sortedNames = counts.targetNames;
        sortedNames.sort();
        errors.insert(Error(ErrorType::MULTIPLE_TARGETS, sortedNames.join(", "), "", "", -1));
    }
    // Если нет отмеченных узлов - добавляем ошибку
    if (counts.markedCount == 0)
    {
        errors.insert(Error(ErrorType::NO_MARKED_NODES, "", "", "", -1));
    }

    // Если целевой узел - лист, то мы не можем проверять для него покрытие, добавляем ошибку
    for (Node* targetNodePtr : counts.targetNodes)
    {
        if (targetNodePtr != nullptr && targetNodePtr->isLeaf())
        {
            errors.insert(Error(ErrorType::LEAF_TARGET, targetNodePtr->name, "", "", -1));
        }
    }
}

void analyzeNonEmptyGraph(ParseContext& ctx)
{
    appendMultiParentErrors(ctx);

    ShapeCounts counts = countShapes(ctx.res.allNodes);
    ctx.res.root = findRoot(ctx);

    // Если не было запрещенных структур и нет синтаксических ошибок - анализируем формы узлов
    if (!ctx.hasForbiddenError && !ctx.hasSyntaxError)
    {
        appendShapeAnalysisErrors(ctx.errors, counts);
    }
}

void analyzeEmptyGraph(ParseContext& ctx)
{
    // Если словарь узлов пуст и нет ни синтаксических ошибок, ни ошибок запрещенных структур -
    // граф пуст, добавляем ошибки отсутствия целевых и отмеченных узлов
    if (!ctx.hasSyntaxError && !ctx.hasForbiddenError)
    {
        ctx.errors.insert(Error(ErrorType::NO_TARGET, "", "", "", -1));
        ctx.errors.insert(Error(ErrorType::NO_MARKED_NODES, "", "", "", -1));
    }
}

ParseResult parseDOT(const QStringList& lines, QSet<Error>& errors)
{
    errors.clear();

    if (lines.isEmpty())
    {
        return ParseResult{};
    }

    // Валидация заголовка файла
    int headerLineIdx = validateHeader(lines, errors);
    // Если нет объявления - не можем отвечать за корректность парсинга - прекращаем его
    if (headerLineIdx < 0)
    {
        return ParseResult{};
    }

    ParseContext ctx(errors);
    ctx.hasSyntaxError = !errors.isEmpty();

    parseBody(ctx, lines, headerLineIdx);

    // Если нет закрывающей скобки для графа в файле - добавляем синтаксическую ошибку
    if (!ctx.hasClosing)
    {
        ctx.errors.insert(Error(ErrorType::SYNTAX_ERROR, "", "", "", lines.size()));
        ctx.hasSyntaxError = true;
    }

    // Анализ графа
    if (!ctx.res.allNodes.isEmpty())
    {
        analyzeNonEmptyGraph(ctx);
    }
    else
    {
        analyzeEmptyGraph(ctx);
    }

    // Если есть ошибки синтаксиса - мы не гарантируем корректное построение графа - очищаем построенную структуру
    if (ctx.hasSyntaxError)
    {
        qDeleteAll(ctx.res.allNodes);
        ctx.res.allNodes.clear();
        ctx.res.root = nullptr;
    }

    return ctx.res;
}

void dfsValidate(Node* node, QSet<Node*>& visited, QStack<Node*>& stack, QSet<Error>& errors)
{
    stack.push(node);
    visited.insert(node);

    // Для каждого ребенка текущего узла
    for (Node* child : node->children)
    {
        // Если ребенок уже в стэке, значит найден цикл
        if (stack.contains(child))
        {
            QString cycleNames;
            int startIndex = stack.indexOf(child); // Берем индекс ребенка

            // Собираем цикл
            for (int i = startIndex; i < stack.size(); ++i)
            {
                cycleNames.append(stack.at(i)->name);
                cycleNames.append("->");
            }
            // Замыкаем цикл
            cycleNames.append(child->name);
            // Добавляем ошибку
            errors.insert(Error(ErrorType::CYCLE_ERROR, "", "", cycleNames, -1));
        }
        // Если ребенок еще не был посещен - запускаем проверку циклов от него
        else if (!visited.contains(child))
        {
            dfsValidate(child, visited, stack, errors);
        }
    }
    // Удаляем узел из стэка
    stack.pop();
}

bool findComponentRoot(Node* node, const QSet<Node*>& visited, Node*& outRoot)
{
    QSet<Node*> alreadyVisited;
    Node* currNode = node;
    // Пока у текущего узла есть родитель(он не является корнем) и он еще не был посещен глобально
    while (currNode->parent != nullptr && !visited.contains(currNode->parent))
    {
        // Если в ходе работы функции мы уже посещали родителя текущего узла - значит есть цикл и мы вернем родителя текущего узла
        if (alreadyVisited.contains(currNode->parent))
        {
            outRoot = currNode->parent;
            return false;
        }
        // Добавляем в посещенные текущий узел
        alreadyVisited.insert(currNode);
        // Считаем текущим узлом родителя текущего узла
        currNode = currNode->parent;
    }
    // Считать найденным текущий узел
    outRoot = currNode;
    return true;
}

void validateStructure(Node* root, QSet<Error>& errors, const QMap<QString, Node*>& allNodes)
{
    QSet<Node*> visited;
    QStack<Node*> stack;

    // Если в дереве нет корня - добавляем ошибку
    if (root == nullptr)
    {
        errors.insert(Error(ErrorType::NO_ROOT));
    }
    // Иначе запускаем поиск циклов от корня
    else
    {
        dfsValidate(root, visited, stack, errors);
    }

    // Независимо от того есть корень или нет, проходим по словарю узлов
    // Если корень есть - мы пройдем по остальным узлам в словаре и проверим, есть ли несвязные компоненты и циклы в них
    // Если корня нет - мы будем искать циклы
    for (Node* node : allNodes)
    {
        if (!visited.contains(node))
        {
            // Ищем корень компоненты
            Node* componentRoot;
            findComponentRoot(node, visited, componentRoot);

            // Если дерево содержит корень, добавляем ошибку связности
            if (root != nullptr)
            {
                errors.insert(Error(ErrorType::CONNECTIVITY_ERROR, QString("%1").arg(root->name), QString("%1").arg(componentRoot->name)));
            }

            // Очищаем стэк, ищем циклы от корня компоненты
            stack.clear();
            dfsValidate(componentRoot, visited, stack, errors);
        }
    }
}
