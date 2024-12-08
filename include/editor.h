#ifndef EDITOR_H
#define EDITOR_H

#include <QtWidgets/QPlainTextEdit>
#include <QtGui/QSyntaxHighlighter>
#include <QtGui/QUndoStack>
#include <QtGui/QUndoCommand>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtCore/QHash>
#include <QtCore/QVector>
#include "splitviewcontainer.h"

class LineNumberArea;
class SettingsDialog;
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class EditorToolBar;
class EditorContextMenu;

// Forward declare CodeEditor for TextEditCommand
class CodeEditor;

// Command classes for Undo/Redo
class TextEditCommand : public QUndoCommand
{
public:
    TextEditCommand(CodeEditor* editor, const QString& oldText, const QString& newText,
                   int position, int charsRemoved, int charsAdded);
    void undo() override;
    void redo() override;

private:
    CodeEditor* editor;
    QString oldText;
    QString newText;
    int position;
    int charsRemoved;
    int charsAdded;
};

struct FoldedRegion {
    int startBlock;
    int endBlock;
    QString placeholder;
    bool isCollapsed;
};

struct Cursor {
    QTextCursor cursor;
    bool isMain;
    bool isColumnMode;
};

struct Selection {
    int startPosition;
    int endPosition;
    bool isColumnMode;
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth() const;
    QUndoStack* undoStack() const { return m_undoStack; }
    
    // Split view related
    void setSplitContainer(SplitViewContainer* container);
    SplitViewContainer* splitContainer() const { return splitViewContainer; }
    void splitHorizontally();
    void splitVertically();
    void closeSplit();

    // Toolbar and context menu
    EditorToolBar* toolbar() const { return m_toolbar; }
    EditorContextMenu* contextMenu() const { return m_contextMenu; }
    void setToolbar(EditorToolBar* toolbar);
    void setContextMenu(EditorContextMenu* contextMenu);

    // Language support
    void setLanguage(const QString& language);
    QString currentLanguage() const { return m_currentLanguage; }

    // Selection operations
    void selectLine();
    void selectWord();
    void expandSelection();
    void shrinkSelection();

    // Add blockAtPosition declaration
    QTextBlock blockAtPosition(const QPoint &pos) const;

    // File operations
    void setFilePath(const QString& path) { filePath = path; emit filePathChanged(path); }
    QString getFilePath() const { return filePath; }

    // View operations
    void resetZoom();
    void showReplaceDialog();
    bool findNext() { return findNext(lastSearchText, lastCaseSensitive, lastWholeWords, true); }
    bool findPrevious() { return findPrevious(lastSearchText, lastCaseSensitive, lastWholeWords, true); }

signals:
    void filePathChanged(const QString& path);

public slots:
    void find(const QString &text, bool caseSensitive, bool wholeWords, bool searchBackwards, bool wrapAround);
    void replace(const QString &searchText, const QString &replaceText, bool caseSensitive, bool wholeWords);
    void replaceAll(const QString &searchText, const QString &replaceText, bool caseSensitive, bool wholeWords);
    void showFindDialog();
    void undo();
    void redo();
    void showSettingsDialog();
    void applySettings();
    void loadSettings();
    void checkAutoSave();
    void toggleFold();
    void foldAll();
    void unfoldAll();
    void updateFoldingRegions();
    void addCursorAbove();
    void addCursorBelow();
    void addCursorToWordOccurrences();
    void startColumnSelection();
    void clearAdditionalCursors();
    void addCursorAtMousePosition(const QPoint &pos);
    void updateSplitView();
    void setLineNumbersVisible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void insertFromMimeData(const QMimeData *source) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    bool findNext(const QString &searchText, bool caseSensitive, bool wholeWords, bool wrapAround);
    bool findPrevious(const QString &searchText, bool caseSensitive, bool wholeWords, bool wrapAround);
    void handleTextChanged(int position, int charsRemoved, int charsAdded);
    void highlightFoldingRegions();
    void updateCursors();
    void handleSelectionChanged();

private:
    QWidget *lineNumberArea;
    QSyntaxHighlighter *highlighter;
    QUndoStack *m_undoStack;
    QString m_lastText;
    bool m_isUndoRedoOperation;
    
    bool findText(const QString &searchText, bool caseSensitive, bool wholeWords, bool searchBackwards, bool wrapAround);
    Qt::CaseSensitivity getCaseSensitivity(bool caseSensitive) const;
    QTextDocument::FindFlags getSearchFlags(bool caseSensitive, bool wholeWords, bool searchBackwards) const;
    void setupUndoRedo();
    SettingsDialog *settingsDialog;
    QTimer *autoSaveTimer;
    QString currentFilePath;
    
    // Settings
    QFont editorFont;
    int tabSize;
    bool autoIndent;
    bool lineWrapping;
    QString themeName;
    QColor editorBackgroundColor;
    QColor editorForegroundColor;
    QColor lineNumberBackgroundColor;
    QColor lineNumberForegroundColor;
    QColor currentLineColor;
    bool autoSaveEnabled;
    int autoSaveInterval;
    
    // Folding related members
    QHash<int, FoldedRegion> foldedRegions;
    int foldingMarginWidth;
    bool isFoldingEnabled;
    QColor foldingMarkerColor;
    
    // Multiple cursor support
    QVector<Cursor> cursors;
    QVector<Selection> selections;
    bool isColumnSelectionMode;
    QPoint columnSelectionOrigin;
    
    SplitViewContainer* splitViewContainer;
    QString filePath;
    
    EditorToolBar* m_toolbar;
    EditorContextMenu* m_contextMenu;
    QString m_currentLanguage;
    
    void setupEditor();
    void setupAutoSave();
    void updateEditorColors();
    void updateEditorFont();
    void updateEditorSettings();
    void saveFile();
    void setupFolding();
    void detectFoldingRegions();
    bool isFoldableBlock(const QTextBlock &block) const;
    int getFoldingIndentLevel(const QTextBlock &block) const;
    void drawFoldingMarker(QPainter *painter, const QRect &rect, bool collapsed);
    void toggleFoldAt(const QTextBlock &block);
    void setBlockVisible(const QTextBlock &block, bool visible);
    QRect getFoldingMarkerRect(const QTextBlock &block) const;
    QString createPlaceholderText(const QTextBlock &startBlock, const QTextBlock &endBlock) const;
    void updateViewportMargins();
    void ensureBlockIsVisible(const QTextBlock &block);
    QTextBlock findFoldingEndBlock(const QTextBlock &startBlock) const;
    void setupMultipleCursors();
    void insertTextAtAllCursors(const QString &text);
    void removeTextAtCursors(bool isBackspace);
    void moveCursors(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    void mergeCursors();
    void sortCursors();
    bool areCursorsOverlapping(const QTextCursor &c1, const QTextCursor &c2) const;
    void updateColumnSelection(const QPoint &pos);
    QTextCursor createCursorForColumn(int blockNumber, int column);
    void addCursorAtWordOccurrence(const QString &word);
    void ensureVisibleCursors();
    void paintCursors(QPainter *painter);
    QRect cursorRect(const QTextCursor &cursor) const;
    void handleMultipleCursorKeyPress(QKeyEvent *event);
    void setupSplitView();

    // Selection helpers
    QTextCursor expandSelectionToBoundary(const QTextCursor& cursor);
    QTextCursor shrinkSelectionToBoundary(const QTextCursor& cursor);

    QString lastSearchText;
    bool lastCaseSensitive;
    bool lastWholeWords;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
    {}

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

#endif // EDITOR_H 