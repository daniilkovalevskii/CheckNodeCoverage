#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <QString>
#include <QVector>
#include <QMap>

/*!
 *  \file structures.h
 *  \brief Объявление основных структур данных, перечислений и классов с методами для анализа графов DOT.
 */

/*!
 *  \brief Перечисление допустимых форм узлов в формате DOT.
 */
enum NodeShape
{
    DIAMOND,
    RECTANGLE,
    DEFAULT
};


/*!
 *  \brief Класс, представляющий узел графа.
 */
class Node
{
public:
    /*!
     *  \brief Конструктор узла.
     *  \param[in] newName Имя узла.
     *  \param[in] newShape Форма узла (по умолчанию выставляется \c DEFAULT).
     */
    Node(QString newName, NodeShape newShape = NodeShape::DEFAULT)
    {
        name = newName;
        shape = newShape;

        parent = nullptr;
    };

    /*!
     *  \brief Деструктор узла.
     *
     *  \note В ходе написания кода программы было решено отказаться от рекурсивного удаления содержимого узла,
     *  так как оно вызывало критическую ошибку \c SIGABRT.
     *  Вместо этого используется \c qDeleteAll, который гарантированно удалит все узлы по одному разу.
     */
    ~Node()
    {}

    QString name; /**< Имя узла. */
    NodeShape shape; /**< Форма узла. */
    Node* parent; /**< Указатель на родителя узла (\c nullptr, если узел является корнем). */
    QVector<Node*> children; /**< Список указателей на дочерние узлы. */

    /*!
     *  \brief Проверяет, является ли узел листом дерева.
     *  \return \c true, если у узла нет детей; иначе - \c false.
     */
    bool isLeaf() const
    {
        return children.isEmpty();
    };
};

/*!
 *  \brief Перечисление поддерживаемых типов ошибок.
 */
enum ErrorType
{
    FILE_ERROR, /**< Ошибка чтения входного файла. */
    OUTPUT_FILE_ERROR, /**< Ошибка записи/создания выходного файла. */
    CONNECTIVITY_ERROR, /**< Ошибка связности (создается только для графов, у которых есть корень и несвязные компоненты; для остальных добавляется ошибка \c NO_ROOT). */
    NO_MARKED_NODES, /**< Ошибка, сигнализирующая о том, что во входном файле нет отмеченных узлов (с формой \c RECTANGLE). */
    LEAF_TARGET, /**< Ошибка, сигнализирующая о том, что целевой узел является листом - проверить его покрытие невозможно. */
    NO_TARGET, /**< Ошибка, сигнализирующая о том, что во входном файле нет целевого узла (с формой \c DIAMOND). */
    MULTIPLE_TARGETS, /**< Ошибка, сигнализирующая о том, что во входном файле есть несколько целевых узлов (с формой \c DIAMOND). */
    CYCLE_ERROR, /**< Ошибка, сигнализирующая о том, что в описанном графе есть циклы. */
    NOT_DESCENDANT, /**< Ошибка, сигнализирующая о том, что отмеченный узел находится вне поддерева целевого узла. */
    MULTI_PARENT, /**< Ошибка, сигнализирующая о том, что у узла несколько родителей во входном файле. */
    REDUNDANT_NODE, /**< Ошибка, сигнализирующая о том, что отмеченный узел находится в поддереве уже отмеченного узла. */
    NO_ROOT, /**< Ошибка, сигнализирующая о том, что в переданном графе нет корня. */
    SYNTAX_ERROR, /**< Синтаксическая ошибка, привязана к строке файла. */
    FORBIDDEN_STRUCTURE_OR_FORM /**< Ошибка, сигнализирующая о том, что во входном графе используются запрещенные структуры или формы (например, \c subgraph или \c circle). */
};

/*!
 *  \brief Класс, хранящий информацию об ошибке.
 */
class Error
{
public:
    /*!
     *  \brief Конструктор по умолчанию.
     */
    Error()
    {
    };

    /*!
         *  \brief Конструктор с полной инициализацией полей.
         *  \param[in] newType Тип ошибки (хранится в перечислении \c ErrorType).
         *  \param[in] nodeName1 Имя узла, с которым связана ошибка (если ошибка подразумевает несколько узлов, которые не имеют зависимости друг от друга, то все они помещаются сюда).
         *  \param[in] nodeName2 Имя второго узла, с которым связана ошибка.
         *  \param[in] newPath Путь для циклов/путь до файла, который не удалось открыть.
         *  \param[in] newLine Номер строки, содержащей ошибку во входном файле (\c -1, если ошибка не относится к строке или нельзя однозначно утверждать принадлежность к какой-либо строке).
         */
    Error(ErrorType newType, QString nodeName1 = "", QString nodeName2 = "", QString newPath = "", int newLine = -1)
    {
        type = newType;
        nodeName = nodeName1;
        relatedNodeName = nodeName2;
        path = newPath;
        line = newLine;
    };

    ErrorType type; /**< Тип ошибки (хранится в перечислении \c ErrorType). */
    QString nodeName; /**< Имя основного узла, связанного с ошибкой
                            *  (если ошибка подразумевает несколько узлов, которые не имеют зависимости друг от друга,
                            *  то все они помещаются сюда, например \c MULTIPLE_TARGETS). */
    QString relatedNodeName; /**< Имя второго узла, с которым связана ошибка. */
    QString path; /**< Путь для циклов/путь до файла, который не удалось открыть. */
    int line; /**< Номер строки, содержащей ошибку во входном файле (\c -1, если ошибка не относится к строке или нельзя однозначно утверждать принадлежность к какой-либо строке). */

    /*!
         *  \brief Генерирует сообщение об ошибке.
         *  \return Строка с текстом ошибки.
         *
         *  \pre Текущий объект \c Error должен содержать валидный тип ошибки \c type.
         *  \post Возвращает сформированную строку сообщения, при этом состояние объекта \c Error не изменяется.
         */
    QString generateMessage() const
    {
        switch (type)
        {
        case FILE_ERROR:
            return "Входной файл не найден или недоступен для чтения.";

        case OUTPUT_FILE_ERROR:
            return "Неверно указан файл для выходных данных. Возможно указанного расположения не существует или нет прав на запись.";

        case CYCLE_ERROR:
            return QString("Обнаружен цикл %1. Граф не является деревом.").arg(path);

        case FORBIDDEN_STRUCTURE_OR_FORM:
            return QString("Обнаружена запрещенная структура или сложный тип узла в строке %1. Используйте только плоский digraph и простые формы.").arg(line);

        case NO_TARGET:
            return "Не найдено целевого узла.";

        case CONNECTIVITY_ERROR:
            return QString("Нарушена связность. Нет связи между узлами %1 и %2.").arg(nodeName, relatedNodeName);

        case SYNTAX_ERROR:
            return QString("Синтаксическая ошибка в описании графа DOT в строке %1.").arg(line);

        case MULTIPLE_TARGETS:
            return QString("Обнаружено более одного целевого узла (Узлы %1). Допустим только один целевой узел.").arg(nodeName);

        case NOT_DESCENDANT:
            return QString("Узел %1 не является потомком целевого узла.").arg(nodeName);

        case REDUNDANT_NODE:
            return QString("Узел %1 избыточен (находится в поддереве уже отмеченного узла %2).").arg(nodeName, relatedNodeName);

        case LEAF_TARGET:
            return QString("Целевой узел %1 не может быть листом.").arg(nodeName);

        case MULTI_PARENT:
            return QString("Узел %1 имеет несколько родителей.").arg(nodeName);

        case NO_ROOT:
            return "Не удалось определить корень дерева.";

        case NO_MARKED_NODES:
            return "Нет отмеченных узлов покрытия.";
        }

        return "";
    }
    /*!
         * \brief Оператор для сравнения двух ошибок.
         * \param[in] other Ссылка на ошибку для сравнения.
         * \return \c true, если все поля совпадают; иначе - \c false.
         */
    bool operator==(const Error& other) const
    {
        return type == other.type &&
               nodeName == other.nodeName &&
               relatedNodeName == other.relatedNodeName &&
               path == other.path &&
               line == other.line;
    };
};

/*!
 * \brief Функция хэширования, для того, чтобы хранить \c QSet<Error>.
 * \param[in] key Объект ошибки, для которого считается хэш.
 * \param[in] seed Начальное значение хэш-функции.
 * \return Вычисленное значение.
 */
inline size_t qHash(const Error& key, size_t seed = 0)
{
    return qHash(static_cast<int>(key.type), seed) ^
           qHash(key.nodeName, seed) ^
           qHash(key.relatedNodeName, seed) ^
           qHash(key.path, seed) ^
           qHash(key.line, seed);
}

/*!
 * \brief Перечисление для возможных статусов покрытия узла.
 */
enum CoverageStatus
{
    NOT_COVERED,
    PARTIALLY_COVERED,
    FULLY_COVERED
};

/*!
 * \brief Структура с результатами парсинга входного файла (пустая,
 * если были встречены ошибки \c SYNTAX_ERROR или \c FORBIDDEN_STRUCTURE_OR_FORM, так как не можем гарантировать корректность данных).
 */
struct ParseResult
{
    Node* root; /**< Указатель на корень дерева. */
    QMap<QString, Node*> allNodes; /**< Словарь всех узлов дерева вида (Имя узла, указатель на узел). */
};

/*!
 * \brief Структура с результатом проверки покрытия.
 */
struct Result
{
    QVector<Node*> valid; /**< Список корректных отмеченных узлов (выполняют свою работу по покрытию целевого узла). */
    QVector<Node*> missing; /**< Список недостающих узлов для минимального покрытия целевого узла. */
};

#endif // STRUCTURES_H
