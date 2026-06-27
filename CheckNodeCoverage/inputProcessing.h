#ifndef INPUTPROCESSING_H
#define INPUTPROCESSING_H
#include "structures.h"
#include <QRegularExpression>

/*!
 * \file inputProcessing.h
 * \brief Функции для чтения, парсинга и проверки дерева \c DOT.
 */

/*!
 * \brief Построчно считывает файл в массив строк.
 * \param[in] filepath Путь к входному файлу.
 * \param[out] errors Набор ошибок.
 * \return Массив строк с содержимым файла.
 *
 * \pre Путь к файлу \c filepath не должен быть пустым.
 * \post Возвращает считанные строки в формате \c QStringList. При ошибке доступа к файлу в контейнер \c errors записывается \c FILE_ERROR.
 */
QStringList readFile(const QString& filepath, QSet<Error>& errors);

/*!
 * \brief Контекст одного вызова parseDOT.
 *
 * Собирает в одном месте всё состояние, которое в исходной функции было локальными переменными:
 * результат разбора, накопленные ошибки (по ссылке на внешний QSet), структуры для поиска родителей
 * и корня, а также флаги текущего состояния разбора.
 * Передаётся по ссылке во все подфункции вместо длинного списка параметров.
 */
struct ParseContext
{
    ParseResult res; /**< Результат разбора: дерево узлов и карта allNodes. */
    QSet<Error>& errors; /**< Ссылка на внешний контейнер ошибок. */
    QStringList multiParentOrder; /**< Имена узлов, у которых обнаружен второй родитель (в порядке обнаружения). */
    QMap<QString, QStringList> nodeAllParents; /**< Для каждого узла — список имён всех его родителей. */
    QStringList fileOrderNodes; /**< Имена узлов в порядке первого появления в файле. */
    int subgraphDepth = 0; /**< Текущая глубина вложенности фигурных скобок внутри запрещённой конструкции. */
    bool hasClosing = false; /**< true, если найдена закрывающая скобка основного графа. */
    bool hasSyntaxError = false; /**< true, если хотя бы одна синтаксическая ошибка зафиксирована. */
    bool hasForbiddenError = false; /**< true, если хотя бы одна запрещённая конструкция зафиксирована. */

    /*!
     * \brief Создаёт контекст разбора, связывая его с внешним контейнером ошибок.
     * \param[in,out] errs Внешний контейнер, в который будут записываться все ошибки разбора.
     */
    explicit ParseContext(QSet<Error>& errs);
};

/*!
 * \brief Статические регулярные выражения грамматики языка DOT.
 *
 * Вынесены в неймспейс, а не в \c ParseContext, так как описывают саму грамматику языка — это не часть изменяемого состояния одного разбора.
 */
namespace
{
    const QRegularExpression header(R"(^\s*digraph\s+\w+\s*\{\s*$)"); /**< Заголовок графа: digraph имя {. */
    const QRegularExpression closing(R"(^\s*\}\s*$)"); /**< Закрывающая скобка графа. */
    const QRegularExpression edgeChain(R"(^\s*\w+(?:\s*->\s*\w+)+\s*;\s*$)"); /**< Цепочка рёбер вида a -> b -> c;. */
    const QRegularExpression singleNode(R"(^\s*(\w+)(?:\s*\[.*\])?\s*;\s*$)"); /**< Объявление одного узла, опционально с атрибутами. */
    const QRegularExpression shapeValue(R"(shape\s*=\s*\"?(\w+)\"?)"); /**< Значение атрибута shape внутри квадратных скобок. */
    const QRegularExpression forbidden(R"(^\s*(subgraph|cluster|graph|node|edge)\b)"); /**< Запрещённые ключевые слова в начале строки. */
}

/*!
 * \brief Находит и валидирует заголовок графа.
 *
 * Пропускает пустые строки в начале файла. Первая непустая строка обязана быть валидным заголовком;
 * если это не так, фиксируется синтаксическая ошибка на этой строке.
 *
 * \param[in] lines Все строки входного файла.
 * \param[out] errors Контейнер ошибок, в который добавляется SYNTAX_ERROR, если первая непустая строка
 * не является заголовком.
 * \return Индекс строки заголовка в \c lines, либо -1, если корректный заголовок не найден.
 */
int validateHeader(const QStringList& lines, QSet<Error>& errors);

/*!
 * \brief Регистрирует связь "родитель -> ребёнок" между двумя узлами по их именам.
 *
 * Фиксирует оба имени в порядке появления в файле, создаёт узлы при первом упоминании, обновляет список
 * родителей ребёнка и проставляет связь parent/children, если у ребёнка ещё нет родителя. Если у родителя
 * уже есть такой ребёнок — фиксирует синтаксическую ошибку, так как граф не является деревом.
 *
 * \param[in,out] ctx Контекст разбора.
 * \param[in] parentName Имя узла-родителя.
 * \param[in] childName Имя узла-ребёнка.
 * \param[in] lineNumber Номер строки исходного файла, используется при формировании ошибок.
 */
void linkParentChild(ParseContext& ctx, const QString& parentName, const QString& childName, int lineNumber);

/*!
 * \brief Разбирает строку с цепочкой рёбер вида a -> b -> c; и связывает все пары.
 *
 * Удаляет из строки точку с запятой и пробелы, разбивает по стрелке и последовательно вызывает
 * \c linkParentChild для каждой соседней пары имён в цепочке.
 *
 * \param[in,out] ctx Контекст разбора (передаётся в \c linkParentChild для каждой пары).
 * \param[in] line Строка файла, уже прошедшая проверку \c edgeChain.
 * \param[in] lineNumber Номер строки исходного файла.
 */
void processEdgeChain(ParseContext& ctx, const QString& line, int lineNumber);

/*!
 * \brief Разбирает значение атрибута shape и присваивает узлу соответствующую форму.
 *
 * Если атрибут shape отсутствует в строке либо его значение не равно "diamond" или "rectangle" —
 * узлу присваивается форма DEFAULT и фиксируется ошибка FORBIDDEN_STRUCTURE_OR_FORM.
 *
 * \param[in,out] ctx Контекст разбора.
 * \param[in,out] node Указатель на узел, которому присваивается форма.
 * \param[in] nodeName Имя узла — используется только для текста ошибки.
 * \param[in] line Строка файла, содержащая объявление узла с атрибутами в квадратных скобках.
 * \param[in] lineNumber Номер строки исходного файла.
 */
void parseShapeAttribute(ParseContext& ctx, Node* node, const QString& nodeName, const QString& line, int lineNumber);

/*!
 * \brief Обрабатывает строку с объявлением одного узла, опционально с атрибутами.
 *
 * Регистрирует узел и фиксирует порядок его появления в файле. Если строка содержит квадратные скобки,
 * делегирует разбор атрибута формы в \c parseShapeAttribute.
 *
 * \param[in,out] ctx Контекст разбора.
 * \param[in] nodeMatch Результат успешного сопоставления строки с \c singleNode.
 * \param[in] line Полная строка файла.
 * \param[in] lineNumber Номер строки исходного файла.
 */
void processSingleNode(ParseContext& ctx, const QRegularExpressionMatch& nodeMatch, const QString& line, int lineNumber);

/*!
 * \brief Обрабатывает строку, которая может быть запрещённой конструкцией либо частью уже открытой вложенности.
 *
 * Если строка начинается с запрещённого ключевого слова — фиксирует ошибку FORBIDDEN_STRUCTURE_OR_FORM
 * и входит в режим пропуска вложенности. Если разбор уже находится внутри такой вложенности — обновляет
 * счётчик глубины без новой ошибки.
 *
 * \param[in,out] ctx Контекст разбора.
 * \param[in] line Строка файла после удаления пробелов по краям.
 * \param[in] lineNumber Номер строки исходного файла.
 * \return true, если строка обработана как запрещённая конструкция или часть вложенности и дальнейшая
 * обработка не требуется; false, если строку нужно обрабатывать дальше как ребро, узел или ошибку.
 */
bool tryProcessForbiddenOrNested(ParseContext& ctx, const QString& line, int lineNumber);

/*!
 * \brief Обрабатывает одну непустую строку тела графа — диспетчер строки.
 *
 * Порядок проверок: закрывающая скобка верхнего уровня, запрещённая конструкция или вложенность,
 * цепочка рёбер, объявление одного узла, иначе — синтаксическая ошибка.
 *
 * \param[in,out] ctx Контекст разбора.
 * \param[in] line Строка файла после удаления пробелов по краям, гарантированно непустая.
 * \param[in] lineNumber Номер строки исходного файла.
 */
void processBodyLine(ParseContext& ctx, const QString& line, int lineNumber);

/*!
 * \brief Проходит по всем строкам тела графа после строки заголовка.
 *
 * \param[in,out] ctx Контекст разбора, накопительно изменяется на протяжении всего проход.
 * \param[in] lines Все строки входного файла.
 * \param[in] headerLineIdx Индекс строки заголовка; обработка тела начинается со следующей строки.
 */
void parseBody(ParseContext& ctx, const QStringList& lines, int headerLineIdx);

/*!
 * \brief Находит корень дерева — первый по порядку появления в файле узел без родителя.
 *
 * \param[in] ctx Контекст разбора.
 * \return Указатель на первый найденный узел без родителя, либо nullptr, если такого узла не нашлось.
 */
Node* findRoot(const ParseContext& ctx);

/*!
 * \brief Результаты подсчёта узлов по форме.
 *
 * Используется вместо нескольких выходных параметров \c countShapes.
 */
struct ShapeCounts
{
    int targetCount = 0; /**< Количество узлов с формой DIAMOND (целевых узлов). */
    int markedCount = 0; /**< Количество узлов с формой RECTANGLE (отмеченных узлов). */
    QStringList targetNames; /**< Имена всех целевых узлов, в порядке обхода allNodes. */
    QVector<Node*> targetNodes; /**< Указатели на все целевые узлы, в том же порядке. */
};

/*!
 * \brief Подсчитывает количество узлов с формами DIAMOND и RECTANGLE.
 *
 * \param[in] allNodes Карта "имя узла → указатель на узел" из ParseResult. Допускаются значения nullptr.
 * \return Структура ShapeCounts с подсчитанными значениями. Списки targetNames и targetNodes
 * находятся в одном и том же порядке.
 */
ShapeCounts countShapes(const QMap<QString, Node*>& allNodes);

/*!
 * \brief Формирует ошибки MULTI_PARENT для всех узлов с более чем одним родителем.
 *
 * \param[in,out] ctx Контекст разбора.
 */
void appendMultiParentErrors(ParseContext& ctx);

/*!
 * \brief Формирует ошибки по результатам подсчёта форм узлов.
 *
 * Проверяет отсутствие или множественность целевых узлов, отсутствие отмеченных узлов и наличие
 * целевых узлов, являющихся листьями.
 *
 * \param[out] errors Контейнер ошибок, в который добавляются результаты проверок.
 * \param[in] counts Результат countShapes для текущего графа.
 */
void appendShapeAnalysisErrors(QSet<Error>& errors, const ShapeCounts& counts);

/*!
 * \brief Анализирует построенный граф в случае, когда есть хотя бы один узел.
 *
 * Регистрирует ошибки MULTI_PARENT, считает формы узлов, определяет корень дерева и, если
 * не было запрещённых конструкций и синтаксических ошибок, добавляет ошибки по результатам
 * анализа форм.
 *
 * \param[in,out] ctx Контекст разбора.
 */
void analyzeNonEmptyGraph(ParseContext& ctx);

/*!
 * \brief Анализирует случай полностью пустого графа.
 *
 * Если не зафиксировано ни синтаксических, ни запрещённых ошибок — граф пуст легитимно,
 * и в этом случае требуется сообщить об отсутствии целевого и отмеченных узлов.
 *
 * \param[in,out] ctx Контекст разбора.
 */
void analyzeEmptyGraph(ParseContext& ctx);

/*!
 * \brief Разбирает текстовое представление DOT-файла в дерево узлов.
 *
 * Проверяет, что файл не пуст, валидирует заголовок графа,
 * построчно разбирает тело графа, проверяет наличие закрывающей скобки и анализирует
 * построенный граф. Если зафиксирована хотя бы одна синтаксическая ошибка, построенная
 * структура графа не гарантированно корректна и удаляется целиком.
 *
 * \param[in] lines Строки входного DOT-файла.
 * \param[out] errors Контейнер, в который будут записаны все найденные ошибки.
 * Очищается в начале вызова.
 * \return Результат разбора: построенное дерево узлов, либо пустой ParseResult, если
 * разбор не удался.
 */
ParseResult parseDOT(const QStringList& lines, QSet<Error>& errors);

/*!
 * \brief Рекурсивная функция для поиска циклов в графе.
 * \param[in] node Узел, с которого начинается проверка.
 * \param[in,out] visited Множество уже посещенных узлов в ходе проверки на циклы.
 * \param[in,out] stack Стек для хранения пути цикла.
 * \param[out] errors Набор ошибок.
 *
 * \pre Указатель \c node должен быть валидным и не равным \c nullptr.
 * \post Обходит граф в глубину. При обнаружении цикла записывает ошибку \c CYCLE_ERROR в набор \c errors и формирует путь в \c stack.
 */
void dfsValidate(Node* node, QSet<Node*>& visited, QStack<Node*>& stack, QSet<Error>& errors);

/*!
 * \brief Ищет локальный корень компоненты.
 * \param[in] node Узел, с которого начинается поиск корня компоненты.
 * \param[in] visited Множество посещенных узлов в ходе проверки на циклы.
 * \param[out] outRoot Указатель на корень локальной компоненты.
 * \return \c true, если найден корень компоненты; если компонента состоит из цикла - \c false.
 *
 * \pre Указатель \c node не должен быть равен \c nullptr.
 * \post Записывает адрес найденного корня компоненты в \c outRoot, возвращая результат успешности поиска.
 */
bool findComponentRoot(Node* node, const QSet<Node*>& visited, Node*& outRoot);

/*!
 * \brief Проверяет граф на циклы и связность.
 * \param[in] root Указатель на корень (\c nullptr, если в ходе парсинга не было найдено корня).
 * \param[out] errors Набор ошибок.
 * \param[in] allNodes Словарь всех узлов.
 *
 * \pre Контейнер \c allNodes должен содержать все распарсенные узлы графа для проверки связности.
 * \post Граф полностью проверен на ошибки топологии (циклы, множественные родители, отсутствие корня). Все найденные несоответствия занесены в \c errors.
 */
void validateStructure(Node* root, QSet<Error>& errors, const QMap<QString, Node*>& allNodes);

#endif // INPUTPROCESSING_H
