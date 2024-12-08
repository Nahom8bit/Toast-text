#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include <QMenu>
#include <QMap>

class QAction;
class CodeEditor;
class MainWindow;

class EditorContextMenu : public QMenu
{
    Q_OBJECT

public:
    explicit EditorContextMenu(CodeEditor* editor, MainWindow* mainWindow);
    ~EditorContextMenu();

    enum MenuSection {
        EditSection,
        CursorSection,
        SelectionSection,
        ClipboardSection,
        SplitSection,
        LanguageSection
    };

    // Menu management
    void addActionToSection(QAction* action, MenuSection section);
    void removeActionFromSection(QAction* action);
    void setSectionVisible(MenuSection section, bool visible);
    void updateActions();

public slots:
    void handleEditorStateChanged();
    void showContextMenu(const QPoint& pos);

private:
    void createEditActions();
    void createCursorActions();
    void createSelectionActions();
    void createClipboardActions();
    void createSplitActions();
    void createLanguageActions();

    void setupEditSection();
    void setupCursorSection();
    void setupSelectionSection();
    void setupClipboardSection();
    void setupSplitSection();
    void setupLanguageSection();

    void updateMenuSection(MenuSection section);
    void addSeparatorToSection(MenuSection section);
    void createMenuSection(MenuSection section);
    QMenu* getOrCreateSubMenu(MenuSection section);

    CodeEditor* editor;
    MainWindow* mainWindow;
    QMap<MenuSection, QList<QAction*>> menuSections;
    QMap<MenuSection, QMenu*> subMenus;

    // Action state management
    void updateEditActions();
    void updateCursorActions();
    void updateSelectionActions();
    void updateClipboardActions();
    void updateSplitActions();
    void updateLanguageActions();

    // Helper methods
    void setupActionConnections();
};

#endif // CONTEXTMENU_H 