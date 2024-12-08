#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>
#include <QtGui/QTextCharFormat>
#include <QtCore/QRegularExpression>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

class SyntaxRule
{
public:
    QRegularExpression pattern;
    QTextCharFormat format;
    bool multiLine;
    QString startPattern;
    QString endPattern;
};

class LanguageDefinition
{
public:
    QString name;
    QVector<SyntaxRule> rules;
    QStringList keywords;
    QStringList types;
    QStringList literals;
    QStringList builtin;
    QStringList comments;
    QString multiLineCommentStart;
    QString multiLineCommentEnd;
    QString singleLineComment;
    QString stringDelimiter;
    QString charDelimiter;
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);
    void setLanguage(const QString &extension);
    void updateTheme(const QColor &defaultForeground);

protected:
    void highlightBlock(const QString &text) override;

private:
    void loadLanguageDefinition(const QString &language);
    void setupFormats();
    void highlightMultiLineComments(const QString &text);
    void createLanguageMap();
    
    LanguageDefinition currentLanguage;
    QHash<QString, QString> extensionToLanguage;
    
    // Format definitions
    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat literalFormat;
    QTextCharFormat builtinFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat numberFormat;
    
    // State for multi-line constructs
    int multiLineCommentStartIndex;
    int multiLineCommentLength;
    bool isInMultiLineComment;
    
    // Common regular expressions
    QRegularExpression numberRegex;
    QRegularExpression functionRegex;
    QRegularExpression operatorRegex;
};

#endif // SYNTAXHIGHLIGHTER_H 