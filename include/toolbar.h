#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QMap>

class QAction;
class CodeEditor;
class MainWindow;

class EditorToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit EditorToolBar(MainWindow *parent = nullptr);
    ~EditorToolBar();

    // Action groups
    enum ActionGroup {
        FileGroup,
        EditGroup,
        ViewGroup,
        SearchGroup,
        SplitGroup,
        ToolsGroup
    };

    // Action management
    void addActionToGroup(QAction* action, ActionGroup group);
    void removeActionFromGroup(QAction* action);
    void setGroupVisible(ActionGroup group, bool visible);
    void updateActions();

    // Editor integration
    void setCurrentEditor(CodeEditor* editor);

public slots:
    void handleEditorChanged(CodeEditor* editor);
    void updateActionStates();

private:
    MainWindow* mainWindow;
    CodeEditor* currentEditor;
    QMap<ActionGroup, QList<QAction*>> actionGroups;

    // Action creation
    void createFileActions();
    void createEditActions();
    void createViewActions();
    void createSearchActions();
    void createSplitActions();
    void createToolsActions();

    // Action setup
    void setupFileActions();
    void setupEditActions();
    void setupViewActions();
    void setupSearchActions();
    void setupSplitActions();
    void setupToolsActions();

    // Action state management
    void updateFileActions();
    void updateEditActions();
    void updateViewActions();
    void updateSearchActions();
    void updateSplitActions();
    void updateToolsActions();

    // Helper methods
    void addSeparatorToGroup(ActionGroup group);
    void createActionGroup(ActionGroup group);
    void setupActionConnections();
    void updateActionGroup(ActionGroup group);
};

#endif // TOOLBAR_H 