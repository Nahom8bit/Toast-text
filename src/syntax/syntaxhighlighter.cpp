#include "syntax/syntaxhighlighter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent),
      multiLineCommentStartIndex(-1),
      multiLineCommentLength(0),
      isInMultiLineComment(false)
{
    setupFormats();
    createLanguageMap();
    
    // Set up common regular expressions
    numberRegex.setPattern("\\b\\d+(\\.\\d+)?([eE][+-]?\\d+)?\\b");
    functionRegex.setPattern("\\b[A-Za-z0-9_]+(?=\\s*\\()");
    operatorRegex.setPattern("[+\\-*/%=<>!&|^~]|\\b(and|or|not)\\b");
}

void SyntaxHighlighter::setupFormats()
{
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(Qt::darkBlue);

    typeFormat.setFontWeight(QFont::Bold);
    typeFormat.setForeground(Qt::darkMagenta);

    literalFormat.setForeground(Qt::darkRed);

    builtinFormat.setFontWeight(QFont::Bold);
    builtinFormat.setForeground(Qt::darkCyan);

    commentFormat.setForeground(Qt::darkGreen);
    commentFormat.setFontItalic(true);

    quotationFormat.setForeground(Qt::darkRed);

    functionFormat.setFontWeight(QFont::Bold);
    functionFormat.setForeground(Qt::blue);

    operatorFormat.setForeground(Qt::darkGray);

    numberFormat.setForeground(Qt::darkRed);
}

void SyntaxHighlighter::createLanguageMap()
{
    extensionToLanguage = {
        {"cpp", "C++"},
        {"h", "C++"},
        {"hpp", "C++"},
        {"c", "C"},
        {"py", "Python"},
        {"js", "JavaScript"},
        {"ts", "TypeScript"},
        {"java", "Java"},
        {"rb", "Ruby"},
        {"php", "PHP"},
        {"cs", "CSharp"},
        {"go", "Go"},
        {"rs", "Rust"},
        {"swift", "Swift"},
        {"kt", "Kotlin"},
        {"scala", "Scala"},
        {"html", "HTML"},
        {"css", "CSS"},
        {"json", "JSON"},
        {"xml", "XML"},
        {"yaml", "YAML"},
        {"md", "Markdown"}
    };
}

void SyntaxHighlighter::setLanguage(const QString &extension)
{
    QString language = extensionToLanguage.value(extension.toLower(), "Text");
    loadLanguageDefinition(language);
    rehighlight();
}

void SyntaxHighlighter::loadLanguageDefinition(const QString &language)
{
    // Clear current language definition
    currentLanguage = LanguageDefinition();
    currentLanguage.name = language;

    // Load language definition from JSON file
    QString filename = QString(":/syntax/%1.json").arg(language.toLower());
    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    // Load keywords
    QJsonArray keywords = root["keywords"].toArray();
    for (const QJsonValue &keyword : keywords) {
        currentLanguage.keywords.append(keyword.toString());
    }

    // Load types
    QJsonArray types = root["types"].toArray();
    for (const QJsonValue &type : types) {
        currentLanguage.types.append(type.toString());
    }

    // Load literals
    QJsonArray literals = root["literals"].toArray();
    for (const QJsonValue &literal : literals) {
        currentLanguage.literals.append(literal.toString());
    }

    // Load builtin functions/objects
    QJsonArray builtins = root["builtins"].toArray();
    for (const QJsonValue &builtin : builtins) {
        currentLanguage.builtin.append(builtin.toString());
    }

    // Load comment definitions
    currentLanguage.singleLineComment = root["singleLineComment"].toString();
    currentLanguage.multiLineCommentStart = root["multiLineCommentStart"].toString();
    currentLanguage.multiLineCommentEnd = root["multiLineCommentEnd"].toString();

    // Load string delimiters
    currentLanguage.stringDelimiter = root["stringDelimiter"].toString();
    currentLanguage.charDelimiter = root["charDelimiter"].toString();

    // Load custom rules
    QJsonArray rules = root["rules"].toArray();
    for (const QJsonValue &ruleValue : rules) {
        QJsonObject ruleObj = ruleValue.toObject();
        SyntaxRule rule;
        rule.pattern.setPattern(ruleObj["pattern"].toString());
        rule.multiLine = ruleObj["multiLine"].toBool();
        if (rule.multiLine) {
            rule.startPattern = ruleObj["startPattern"].toString();
            rule.endPattern = ruleObj["endPattern"].toString();
        }
        
        // Set format based on type
        QString type = ruleObj["type"].toString();
        if (type == "keyword") rule.format = keywordFormat;
        else if (type == "type") rule.format = typeFormat;
        else if (type == "literal") rule.format = literalFormat;
        else if (type == "builtin") rule.format = builtinFormat;
        else if (type == "comment") rule.format = commentFormat;
        else if (type == "string") rule.format = quotationFormat;
        else if (type == "function") rule.format = functionFormat;
        else if (type == "operator") rule.format = operatorFormat;
        else if (type == "number") rule.format = numberFormat;

        currentLanguage.rules.append(rule);
    }
}

void SyntaxHighlighter::updateTheme(const QColor &defaultForeground)
{
    // Update format colors based on theme
    keywordFormat.setForeground(defaultForeground.darker(150));
    typeFormat.setForeground(defaultForeground.darker(130));
    literalFormat.setForeground(defaultForeground.darker(120));
    builtinFormat.setForeground(defaultForeground.darker(140));
    commentFormat.setForeground(defaultForeground.lighter(150));
    quotationFormat.setForeground(defaultForeground.darker(110));
    functionFormat.setForeground(defaultForeground.darker(160));
    operatorFormat.setForeground(defaultForeground.darker(120));
    numberFormat.setForeground(defaultForeground.darker(110));

    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    // Handle multi-line comments
    highlightMultiLineComments(text);
    if (isInMultiLineComment) {
        return;
    }

    // Apply single-line comment highlighting
    if (!currentLanguage.singleLineComment.isEmpty()) {
        int commentStart = text.indexOf(currentLanguage.singleLineComment);
        if (commentStart >= 0) {
            setFormat(commentStart, text.length() - commentStart, commentFormat);
            return;
        }
    }

    // Apply custom rules
    for (const SyntaxRule &rule : currentLanguage.rules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Apply common highlighting patterns
    // Numbers
    QRegularExpressionMatchIterator numberMatches = numberRegex.globalMatch(text);
    while (numberMatches.hasNext()) {
        QRegularExpressionMatch match = numberMatches.next();
        setFormat(match.capturedStart(), match.capturedLength(), numberFormat);
    }

    // Functions
    QRegularExpressionMatchIterator functionMatches = functionRegex.globalMatch(text);
    while (functionMatches.hasNext()) {
        QRegularExpressionMatch match = functionMatches.next();
        setFormat(match.capturedStart(), match.capturedLength(), functionFormat);
    }

    // Operators
    QRegularExpressionMatchIterator operatorMatches = operatorRegex.globalMatch(text);
    while (operatorMatches.hasNext()) {
        QRegularExpressionMatch match = operatorMatches.next();
        setFormat(match.capturedStart(), match.capturedLength(), operatorFormat);
    }

    // Keywords
    for (const QString &keyword : currentLanguage.keywords) {
        QRegularExpression keywordRegex("\\b" + keyword + "\\b");
        QRegularExpressionMatchIterator matches = keywordRegex.globalMatch(text);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            setFormat(match.capturedStart(), match.capturedLength(), keywordFormat);
        }
    }

    // Types
    for (const QString &type : currentLanguage.types) {
        QRegularExpression typeRegex("\\b" + type + "\\b");
        QRegularExpressionMatchIterator matches = typeRegex.globalMatch(text);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            setFormat(match.capturedStart(), match.capturedLength(), typeFormat);
        }
    }

    // Builtin functions/objects
    for (const QString &builtin : currentLanguage.builtin) {
        QRegularExpression builtinRegex("\\b" + builtin + "\\b");
        QRegularExpressionMatchIterator matches = builtinRegex.globalMatch(text);
        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            setFormat(match.capturedStart(), match.capturedLength(), builtinFormat);
        }
    }
}

void SyntaxHighlighter::highlightMultiLineComments(const QString &text)
{
    if (currentLanguage.multiLineCommentStart.isEmpty() || currentLanguage.multiLineCommentEnd.isEmpty()) {
        return;
    }

    int startIndex = 0;
    int blockState = previousBlockState();
    
    // If we're continuing a multi-line comment from the previous block
    if (blockState == 1) {
        isInMultiLineComment = true;
        int endIndex = text.indexOf(currentLanguage.multiLineCommentEnd);
        if (endIndex == -1) {
            // Comment continues to next block
            setFormat(0, text.length(), commentFormat);
            setCurrentBlockState(1);
            return;
        }
        // Found end of comment
        endIndex += currentLanguage.multiLineCommentEnd.length();
        setFormat(0, endIndex, commentFormat);
        startIndex = endIndex;
        isInMultiLineComment = false;
    }

    // Look for new multi-line comments in this block
    while (startIndex >= 0) {
        int commentStart = text.indexOf(currentLanguage.multiLineCommentStart, startIndex);
        if (commentStart == -1) {
            break;
        }

        int commentEnd = text.indexOf(currentLanguage.multiLineCommentEnd, commentStart + currentLanguage.multiLineCommentStart.length());
        int commentLength;

        if (commentEnd == -1) {
            // Comment continues to next block
            setCurrentBlockState(1);
            commentLength = text.length() - commentStart;
            isInMultiLineComment = true;
        } else {
            commentLength = commentEnd - commentStart + currentLanguage.multiLineCommentEnd.length();
            isInMultiLineComment = false;
        }

        setFormat(commentStart, commentLength, commentFormat);
        startIndex = (commentEnd == -1) ? -1 : commentEnd + currentLanguage.multiLineCommentEnd.length();
    }
} 