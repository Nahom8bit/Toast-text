#include "contextmenu.h"
#include "editor.h"
#include <QAction>
#include <QApplication>
#include <QClipboard>

EditorContextMenu::EditorContextMenu(CodeEditor* editor, MainWindow* mainWindow)
    : QMenu(editor), editor(editor), mainWindow(mainWindow)
{
    // Create menu sections
    createMenuSection(EditSection);
    createMenuSection(CursorSection);
    createMenuSection(SelectionSection);
    createMenuSection(ClipboardSection);
    createMenuSection(SplitSection);
    createMenuSection(LanguageSection);

    // Create and setup actions
    createEditActions();
    createCursorActions();
    createSelectionActions();
    createClipboardActions();
    createSplitActions();
    createLanguageActions();

    setupActionConnections();
    updateActions();

    // Connect to editor signals
    connect(editor, &CodeEditor::selectionChanged, this, &EditorContextMenu::handleEditorStateChanged);
    connect(editor, &CodeEditor::cursorPositionChanged, this, &EditorContextMenu::handleEditorStateChanged);
}

EditorContextMenu::~EditorContextMenu()
{
}

void EditorContextMenu::createEditActions()
{
    QAction* undoAction = new QAction(tr("Undo"), this);
    QAction* redoAction = new QAction(tr("Redo"), this);

    addActionToSection(undoAction, EditSection);
    addActionToSection(redoAction, EditSection);
    addSeparatorToSection(EditSection);
}

void EditorContextMenu::createCursorActions()
{
    QMenu* cursorMenu = getOrCreateSubMenu(CursorSection);
    cursorMenu->setTitle(tr("Cursor"));

    QAction* addCursorAboveAction = new QAction(tr("Add Cursor Above"), this);
    QAction* addCursorBelowAction = new QAction(tr("Add Cursor Below"), this);
    QAction* addCursorsToLineEndsAction = new QAction(tr("Add Cursors to Line Ends"), this);

    addActionToSection(addCursorAboveAction, CursorSection);
    addActionToSection(addCursorBelowAction, CursorSection);
    addActionToSection(addCursorsToLineEndsAction, CursorSection);
}

void EditorContextMenu::createSelectionActions()
{
    QMenu* selectionMenu = getOrCreateSubMenu(SelectionSection);
    selectionMenu->setTitle(tr("Selection"));

    QAction* selectAllAction = new QAction(tr("Select All"), this);
    QAction* selectLineAction = new QAction(tr("Select Line"), this);
    QAction* selectWordAction = new QAction(tr("Select Word"), this);
    QAction* expandSelectionAction = new QAction(tr("Expand Selection"), this);
    QAction* shrinkSelectionAction = new QAction(tr("Shrink Selection"), this);

    addActionToSection(selectAllAction, SelectionSection);
    addActionToSection(selectLineAction, SelectionSection);
    addActionToSection(selectWordAction, SelectionSection);
    addSeparatorToSection(SelectionSection);
    addActionToSection(expandSelectionAction, SelectionSection);
    addActionToSection(shrinkSelectionAction, SelectionSection);
}

void EditorContextMenu::createClipboardActions()
{
    QAction* cutAction = new QAction(tr("Cut"), this);
    QAction* copyAction = new QAction(tr("Copy"), this);
    QAction* pasteAction = new QAction(tr("Paste"), this);

    addActionToSection(cutAction, ClipboardSection);
    addActionToSection(copyAction, ClipboardSection);
    addActionToSection(pasteAction, ClipboardSection);
}

void EditorContextMenu::createSplitActions()
{
    QMenu* splitMenu = getOrCreateSubMenu(SplitSection);
    splitMenu->setTitle(tr("Split"));

    QAction* splitHorizontalAction = new QAction(tr("Split Horizontal"), this);
    QAction* splitVerticalAction = new QAction(tr("Split Vertical"), this);
    QAction* removeSplitAction = new QAction(tr("Remove Split"), this);

    addActionToSection(splitHorizontalAction, SplitSection);
    addActionToSection(splitVerticalAction, SplitSection);
    addActionToSection(removeSplitAction, SplitSection);
}

void EditorContextMenu::createLanguageActions()
{
    QMenu* languageMenu = getOrCreateSubMenu(LanguageSection);
    languageMenu->setTitle(tr("Language"));

    // Add supported languages
    QAction* plainTextAction = new QAction(tr("Plain Text"), this);
    QAction* cppAction = new QAction(tr("C++"), this);
    QAction* pythonAction = new QAction(tr("Python"), this);
    QAction* javaScriptAction = new QAction(tr("JavaScript"), this);

    addActionToSection(plainTextAction, LanguageSection);
    addActionToSection(cppAction, LanguageSection);
    addActionToSection(pythonAction, LanguageSection);
    addActionToSection(javaScriptAction, LanguageSection);
}

void EditorContextMenu::setupActionConnections()
{
    // Edit section
    connect(menuSections[EditSection][0], &QAction::triggered, editor, &CodeEditor::undo);
    connect(menuSections[EditSection][1], &QAction::triggered, editor, &CodeEditor::redo);
    connect(menuSections[EditSection][2], &QAction::triggered, editor, &CodeEditor::cut);
    connect(menuSections[EditSection][3], &QAction::triggered, editor, &CodeEditor::copy);
    connect(menuSections[EditSection][4], &QAction::triggered, editor, &CodeEditor::paste);
    connect(menuSections[EditSection][5], &QAction::triggered, editor, &CodeEditor::selectAll);

    // Cursor section
    connect(menuSections[CursorSection][0], &QAction::triggered, [this]() {
        QPoint pos = editor->mapFromGlobal(QCursor::pos());
        editor->addCursorAtMousePosition(pos);
    });
    connect(menuSections[CursorSection][1], &QAction::triggered, editor, &CodeEditor::clearAdditionalCursors);
    connect(menuSections[CursorSection][2], &QAction::triggered, editor, &CodeEditor::addCursorAbove);
    connect(menuSections[CursorSection][3], &QAction::triggered, editor, &CodeEditor::addCursorBelow);

    // Selection section
    connect(menuSections[SelectionSection][0], &QAction::triggered, editor, &CodeEditor::expandSelection);
    connect(menuSections[SelectionSection][1], &QAction::triggered, editor, &CodeEditor::shrinkSelection);
    connect(menuSections[SelectionSection][2], &QAction::triggered, editor, &CodeEditor::selectWord);
    connect(menuSections[SelectionSection][3], &QAction::triggered, editor, &CodeEditor::selectLine);

    // Clipboard section
    connect(menuSections[ClipboardSection][0], &QAction::triggered, [this]() {
        editor->selectLine();
        editor->cut();
    });
    connect(menuSections[ClipboardSection][1], &QAction::triggered, [this]() {
        editor->selectLine();
        editor->copy();
    });
    connect(menuSections[ClipboardSection][2], &QAction::triggered, [this]() {
        editor->selectLine();
        editor->copy();
        editor->textCursor().movePosition(QTextCursor::EndOfLine);
        editor->insertPlainText("\n");
        editor->paste();
    });
    connect(menuSections[ClipboardSection][3], &QAction::triggered, [this]() {
        editor->selectLine();
        editor->textCursor().removeSelectedText();
    });

    // Split section
    connect(menuSections[SplitSection][0], &QAction::triggered, editor, &CodeEditor::splitHorizontally);
    connect(menuSections[SplitSection][1], &QAction::triggered, editor, &CodeEditor::splitVertically);
    connect(menuSections[SplitSection][2], &QAction::triggered, editor, &CodeEditor::closeSplit);

    // Language section
    for (QAction* action : menuSections[LanguageSection]) {
        connect(action, &QAction::triggered, [this, action]() {
            editor->setLanguage(action->text());
        });
    }
}

void EditorContextMenu::addActionToSection(QAction* action, MenuSection section)
{
    menuSections[section].append(action);
    QMenu* subMenu = subMenus.value(section, nullptr);
    if (subMenu) {
        subMenu->addAction(action);
    } else {
        addAction(action);
    }
}

void EditorContextMenu::removeActionFromSection(QAction* action)
{
    for (auto it = menuSections.begin(); it != menuSections.end(); ++it) {
        it.value().removeOne(action);
    }
    removeAction(action);
}

void EditorContextMenu::setSectionVisible(MenuSection section, bool visible)
{
    QMenu* subMenu = subMenus.value(section, nullptr);
    if (subMenu) {
        subMenu->menuAction()->setVisible(visible);
    } else {
        for (QAction* action : menuSections[section]) {
            action->setVisible(visible);
        }
    }
}

void EditorContextMenu::updateActions()
{
    // Update all menu sections at once
    for (auto section : {EditSection, CursorSection, SelectionSection, 
                        ClipboardSection, SplitSection, LanguageSection}) {
        updateMenuSection(section);
    }
}

void EditorContextMenu::handleEditorStateChanged()
{
    updateActions();
}

void EditorContextMenu::showContextMenu(const QPoint& pos)
{
    updateActions();
    exec(editor->mapToGlobal(pos));
}

void EditorContextMenu::addSeparatorToSection(MenuSection section)
{
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    addActionToSection(separator, section);
}

void EditorContextMenu::createMenuSection(MenuSection section)
{
    if (!menuSections.contains(section)) {
        menuSections[section] = QList<QAction*>();
    }
}

QMenu* EditorContextMenu::getOrCreateSubMenu(MenuSection section)
{
    if (!subMenus.contains(section)) {
        QMenu* subMenu = new QMenu(this);
        subMenus[section] = subMenu;
        addMenu(subMenu);
    }
    return subMenus[section];
}

void EditorContextMenu::updateMenuSection(MenuSection section)
{
    bool hasSelection = editor->textCursor().hasSelection();
    
    switch (section) {
        case EditSection:
            menuSections[EditSection][0]->setEnabled(editor->document()->isUndoAvailable());
            menuSections[EditSection][1]->setEnabled(editor->document()->isRedoAvailable());
            break;
        case ClipboardSection:
            menuSections[ClipboardSection][0]->setEnabled(hasSelection);
            menuSections[ClipboardSection][1]->setEnabled(hasSelection);
            menuSections[ClipboardSection][2]->setEnabled(!QApplication::clipboard()->text().isEmpty());
            break;
        case SelectionSection:
            for (QAction* action : menuSections[SelectionSection]) {
                action->setEnabled(true);
            }
            break;
        case CursorSection:
            for (QAction* action : menuSections[CursorSection]) {
                action->setEnabled(true);
            }
            break;
        case SplitSection:
            for (QAction* action : menuSections[SplitSection]) {
                action->setEnabled(true);
            }
            break;
        case LanguageSection:
            for (QAction* action : menuSections[LanguageSection]) {
                action->setEnabled(true);
            }
            break;
    }
} 