#include "inputProcessing.h"
#include <QSet>
#include <QRegularExpression>
#include <QStack>

ParseResult parseDOT(const QStringList& lines, QSet<Error>& errors)
{
    ParseResult res;
    errors.clear();
    res.root = nullptr;

    if (lines.isEmpty())
        return res;

    // Регулярные выражения для парсинга файла
    QRegularExpression headerRegex(R"(^\s*digraph\s+\w+\s*\{\s*$)");
    QRegularExpression closingRegex(R"(^\s*\}\s*$)");
    QRegularExpression edgeChainRegex(R"(^\s*\w+(?:\s*->\s*\w+)+\s*;\s*$)");
    QRegularExpression singleNodeRegex(R"(^\s*(\w+)(?:\s*\[.*\])?\s*;\s*$)");
    QRegularExpression shapeValueRegex(R"(shape\s*=\s*\"?(\w+)\"?)");
    QRegularExpression forbiddenRegex(R"(^\s*(subgraph|cluster|graph|node|edge)\b)");

    bool hasHeader = false;
    int headerLineIdx = -1;

    // Валидация заголовка файла
    for (int i = 0; i < lines.size(); ++i)
    {
        QString line = lines.at(i).trimmed();
        // Пропуск пустых строк
        if (line.isEmpty())
            continue;

        // Проверяем наличие объявления графа
        if (headerRegex.match(line).hasMatch())
        {
            hasHeader = true;
            headerLineIdx = i;
            break;
        }
        // Если нет корректного объявления графа - фиксируем ошибку
        else
        {
            errors.insert(Error(ErrorType::SYNTAX_ERROR, "", "", "", i + 1));
            break;
        }
    }

    // Если нет объявления - не можем отвечать за корректность парсинга - прекращаем его
    if (!hasHeader)
        return res;

    bool hasClosing = false;
    bool hasSyntaxError = !errors.isEmpty();
    bool hasForbiddenError = false;

    QStringList multiParentOrder; // Список узлов, у которых больше одного родителя
    QMap<QString, QStringList> nodeAllParents; // Список родителей узлов
    QStringList fileOrderNodes; // Список узлов в порядке появления в файле
    int subgraphDepth = 0; // Счетчик вложенности для подсчета фигурных скобок

    // Построчный разбор тела графа
    for (int i = headerLineIdx + 1; i < lines.size(); ++i)
    {
        QString line = lines.at(i).trimmed();
        int lineNumber = i + 1;

        if (line.isEmpty())
            continue;

        // Проверяем закрывающую скобку графа, если не во вложенной структуре
        if (closingRegex.match(line).hasMatch() && subgraphDepth == 0)
        {
            hasClosing = true;
            continue;
        }

        QRegularExpressionMatch forbiddenMatch = forbiddenRegex.match(line);
        int opens = line.count('{');
        int closes = line.count('}');

        // Обработка запрещенных структур
        if (forbiddenMatch.hasMatch())
        {
            errors.insert(Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, "", "", "", lineNumber));
            hasForbiddenError = true;
            subgraphDepth += opens - closes;
        }
        else if (subgraphDepth > 0)
        {
            // Пропускаем вложенные строки, если внутри запрещенной структуры
            subgraphDepth += opens - closes;
            if (subgraphDepth < 0)
                subgraphDepth = 0;
        }
        else
        {
            // Проверяем совпадение с объявлением одного узла(или его атрибутов)/ребра(цепочки ребер)
            QRegularExpressionMatch edgeChainMatch = edgeChainRegex.match(line);
            QRegularExpressionMatch nodeMatch = singleNodeRegex.match(line);

            // Если найдено объявление ребер
            if (edgeChainMatch.hasMatch())
            {
                // Очищаем строку от точки с запятой и пробелов
                QString cleanLine = line;
                cleanLine.remove(";").remove(" ");
                // Разбиваем цепочку на узлы
                QStringList nodes = cleanLine.split("->");

                // Проходим по парам родитель->ребенок
                for (int k = 0; k < nodes.size() - 1; ++k)
                {
                    QString parentName = nodes[k];
                    QString childName = nodes[k+1];

                    // Фиксируем порядок появления узлов в файле
                    if (!fileOrderNodes.contains(parentName))
                        fileOrderNodes.append(parentName);
                    if (!fileOrderNodes.contains(childName))
                        fileOrderNodes.append(childName);

                    // Если узлов еще нет в общем словаре - создаем их
                    if (!res.allNodes.contains(parentName))
                    {
                        res.allNodes.insert(parentName, new Node(parentName, NodeShape::DEFAULT));
                    }
                    if (!res.allNodes.contains(childName))
                    {
                        res.allNodes.insert(childName, new Node(childName, NodeShape::DEFAULT));
                    }

                    Node* parentNode = res.allNodes[parentName];
                    Node* childNode = res.allNodes[childName];

                    if (!nodeAllParents[childName].contains(parentName))
                    {
                        // Если текущий родитель не зафиксирован в списке родителей узла - добавляем
                        nodeAllParents[childName].append(parentName);
                        // Добавляем узел в список узлов с несколькими родителями только тогда, когда появляется второй родитель
                        if (nodeAllParents[childName].size() == 2)
                        {
                            multiParentOrder.append(childName);
                        }
                    }

                    // Если у текущего узла нет родителя - добавляем его
                    if (childNode->parent == nullptr)
                    {
                        childNode->parent = parentNode;
                        parentNode->children.append(childNode);
                    }
                }
            }
            // Если обнаружено объявление узла
            else if (nodeMatch.hasMatch())
            {
                // Извлекаем имя узла
                QString nodeName = nodeMatch.captured(1);
                // Фиксируем появление в файле, если раньше не видели такого узла
                if (!fileOrderNodes.contains(nodeName))
                    fileOrderNodes.append(nodeName);

                // Добавляем в словарь, если такого узла не было
                if (!res.allNodes.contains(nodeName))
                {
                    res.allNodes.insert(nodeName, new Node(nodeName, NodeShape::DEFAULT));
                }

                Node* singleNode = res.allNodes[nodeName];

                // Если в строке есть квадратные скобки
                if (line.contains("[") || line.contains("]"))
                {
                    // Проверяем паттерн объявления атрибутов
                    QRegularExpressionMatch shapeMatch = shapeValueRegex.match(line);
                    if (shapeMatch.hasMatch())
                    {
                        // Извлекаем форму узла
                        QString shapeType = shapeMatch.captured(1).toLower();
                        // Присваиваем узлу форму
                        if (shapeType == "diamond")
                        {
                            singleNode->shape = NodeShape::DIAMOND;
                        }
                        else if (shapeType == "rectangle")
                        {
                            singleNode->shape = NodeShape::RECTANGLE;
                        }
                        // Если форма запрещена - фиксируем соответствующую ошибку
                        else
                        {
                            singleNode->shape = NodeShape::DEFAULT;
                            errors.insert(Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, nodeName, "", "", lineNumber));
                        }
                    }
                    // Если есть квадратные скобки, но нет атрибута формы - присваиваем форму по умолчанию
                    else
                    {
                        singleNode->shape = NodeShape::DEFAULT;
                        errors.insert(Error(ErrorType::FORBIDDEN_STRUCTURE_OR_FORM, nodeName, "", "", lineNumber));
                    }
                }
            }
            // Если строка не подошла ни под один шаблон - добавляем синтаксическую ошибку
            else
            {
                errors.insert(Error(ErrorType::SYNTAX_ERROR, "", "", "", lineNumber));
                hasSyntaxError = true;
            }
        }
    }

    // Если нет закрывающей скобки для графа в файле - добавляем синтаксическую ошибку
    if (!hasClosing)
    {
        errors.insert(Error(ErrorType::SYNTAX_ERROR, "", "", "", lines.size()));
        hasSyntaxError = true;
    }

    // Анализ графа
    if (!res.allNodes.isEmpty())
    {
        int targetCount = 0;
        int markedCount = 0;
        QStringList targetNames;
        QVector<Node*> targetNodes;

        // Добавляем ошибки для всех узлов с несколькими родителями
        for (const QString& childName : multiParentOrder)
        {
            QString parentsStr = nodeAllParents.value(childName).join(", ");
            errors.insert(Error(ErrorType::MULTI_PARENT, childName, parentsStr, "", -1));
        }

        // Подсчитываем количество целевых и отмеченных узлов
        for (auto it = res.allNodes.constBegin(); it != res.allNodes.constEnd(); ++it)
        {
            Node* n = it.value();
            if (n != nullptr)
            {
                if (n->shape == NodeShape::DIAMOND)
                {
                    targetCount++;
                    targetNodes.append(n);
                    targetNames.append(n->name);
                }
                if (n->shape == NodeShape::RECTANGLE)
                {
                    markedCount++;
                }
            }
        }

        // Находим корень
        for (const QString& name : fileOrderNodes)
        {
            Node* n = res.allNodes.value(name, nullptr);
            if (n != nullptr && n->parent == nullptr)
            {
                res.root = n;
                break;
            }
        }

        // Если не было запрещенных структур
        if (!hasForbiddenError)
        {
            // Если нет синтаксических ошибок
            if (!hasSyntaxError)
            {
                // Если нет целевого узла, добавляем ошибку
                if (targetCount == 0)
                {
                    errors.insert(Error(ErrorType::NO_TARGET, "", "", "", -1));
                }
                // Если больше одного целевого узла - добавляем ошибку
                if (targetCount > 1)
                {
                    targetNames.sort();
                    errors.insert(Error(ErrorType::MULTIPLE_TARGETS, targetNames.join(", "), "", "", -1));
                }
                // Если нет отмеченных узлов - добавляем ошибку
                if (markedCount == 0)
                {
                    errors.insert(Error(ErrorType::NO_MARKED_NODES, "", "", "", -1));
                }

                // Если целевой узел - лист, то мы не можем проверять для него покрытие, добавляем ошибку
                for (Node* targetNodePtr : targetNodes)
                {
                    if (targetNodePtr != nullptr && targetNodePtr->isLeaf())
                    {
                        errors.insert(Error(ErrorType::LEAF_TARGET, targetNodePtr->name, "", "", -1));
                    }
                }
            }
        }
    }
    // Если словарь узлов пуст и нет ни синтаксических ошибок, ни ошибок запрещенных структур - граф пуст, добавляем ошибки отсутствия
    // целевых и отмеченных узлов
    else
    {
        if (!hasSyntaxError && !hasForbiddenError)
        {
            errors.insert(Error(ErrorType::NO_TARGET, "", "", "", -1));
            errors.insert(Error(ErrorType::NO_MARKED_NODES, "", "", "", -1));
        }
    }

    // Если есть ошибки синтаксиса - мы не гарантируем корректное построение графа - очищаем построенную структуру
    if (hasSyntaxError)
    {
        qDeleteAll(res.allNodes);
        res.allNodes.clear();
        res.root = nullptr;
    }

    return res;
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
    return;
}
