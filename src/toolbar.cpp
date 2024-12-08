#include "toolbar.h"
#include "editor.h"
#include "mainwindow.h"
#include <QAction>
#include <QToolButton>
#include <QMenu>
#include <QToolBar>
#include <QApplication>
#include <QClipboard>

EditorToolBar::EditorToolBar(MainWindow *parent)
    : QToolBar(static_cast<QWidget*>(parent)), mainWindow(parent), currentEditor(nullptr)
{
    setObjectName("mainToolBar");
    setMovable(true);
    setFloatable(true);

    // Create toolbar buttons
    createFileActions();
    createEditActions();
    createViewActions();
    createSearchActions();
    createSplitActions();
    createToolsActions();
    setupActionConnections();
}

EditorToolBar::~EditorToolBar()
{
}

void EditorToolBar::createFileActions()
{
    QAction* newAction = new QAction(QIcon::fromTheme("document-new"), tr("New"), this);
    QAction* openAction = new QAction(QIcon::fromTheme("document-open"), tr("Open"), this);
    QAction* saveAction = new QAction(QIcon::fromTheme("document-save"), tr("Save"), this);
    QAction* saveAsAction = new QAction(QIcon::fromTheme("document-save-as"), tr("Save As"), this);

    addActionToGroup(newAction, FileGroup);
    addActionToGroup(openAction, FileGroup);
    addSeparatorToGroup(FileGroup);
    addActionToGroup(saveAction, FileGroup);
    addActionToGroup(saveAsAction, FileGroup);
}

void EditorToolBar::createEditActions()
{
    QAction* undoAction = new QAction(QIcon::fromTheme("edit-undo"), tr("Undo"), this);
    QAction* redoAction = new QAction(QIcon::fromTheme("edit-redo"), tr("Redo"), this);
    QAction* cutAction = new QAction(QIcon::fromTheme("edit-cut"), tr("Cut"), this);
    QAction* copyAction = new QAction(QIcon::fromTheme("edit-copy"), tr("Copy"), this);
    QAction* pasteAction = new QAction(QIcon::fromTheme("edit-paste"), tr("Paste"), this);

    addActionToGroup(undoAction, EditGroup);
    addActionToGroup(redoAction, EditGroup);
    addSeparatorToGroup(EditGroup);
    addActionToGroup(cutAction, EditGroup);
    addActionToGroup(copyAction, EditGroup);
    addActionToGroup(pasteAction, EditGroup);
}

void EditorToolBar::createViewActions()
{
    QAction* zoomInAction = new QAction(QIcon::fromTheme("zoom-in"), tr("Zoom In"), this);
    QAction* zoomOutAction = new QAction(QIcon::fromTheme("zoom-out"), tr("Zoom Out"), this);
    QAction* resetZoomAction = new QAction(QIcon::fromTheme("zoom-original"), tr("Reset Zoom"), this);

    addActionToGroup(zoomInAction, ViewGroup);
    addActionToGroup(zoomOutAction, ViewGroup);
    addActionToGroup(resetZoomAction, ViewGroup);
}

void EditorToolBar::createSearchActions()
{
    QAction* findAction = new QAction(QIcon::fromTheme("edit-find"), tr("Find"), this);
    QAction* replaceAction = new QAction(QIcon::fromTheme("edit-find-replace"), tr("Replace"), this);
    QAction* findNextAction = new QAction(QIcon::fromTheme("go-next"), tr("Find Next"), this);
    QAction* findPrevAction = new QAction(QIcon::fromTheme("go-previous"), tr("Find Previous"), this);

    addActionToGroup(findAction, SearchGroup);
    addActionToGroup(replaceAction, SearchGroup);
    addSeparatorToGroup(SearchGroup);
    addActionToGroup(findNextAction, SearchGroup);
    addActionToGroup(findPrevAction, SearchGroup);
}

void EditorToolBar::createSplitActions()
{
    QAction* splitHorizontalAction = new QAction(QIcon::fromTheme("view-split-left-right"), tr("Split Horizontal"), this);
    QAction* splitVerticalAction = new QAction(QIcon::fromTheme("view-split-top-bottom"), tr("Split Vertical"), this);
    QAction* removeSplitAction = new QAction(QIcon::fromTheme("view-close"), tr("Remove Split"), this);

    addActionToGroup(splitHorizontalAction, SplitGroup);
    addActionToGroup(splitVerticalAction, SplitGroup);
    addActionToGroup(removeSplitAction, SplitGroup);
}

void EditorToolBar::createToolsActions()
{
    QAction* settingsAction = new QAction(QIcon::fromTheme("preferences-system"), tr("Settings"), this);
    addActionToGroup(settingsAction, ToolsGroup);
}

void EditorToolBar::setupActionConnections()
{
    // File actions
    connect(actionGroups[FileGroup][0], SIGNAL(triggered()), mainWindow, SLOT(newFile()));
    connect(actionGroups[FileGroup][1], SIGNAL(triggered()), mainWindow, SLOT(openFile()));
    connect(actionGroups[FileGroup][2], SIGNAL(triggered()), mainWindow, SLOT(saveFile()));
    connect(actionGroups[FileGroup][3], SIGNAL(triggered()), mainWindow, SLOT(saveFileAs()));

    // Edit actions
    connect(actionGroups[EditGroup][0], SIGNAL(triggered()), currentEditor, SLOT(undo()));
    connect(actionGroups[EditGroup][1], SIGNAL(triggered()), currentEditor, SLOT(redo()));
    connect(actionGroups[EditGroup][2], SIGNAL(triggered()), currentEditor, SLOT(cut()));
    connect(actionGroups[EditGroup][3], SIGNAL(triggered()), currentEditor, SLOT(copy()));
    connect(actionGroups[EditGroup][4], SIGNAL(triggered()), currentEditor, SLOT(paste()));

    // View actions
    connect(actionGroups[ViewGroup][0], SIGNAL(triggered()), currentEditor, SLOT(zoomIn()));
    connect(actionGroups[ViewGroup][1], SIGNAL(triggered()), currentEditor, SLOT(zoomOut()));
    connect(actionGroups[ViewGroup][2], SIGNAL(triggered()), currentEditor, SLOT(resetZoom()));

    // Search actions
    connect(actionGroups[SearchGroup][0], SIGNAL(triggered()), mainWindow, SLOT(showFindDialog()));
    connect(actionGroups[SearchGroup][1], SIGNAL(triggered()), mainWindow, SLOT(showFindDialog()));

    // Split actions
    connect(actionGroups[SplitGroup][0], SIGNAL(triggered()), this, SLOT(splitHorizontally()));
    connect(actionGroups[SplitGroup][1], SIGNAL(triggered()), this, SLOT(splitVertically()));

    // Tools actions
    connect(actionGroups[ToolsGroup][0], SIGNAL(triggered()), mainWindow, SLOT(about()));
}

void EditorToolBar::addActionToGroup(QAction* action, ActionGroup group)
{
    actionGroups[group].append(action);
    // Add to toolbar, not menu
    QToolButton* button = qobject_cast<QToolButton*>(addWidget(new QToolButton));
    if (button) {
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
}

void EditorToolBar::removeActionFromGroup(QAction* action)
{
    for (auto it = actionGroups.begin(); it != actionGroups.end(); ++it) {
        it.value().removeOne(action);
    }
    removeAction(action);
}

void EditorToolBar::setGroupVisible(ActionGroup group, bool visible)
{
    for (QAction* action : actionGroups[group]) {
        action->setVisible(visible);
    }
}

void EditorToolBar::updateActions()
{
    updateFileActions();
    updateEditActions();
    updateViewActions();
    updateSearchActions();
    updateSplitActions();
    updateToolsActions();
}

void EditorToolBar::setCurrentEditor(CodeEditor* editor)
{
    if (currentEditor != editor) {
        currentEditor = editor;
        updateActions();
    }
}

void EditorToolBar::handleEditorChanged(CodeEditor* editor)
{
    setCurrentEditor(editor);
}

void EditorToolBar::updateActionStates()
{
    updateActions();
}

void EditorToolBar::addSeparatorToGroup(ActionGroup group)
{
    QAction* separator = addSeparator();
    actionGroups[group].append(separator);
}

void EditorToolBar::createActionGroup(ActionGroup group)
{
    if (!actionGroups.contains(group)) {
        actionGroups[group] = QList<QAction*>();
    }
}

void EditorToolBar::updateActionGroup(ActionGroup group)
{
    bool hasEditor = currentEditor != nullptr;
    
    for (QAction* action : actionGroups[group]) {
        action->setEnabled(hasEditor);
    }

    // Update specific actions based on editor state
    if (hasEditor) {
        switch (group) {
            case EditGroup:
                actionGroups[EditGroup][0]->setEnabled(currentEditor->document()->isUndoAvailable());
                actionGroups[EditGroup][1]->setEnabled(currentEditor->document()->isRedoAvailable());
                actionGroups[EditGroup][3]->setEnabled(currentEditor->textCursor().hasSelection());
                actionGroups[EditGroup][4]->setEnabled(currentEditor->textCursor().hasSelection());
                actionGroups[EditGroup][5]->setEnabled(!QApplication::clipboard()->text().isEmpty());
                break;
            case FileGroup:
                actionGroups[FileGroup][3]->setEnabled(currentEditor->document()->isModified());
                break;
            default:
                break;
        }
    }
}

void EditorToolBar::updateFileActions()
{
    updateActionGroup(FileGroup);
}

void EditorToolBar::updateEditActions()
{
    updateActionGroup(EditGroup);
}

void EditorToolBar::updateViewActions()
{
    updateActionGroup(ViewGroup);
}

void EditorToolBar::updateSearchActions()
{
    updateActionGroup(SearchGroup);
}

void EditorToolBar::updateSplitActions()
{
    updateActionGroup(SplitGroup);
}

void EditorToolBar::updateToolsActions()
{
    updateActionGroup(ToolsGroup);
} 