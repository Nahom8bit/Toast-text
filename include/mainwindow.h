#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QMap>
#include "editor.h"
#include "sessionmanager.h"
#include "dialogs/finddialog.h"
#include "dialogs/autocorrectdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void about();
    void showFindDialog();
    void findNext();
    void findPrevious();
    void showAutoCorrectDialog();
    void toggleAutoCorrect();
    void handleTextChange();
    void updateWordCount();
    void documentWasModified();
    void checkForRecovery();
    void recoverSession();
    void discardSession();

private:
    void createActions();
    void createMenus();
    void setupEditor();
    void setupStatusBar();
    void initializeAutocorrect();
    void setCurrentFile(const QString &fileName);
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    bool find(const QString &searchString, bool forward = true);
    QString applyAutocorrect(const QString &text);

    Ui::MainWindow *ui;
    CodeEditor *textEdit;
    QLabel *wordCountLabel;
    QLabel *autocorrectLabel;
    FindDialog *findDialog;
    AutoCorrectDialog *autocorrectDialog;
    SessionManager *sessionManager;
    bool autoCorrectEnabled;
    QString currentFile;
    QMap<QString, QString> autocorrectRules;

    // Menu actions
    QAction *actionZoomIn;
    QAction *actionZoomOut;
    QAction *actionZoomReset;
    QAction *actionToggleLineNumbers;
    QAction *actionSettings;
    QAction *actionToggleFolding;
    QAction *actionFoldAll;
    QAction *actionUnfoldAll;
    QAction *actionAboutQt;
};

#endif // MAINWINDOW_H 