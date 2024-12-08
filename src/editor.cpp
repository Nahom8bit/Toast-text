#include "editor.h"
#include "toolbar.h"
#include "contextmenu.h"
#include "splitviewcontainer.h"
#include "dialogs/finddialog.h"
#include "dialogs/settingsdialog.h"
#include "syntax/syntaxhighlighter.h"
#include <QTextBlock>
#include <QPainter>
#include <QTextCursor>
#include <QStack>
#include <QDebug>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QMenu>
#include <QContextMenuEvent>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QtWidgets>
#include <QtGui>
#include <QtCore>

TextEditCommand::TextEditCommand(CodeEditor* editor, const QString& oldText, const QString& newText,
                               int position, int charsRemoved, int charsAdded)
    : editor(editor), oldText(oldText), newText(newText),
      position(position), charsRemoved(charsRemoved), charsAdded(charsAdded)
{
}

void TextEditCommand::undo()
{
    QTextCursor cursor = editor->textCursor();
    cursor.setPosition(position);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, charsAdded);
    cursor.insertText(oldText);
}

void TextEditCommand::redo()
{
    QTextCursor cursor = editor->textCursor();
    cursor.setPosition(position);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, charsRemoved);
    cursor.insertText(newText);
}

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent), m_isUndoRedoOperation(false),
      isColumnSelectionMode(false), splitViewContainer(nullptr)
{
    lineNumberArea = new LineNumberArea(this);
    highlighter = new SyntaxHighlighter(document());
    m_undoStack = new QUndoStack(this);
    settingsDialog = nullptr;
    autoSaveTimer = new QTimer(this);

    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);
    
    // Connect text change signals for undo/redo
    connect(document(), &QTextDocument::contentsChange,
            this, &CodeEditor::handleTextChanged);
    
    foldingMarginWidth = 20;
    isFoldingEnabled = true;
    foldingMarkerColor = QColor(Qt::darkGray);
    
    setupEditor();
    setupUndoRedo();
    setupAutoSave();
    setupFolding();
    setupMultipleCursors();
    setupSplitView();
    loadSettings();
    
    m_lastText = toPlainText();
}

CodeEditor::~CodeEditor()
{
    if (settingsDialog) {
        delete settingsDialog;
    }
}

void CodeEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    
    // Paint the base text editor
    QPlainTextEdit::paintEvent(event);
    
    // Paint folding markers
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            if (isFoldableBlock(block)) {
                bool isCollapsed = foldedRegions.contains(blockNumber);
                QRect rect = getFoldingMarkerRect(block);
                drawFoldingMarker(&painter, rect, isCollapsed);
            }
        }
        
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
    
    // Paint multiple cursors
    paintCursors(&painter);
}

void CodeEditor::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QTextBlock block = blockAtPosition(event->pos());
        if (block.isValid()) {
            int blockNumber = block.blockNumber();
            QRect rect = getFoldingMarkerRect(block);
            
            if (rect.contains(event->pos()) && isFoldableBlock(block)) {
                toggleFoldAt(block);
                return;
            }
        }
        
        if (event->modifiers() & Qt::AltModifier) {
            startColumnSelection();
            columnSelectionOrigin = event->pos();
        } else if (event->modifiers() & Qt::ControlModifier) {
            addCursorAtMousePosition(event->pos());
        } else {
            clearAdditionalCursors();
            QPlainTextEdit::mousePressEvent(event);
        }
    } else {
        QPlainTextEdit::mousePressEvent(event);
    }
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

bool CodeEditor::findNext(const QString &searchText, bool caseSensitive,
                         bool wholeWords, bool wrapAround)
{
    QTextDocument::FindFlags flags = getSearchFlags(caseSensitive, wholeWords, false);
    QTextCursor cursor = textCursor();
    QTextCursor found = document()->find(searchText, cursor, flags);

    if (found.isNull() && wrapAround) {
        cursor.movePosition(QTextCursor::Start);
        found = document()->find(searchText, cursor, flags);
    }

    if (!found.isNull()) {
        setTextCursor(found);
        return true;
    }

    return false;
}

bool CodeEditor::findPrevious(const QString &searchText, bool caseSensitive,
                            bool wholeWords, bool wrapAround)
{
    QTextDocument::FindFlags flags = getSearchFlags(caseSensitive, wholeWords, true);
    QTextCursor cursor = textCursor();
    QTextCursor found = document()->find(searchText, cursor, flags);

    if (found.isNull() && wrapAround) {
        cursor.movePosition(QTextCursor::End);
        found = document()->find(searchText, cursor, flags);
    }

    if (!found.isNull()) {
        setTextCursor(found);
        return true;
    }

    return false;
}

QTextBlock CodeEditor::blockAtPosition(const QPoint &pos) const
{
    QTextCursor cursor = cursorForPosition(pos);
    return cursor.block();
}

void CodeEditor::setupSplitView()
{
    // Initial setup for split view functionality
    setFocusPolicy(Qt::StrongFocus);
}

void CodeEditor::setSplitContainer(SplitViewContainer* container)
{
    if (splitViewContainer != container) {
        splitViewContainer = container;
        updateSplitView();
    }
}

void CodeEditor::splitHorizontally()
{
    if (splitViewContainer) {
        splitViewContainer->splitView(this, SplitViewContainer::SplitOrientation::Horizontal);
    }
}

void CodeEditor::splitVertically()
{
    if (splitViewContainer) {
        splitViewContainer->splitView(this, SplitViewContainer::SplitOrientation::Vertical);
    }
}

void CodeEditor::closeSplit()
{
    if (splitViewContainer) {
        splitViewContainer->removeSplit(this);
    }
}

void CodeEditor::updateSplitView()
{
    if (splitViewContainer) {
        splitViewContainer->updateCurrentEditor(this);
    }
}

void CodeEditor::focusInEvent(QFocusEvent *event)
{
    QPlainTextEdit::focusInEvent(event);
    if (splitViewContainer) {
        splitViewContainer->updateCurrentEditor(this);
    }
}

void CodeEditor::resetZoom()
{
    QFont f = font();
    f.setPointSize(editorFont.pointSize());
    setFont(f);
}

void CodeEditor::showReplaceDialog()
{
    FindDialog *dialog = new FindDialog(this);
    dialog->show();
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), lineNumberBackgroundColor);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    int currentLine = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(lineNumberForegroundColor);
            painter.drawText(0, top, lineNumberArea->width() - 10, fontMetrics().height(),
                           Qt::AlignRight, number);
            
            // Draw blue glow dot for current line
            if (blockNumber == currentLine) {
                int dotSize = 6;
                int dotX = lineNumberArea->width() - dotSize - 2;
                int dotY = top + (fontMetrics().height() - dotSize) / 2;
                
                // Draw glow effect
                painter.setPen(Qt::NoPen);
                QRadialGradient gradient(dotX + dotSize/2, dotY + dotSize/2, dotSize * 1.5);
                gradient.setColorAt(0, QColor(0, 120, 255, 120));
                gradient.setColorAt(1, QColor(0, 120, 255, 0));
                painter.setBrush(gradient);
                painter.drawEllipse(dotX - dotSize/2, dotY - dotSize/2, dotSize * 2, dotSize * 2);
                
                // Draw dot
                painter.setBrush(QColor(0, 120, 255));
                painter.drawEllipse(dotX, dotY, dotSize, dotSize);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    setViewportMargins(lineNumberAreaWidth() + foldingMarginWidth, 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::highlightCurrentLine()
{
    // Remove the yellow highlight since we're using the blue dot
    QList<QTextEdit::ExtraSelection> extraSelections;
    setExtraSelections(extraSelections);
    
    // Update the line number area to show the blue dot
    lineNumberArea->update();
}

void CodeEditor::handleTextChanged(int position, int charsRemoved, int charsAdded)
{
    // Only handle undo/redo operations
    if (!m_isUndoRedoOperation) {
        QString oldText = m_lastText.mid(position, charsRemoved);
        QString newText = document()->toPlainText().mid(position, charsAdded);
        
        // Avoid creating undo commands for single character inputs (handled by keyPressEvent)
        if (charsAdded > 1 || charsRemoved > 0) {
            m_undoStack->push(new TextEditCommand(this, oldText, newText, position, charsRemoved, charsAdded));
        }
    }
    m_lastText = document()->toPlainText();
}

void CodeEditor::setupEditor()
{
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
    setMouseTracking(true);
}

void CodeEditor::setupUndoRedo()
{
    m_undoStack->setUndoLimit(1000);
}

void CodeEditor::setupAutoSave()
{
    connect(autoSaveTimer, &QTimer::timeout, this, &CodeEditor::checkAutoSave);
    autoSaveTimer->start(autoSaveInterval * 1000);
}

void CodeEditor::setupFolding()
{
    foldedRegions.clear();
    updateFoldingRegions();
}

void CodeEditor::setupMultipleCursors()
{
    cursors.clear();
    selections.clear();
    isColumnSelectionMode = false;
}

void CodeEditor::setLanguage(const QString& language)
{
    m_currentLanguage = language;
    if (highlighter) {
        static_cast<SyntaxHighlighter*>(highlighter)->setLanguage(language);
    }
}

void CodeEditor::selectLine()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
}

void CodeEditor::selectWord()
{
    QTextCursor cursor = textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    setTextCursor(cursor);
}

void CodeEditor::expandSelection()
{
    QTextCursor cursor = textCursor();
    cursor = expandSelectionToBoundary(cursor);
    setTextCursor(cursor);
}

void CodeEditor::shrinkSelection()
{
    QTextCursor cursor = textCursor();
    cursor = shrinkSelectionToBoundary(cursor);
    setTextCursor(cursor);
}

void CodeEditor::addCursorAbove()
{
    QTextCursor cursor = textCursor();
    if (cursor.block().previous().isValid()) {
        QTextCursor newCursor = cursor;
        newCursor.movePosition(QTextCursor::Up);
        cursors.append({newCursor, false, false});
        updateCursors();
    }
}

void CodeEditor::addCursorBelow()
{
    QTextCursor cursor = textCursor();
    if (cursor.block().next().isValid()) {
        QTextCursor newCursor = cursor;
        newCursor.movePosition(QTextCursor::Down);
        cursors.append({newCursor, false, false});
        updateCursors();
    }
}

void CodeEditor::addCursorToWordOccurrences()
{
    QTextCursor cursor = textCursor();
    QString word = cursor.selectedText();
    if (word.isEmpty()) {
        cursor.select(QTextCursor::WordUnderCursor);
        word = cursor.selectedText();
    }
    
    if (!word.isEmpty()) {
        addCursorAtWordOccurrence(word);
    }
}

void CodeEditor::startColumnSelection()
{
    isColumnSelectionMode = true;
    columnSelectionOrigin = mapFromGlobal(QCursor::pos());
}

void CodeEditor::clearAdditionalCursors()
{
    cursors.clear();
    selections.clear();
    isColumnSelectionMode = false;
    update();
}

void CodeEditor::addCursorAtMousePosition(const QPoint &pos)
{
    QTextCursor cursor = cursorForPosition(pos);
    cursors.append({cursor, false, false});
    updateCursors();
}

void CodeEditor::updateCursors()
{
    update();
}

void CodeEditor::handleSelectionChanged()
{
    if (isColumnSelectionMode) {
        updateColumnSelection(mapFromGlobal(QCursor::pos()));
    }
}

void CodeEditor::paintCursors(QPainter *painter)
{
    for (const Cursor &cursor : cursors) {
        QRect rect = cursorRect(cursor.cursor);
        painter->fillRect(rect, Qt::black);
    }
}

QTextCursor CodeEditor::expandSelectionToBoundary(const QTextCursor& cursor)
{
    QTextCursor expanded = cursor;
    if (!expanded.hasSelection()) {
        expanded.select(QTextCursor::WordUnderCursor);
    } else {
        expanded.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
        expanded.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
    return expanded;
}

QTextCursor CodeEditor::shrinkSelectionToBoundary(const QTextCursor& cursor)
{
    QTextCursor shrunk = cursor;
    if (shrunk.hasSelection()) {
        int start = shrunk.selectionStart();
        int end = shrunk.selectionEnd();
        shrunk.setPosition(start);
        shrunk.setPosition((start + end) / 2, QTextCursor::KeepAnchor);
    }
    return shrunk;
}

void CodeEditor::addCursorAtWordOccurrence(const QString &word)
{
    QTextCursor cursor = document()->find(word);
    while (!cursor.isNull()) {
        bool overlap = false;
        for (const Cursor &existing : cursors) {
            if (areCursorsOverlapping(cursor, existing.cursor)) {
                overlap = true;
                break;
            }
        }
        
        if (!overlap) {
            cursors.append({cursor, false, false});
        }
        
        cursor = document()->find(word, cursor);
    }
    updateCursors();
}

bool CodeEditor::areCursorsOverlapping(const QTextCursor &c1, const QTextCursor &c2) const
{
    int start1 = c1.selectionStart();
    int end1 = c1.selectionEnd();
    int start2 = c2.selectionStart();
    int end2 = c2.selectionEnd();
    
    return (start1 <= end2 && end1 >= start2);
}

void CodeEditor::updateColumnSelection(const QPoint &pos)
{
    int startBlock = cursorForPosition(columnSelectionOrigin).blockNumber();
    int endBlock = cursorForPosition(pos).blockNumber();
    int startColumn = columnSelectionOrigin.x();
    int endColumn = pos.x();
    
    cursors.clear();
    
    int minBlock = qMin(startBlock, endBlock);
    int maxBlock = qMax(startBlock, endBlock);
    
    for (int block = minBlock; block <= maxBlock; ++block) {
        cursors.append({createCursorForColumn(block, startColumn), false, true});
        QTextCursor &cursor = cursors.last().cursor;
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor,
                          (endColumn - startColumn) / fontMetrics().horizontalAdvance(' '));
    }
    
    update();
}

QTextCursor CodeEditor::createCursorForColumn(int blockNumber, int column)
{
    QTextBlock block = document()->findBlockByNumber(blockNumber);
    QTextCursor cursor(block);
    int spaces = column / fontMetrics().horizontalAdvance(' ');
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, spaces);
    return cursor;
}

void CodeEditor::ensureVisibleCursors()
{
    for (const Cursor &cursor : cursors) {
        QTextCursor mainCursor = textCursor();
        setTextCursor(cursor.cursor);
        ensureCursorVisible();
        setTextCursor(mainCursor);
    }
}

QRect CodeEditor::cursorRect(const QTextCursor &cursor) const
{
    QRect rect = QPlainTextEdit::cursorRect(cursor);
    rect.setWidth(2);
    return rect;
}

void CodeEditor::keyPressEvent(QKeyEvent *event)
{
    // Handle special keys first
    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
        if (event->modifiers() & Qt::ShiftModifier) {
            // Unindent
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            QString line = cursor.selectedText();
            if (line.startsWith("    ")) {
                cursor.removeSelectedText();
                cursor.insertText(line.mid(4));
            }
        } else {
            // Indent
            insertPlainText("    ");
        }
        event->accept();
        return;
    }

    // Handle multiple cursors if active
    if (!cursors.isEmpty()) {
        handleMultipleCursorKeyPress(event);
        event->accept();
        return;
    }

    // Handle normal key press
    if (!event->text().isEmpty() && !(event->modifiers() & Qt::ControlModifier)) {
        // For normal text input, handle it directly
        QPlainTextEdit::insertPlainText(event->text());
        event->accept();
    } else {
        // For special keys (navigation, shortcuts, etc.), use default handling
        QPlainTextEdit::keyPressEvent(event);
    }
}

void CodeEditor::inputMethodEvent(QInputMethodEvent *event)
{
    // Completely ignore input method events as we handle all input in keyPressEvent
    event->ignore();
}

void CodeEditor::handleMultipleCursorKeyPress(QKeyEvent *event)
{
    QString text = event->text();
    if (!text.isEmpty()) {
        for (Cursor &cursor : cursors) {
            cursor.cursor.insertText(text);
        }
    } else {
        switch (event->key()) {
            case Qt::Key_Backspace:
            case Qt::Key_Delete:
                for (Cursor &cursor : cursors) {
                    cursor.cursor.deleteChar();
                }
                break;
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Up:
            case Qt::Key_Down:
                QTextCursor::MoveOperation op;
                switch (event->key()) {
                    case Qt::Key_Left: op = QTextCursor::Left; break;
                    case Qt::Key_Right: op = QTextCursor::Right; break;
                    case Qt::Key_Up: op = QTextCursor::Up; break;
                    case Qt::Key_Down: op = QTextCursor::Down; break;
                    default: return;
                }
                moveCursors(op, event->modifiers() & Qt::ShiftModifier ?
                           QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
                break;
        }
    }
    update();
}

void CodeEditor::moveCursors(QTextCursor::MoveOperation op, QTextCursor::MoveMode mode)
{
    for (Cursor &cursor : cursors) {
        cursor.cursor.movePosition(op, mode);
    }
}

void CodeEditor::mergeCursors()
{
    for (int i = 0; i < cursors.size(); ++i) {
        for (int j = i + 1; j < cursors.size(); ++j) {
            if (areCursorsOverlapping(cursors[i].cursor, cursors[j].cursor)) {
                cursors.removeAt(j);
                --j;
            }
        }
    }
}

void CodeEditor::sortCursors()
{
    std::sort(cursors.begin(), cursors.end(),
              [](const Cursor &a, const Cursor &b) {
                  return a.cursor.position() < b.cursor.position();
              });
}

void CodeEditor::mouseMoveEvent(QMouseEvent *event)
{
    if (isColumnSelectionMode) {
        updateColumnSelection(event->pos());
    } else {
        QPlainTextEdit::mouseMoveEvent(event);
    }
}

void CodeEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if (isColumnSelectionMode) {
        isColumnSelectionMode = false;
    }
    QPlainTextEdit::mouseReleaseEvent(event);
}

void CodeEditor::contextMenuEvent(QContextMenuEvent* event)
{
    if (m_contextMenu) {
        m_contextMenu->exec(event->globalPos());
    } else {
        QPlainTextEdit::contextMenuEvent(event);
    }
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                    lineNumberAreaWidth() + foldingMarginWidth, cr.height()));
    updateViewportMargins();
}

void CodeEditor::insertFromMimeData(const QMimeData *source)
{
    if (source->hasText()) {
        insertTextAtAllCursors(source->text());
    } else {
        QPlainTextEdit::insertFromMimeData(source);
    }
}

void CodeEditor::find(const QString &text, bool caseSensitive, bool wholeWords,
                     bool searchBackwards, bool wrapAround)
{
    if (searchBackwards) {
        findPrevious(text, caseSensitive, wholeWords, wrapAround);
    } else {
        findNext(text, caseSensitive, wholeWords, wrapAround);
    }
    lastSearchText = text;
    lastCaseSensitive = caseSensitive;
    lastWholeWords = wholeWords;
}

void CodeEditor::replace(const QString &searchText, const QString &replaceText,
                        bool caseSensitive, bool wholeWords)
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == searchText) {
        cursor.insertText(replaceText);
    }
    findNext(searchText, caseSensitive, wholeWords, true);
}

void CodeEditor::replaceAll(const QString &searchText, const QString &replaceText,
                          bool caseSensitive, bool wholeWords)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    setTextCursor(cursor);
    
    QTextDocument::FindFlags flags = getSearchFlags(caseSensitive, wholeWords, false);
    int count = 0;
    
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = document()->find(searchText, cursor, flags);
        if (!cursor.isNull()) {
            cursor.insertText(replaceText);
            ++count;
        }
    }
}

void CodeEditor::showFindDialog()
{
    FindDialog *dialog = new FindDialog(this);
    connect(dialog, &FindDialog::findNext, this, [this, dialog]() {
        find(dialog->searchText(), dialog->caseSensitive(),
             dialog->wholeWords(), false, true);
    });
    connect(dialog, &FindDialog::findPrevious, this, [this, dialog]() {
        find(dialog->searchText(), dialog->caseSensitive(),
             dialog->wholeWords(), true, true);
    });
    connect(dialog, &FindDialog::replaceRequested, this, [this, dialog]() {
        replace(dialog->searchText(), dialog->replaceText(),
               dialog->caseSensitive(), dialog->wholeWords());
    });
    connect(dialog, &FindDialog::replaceAllRequested, this, [this, dialog]() {
        replaceAll(dialog->searchText(), dialog->replaceText(),
                  dialog->caseSensitive(), dialog->wholeWords());
    });
    dialog->show();
}

void CodeEditor::showSettingsDialog()
{
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(this);
    }
    settingsDialog->show();
}

void CodeEditor::applySettings()
{
    QSettings settings;
    
    // Font settings
    QString fontFamily = settings.value("editor/font/family", "Monospace").toString();
    int fontSize = settings.value("editor/font/size", 12).toInt();
    editorFont = QFont(fontFamily, fontSize);
    setFont(editorFont);
    
    // Editor settings
    tabSize = settings.value("editor/tabSize", 4).toInt();
    autoIndent = settings.value("editor/autoIndent", true).toBool();
    lineWrapping = settings.value("editor/lineWrapping", false).toBool();
    setLineWrapMode(lineWrapping ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    
    // Theme settings
    themeName = settings.value("editor/theme", "light").toString();
    editorBackgroundColor = settings.value("editor/colors/background", QColor(Qt::white)).value<QColor>();
    editorForegroundColor = settings.value("editor/colors/foreground", QColor(Qt::black)).value<QColor>();
    lineNumberBackgroundColor = settings.value("editor/colors/lineNumberBackground", QColor(Qt::lightGray)).value<QColor>();
    lineNumberForegroundColor = settings.value("editor/colors/lineNumberForeground", QColor(Qt::black)).value<QColor>();
    currentLineColor = settings.value("editor/colors/currentLine", QColor(Qt::yellow).lighter(160)).value<QColor>();
    
    // Auto-save settings
    autoSaveEnabled = settings.value("editor/autoSave/enabled", true).toBool();
    autoSaveInterval = settings.value("editor/autoSave/interval", 60).toInt();
    
    updateEditorColors();
    updateEditorFont();
    updateEditorSettings();
}

void CodeEditor::loadSettings()
{
    applySettings();
}

void CodeEditor::checkAutoSave()
{
    if (autoSaveEnabled && document()->isModified()) {
        saveFile();
    }
}

void CodeEditor::toggleFold()
{
    QTextBlock block = textCursor().block();
    if (isFoldableBlock(block)) {
        toggleFoldAt(block);
    }
}

void CodeEditor::foldAll()
{
    QTextBlock block = document()->firstBlock();
    while (block.isValid()) {
        if (isFoldableBlock(block) && !foldedRegions.contains(block.blockNumber())) {
            toggleFoldAt(block);
        }
        block = block.next();
    }
}

void CodeEditor::unfoldAll()
{
    QTextBlock block = document()->firstBlock();
    while (block.isValid()) {
        if (isFoldableBlock(block) && foldedRegions.contains(block.blockNumber())) {
            toggleFoldAt(block);
        }
        block = block.next();
    }
}

void CodeEditor::updateFoldingRegions()
{
    QTextBlock block = document()->firstBlock();
    while (block.isValid()) {
        if (isFoldableBlock(block)) {
            int blockNumber = block.blockNumber();
            if (!foldedRegions.contains(blockNumber)) {
                QTextBlock endBlock = findFoldingEndBlock(block);
                if (endBlock.isValid()) {
                    FoldedRegion region;
                    region.startBlock = blockNumber;
                    region.endBlock = endBlock.blockNumber();
                    region.placeholder = createPlaceholderText(block, endBlock);
                    region.isCollapsed = false;
                    foldedRegions.insert(blockNumber, region);
                }
            }
        }
        block = block.next();
    }
    update();
}

void CodeEditor::toggleFoldAt(const QTextBlock &block)
{
    int blockNumber = block.blockNumber();
    if (foldedRegions.contains(blockNumber)) {
        FoldedRegion &region = foldedRegions[blockNumber];
        region.isCollapsed = !region.isCollapsed;
        
        QTextBlock b = block.next();
        while (b.isValid() && b.blockNumber() <= region.endBlock) {
            setBlockVisible(b, !region.isCollapsed);
            b = b.next();
        }
        
        updateViewportMargins();
        update();
    }
}

void CodeEditor::setBlockVisible(const QTextBlock &block, bool visible)
{
    QTextLayout *layout = block.layout();
    if (layout) {
        layout->setCacheEnabled(visible);
        if (!visible) {
            QTextCursor cursor(block);
            cursor.clearSelection();
            setTextCursor(cursor);
        }
    }
}

QTextDocument::FindFlags CodeEditor::getSearchFlags(bool caseSensitive, bool wholeWords,
                                                  bool searchBackwards) const
{
    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (wholeWords)
        flags |= QTextDocument::FindWholeWords;
    if (searchBackwards)
        flags |= QTextDocument::FindBackward;
    return flags;
}

void CodeEditor::updateEditorColors()
{
    QPalette p = palette();
    p.setColor(QPalette::Base, editorBackgroundColor);
    p.setColor(QPalette::Text, editorForegroundColor);
    setPalette(p);
    
    highlightCurrentLine();
    update();
}

void CodeEditor::updateEditorFont()
{
    setFont(editorFont);
    updateLineNumberAreaWidth(0);
}

void CodeEditor::updateEditorSettings()
{
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * tabSize);
}

void CodeEditor::saveFile()
{
    if (!currentFilePath.isEmpty()) {
        QFile file(currentFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << toPlainText();
            document()->setModified(false);
        }
    }
}

bool CodeEditor::isFoldableBlock(const QTextBlock &block) const
{
    return getFoldingIndentLevel(block) < getFoldingIndentLevel(block.next());
}

int CodeEditor::getFoldingIndentLevel(const QTextBlock &block) const
{
    if (!block.isValid())
        return 0;
    
    QString text = block.text();
    int indent = 0;
    for (QChar c : text) {
        if (c == ' ')
            ++indent;
        else if (c == '\t')
            indent += tabSize;
        else
            break;
    }
    return indent;
}

void CodeEditor::drawFoldingMarker(QPainter *painter, const QRect &rect, bool collapsed)
{
    painter->save();
    painter->setPen(foldingMarkerColor);
    
    int size = 8;
    int x = rect.center().x() - size/2;
    int y = rect.center().y() - size/2;
    
    painter->drawRect(x, y, size, size);
    painter->drawLine(x + 2, y + size/2, x + size - 2, y + size/2);
    if (collapsed) {
        painter->drawLine(x + size/2, y + 2, x + size/2, y + size - 2);
    }
    
    painter->restore();
}

QRect CodeEditor::getFoldingMarkerRect(const QTextBlock &block) const
{
    QRect rect = blockBoundingGeometry(block).toRect();
    rect.setLeft(lineNumberAreaWidth());
    rect.setWidth(foldingMarginWidth);
    return rect;
}

QString CodeEditor::createPlaceholderText(const QTextBlock &startBlock,
                                        const QTextBlock &endBlock) const
{
    return QString("..."); // Simple placeholder, could be more sophisticated
}

void CodeEditor::updateViewportMargins()
{
    int leftMargin = lineNumberArea->isVisible() ? lineNumberAreaWidth() + foldingMarginWidth : foldingMarginWidth;
    setViewportMargins(leftMargin, 0, 0, 0);
}

void CodeEditor::ensureBlockIsVisible(const QTextBlock &block)
{
    QTextBlock b = block;
    while (b.isValid()) {
        if (!b.isVisible()) {
            int blockNumber = b.blockNumber();
            for (const FoldedRegion &region : foldedRegions) {
                if (region.isCollapsed && blockNumber >= region.startBlock &&
                    blockNumber <= region.endBlock) {
                    toggleFoldAt(document()->findBlockByNumber(region.startBlock));
                    break;
                }
            }
        }
        b = b.previous();
    }
}

QTextBlock CodeEditor::findFoldingEndBlock(const QTextBlock &startBlock) const
{
    int startIndent = getFoldingIndentLevel(startBlock);
    QTextBlock block = startBlock.next();
    
    while (block.isValid()) {
        if (getFoldingIndentLevel(block) <= startIndent)
            return block.previous();
        block = block.next();
    }
    
    return document()->lastBlock();
}

void CodeEditor::undo()
{
    m_isUndoRedoOperation = true;
    m_undoStack->undo();
    m_isUndoRedoOperation = false;
}

void CodeEditor::redo()
{
    m_isUndoRedoOperation = true;
    m_undoStack->redo();
    m_isUndoRedoOperation = false;
}

void CodeEditor::highlightFoldingRegions()
{
    update();
}

void CodeEditor::setToolbar(EditorToolBar* toolbar)
{
    m_toolbar = toolbar;
}

void CodeEditor::setContextMenu(EditorContextMenu* contextMenu)
{
    m_contextMenu = contextMenu;
}

bool CodeEditor::findText(const QString &searchText, bool caseSensitive, bool wholeWords,
                         bool searchBackwards, bool wrapAround)
{
    if (searchBackwards) {
        return findPrevious(searchText, caseSensitive, wholeWords, wrapAround);
    } else {
        return findNext(searchText, caseSensitive, wholeWords, wrapAround);
    }
}

void CodeEditor::detectFoldingRegions()
{
    updateFoldingRegions();
}

void CodeEditor::insertTextAtAllCursors(const QString &text)
{
    if (cursors.isEmpty()) {
        QPlainTextEdit::insertPlainText(text);
        return;
    }

    for (Cursor &cursor : cursors) {
        cursor.cursor.insertText(text);
    }
    update();
}

void CodeEditor::removeTextAtCursors(bool isBackspace)
{
    for (Cursor &cursor : cursors) {
        if (isBackspace) {
            if (!cursor.cursor.atStart()) {
                cursor.cursor.deletePreviousChar();
            }
        } else {
            if (!cursor.cursor.atEnd()) {
                cursor.cursor.deleteChar();
            }
        }
    }
    update();
}

void CodeEditor::setLineNumbersVisible(bool visible)
{
    if (visible) {
        lineNumberArea->show();
    } else {
        lineNumberArea->hide();
    }
    updateViewportMargins();
}

// ... existing code ... 