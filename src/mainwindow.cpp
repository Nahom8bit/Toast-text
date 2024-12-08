#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogs/finddialog.h"
#include "dialogs/autocorrectdialog.h"
#include "dialogs/recoverydialog.h"
#include "sessionmanager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QCloseEvent>
#include <QStatusBar>
#include <QTextDocument>
#include <QDir>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , textEdit(new CodeEditor)
    , wordCountLabel(new QLabel(this))
    , autocorrectLabel(new QLabel(this))
    , findDialog(nullptr)
    , autocorrectDialog(nullptr)
    , sessionManager(new SessionManager(textEdit, this))
    , autoCorrectEnabled(true)
{
    ui->setupUi(this);
    setCentralWidget(textEdit);
    
    createActions();
    createMenus();
    setupEditor();
    setupStatusBar();
    initializeAutocorrect();
    
    connect(textEdit, &CodeEditor::textChanged, this, &MainWindow::handleTextChange);
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::documentWasModified);
    
    sessionManager->startAutoSave();
    QTimer::singleShot(0, this, &MainWindow::checkForRecovery);
    
    setCurrentFile(QString());
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete findDialog;
    delete autocorrectDialog;
}

void MainWindow::checkForRecovery()
{
    if (sessionManager->hasAutoSavedSession()) {
        SessionData lastSession = sessionManager->getLastSession();
        RecoveryDialog *dialog = new RecoveryDialog(lastSession.filePath, lastSession.lastSaved, this);
        
        connect(dialog, &RecoveryDialog::recoverSession, this, &MainWindow::recoverSession);
        connect(dialog, &RecoveryDialog::discardSession, this, &MainWindow::discardSession);
        
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    }
}

void MainWindow::recoverSession()
{
    SessionData lastSession = sessionManager->getLastSession();
    textEdit->setPlainText(lastSession.content);
    
    QTextCursor cursor = textEdit->textCursor();
    cursor.setPosition(lastSession.cursorPosition);
    textEdit->setTextCursor(cursor);
    
    if (!lastSession.filePath.isEmpty()) {
        setCurrentFile(lastSession.filePath);
    }
    
    // Clear the auto-saved session since we've recovered it
    sessionManager->clearAutoSavedSession();
    
    // Close the dialog
    if (QDialog *dialog = qobject_cast<QDialog*>(sender()->parent())) {
        dialog->accept();
    }
}

void MainWindow::discardSession()
{
    sessionManager->clearAutoSavedSession();
    
    // Close the dialog
    if (QDialog *dialog = qobject_cast<QDialog*>(sender()->parent())) {
        dialog->accept();
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    sessionManager->setCurrentFile(fileName);
    textEdit->document()->setModified(false);
    
    QString shownName = currentFile;
    if (currentFile.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                           tr("Cannot read file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName),
                               file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    // Enable syntax highlighting based on file extension
    QFileInfo fileInfo(fileName);
    textEdit->setLanguage(fileInfo.suffix().toLower());

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
    updateWordCount();
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Text Editor"),
                           tr("Cannot write file %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
        return false;
    }
    
    QTextStream out(&file);
    out << textEdit->toPlainText();
    
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setupStatusBar()
{
    wordCountLabel->setText("Words: 0");
    autocorrectLabel->setText("Auto-Correct: On");
    
    statusBar()->addPermanentWidget(wordCountLabel);
    statusBar()->addPermanentWidget(autocorrectLabel);
    
    // Make auto-correct label clickable
    autocorrectLabel->setStyleSheet("QLabel { color: blue; text-decoration: underline; }");
    autocorrectLabel->setCursor(Qt::PointingHandCursor);
    connect(autocorrectLabel, &QLabel::linkActivated, this, &MainWindow::toggleAutoCorrect);
}

void MainWindow::createActions()
{
    // File menu actions
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    ui->actionNew->setShortcut(QKeySequence::New);
    
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    ui->actionOpen->setShortcut(QKeySequence::Open);
    
    connect(ui->actionSave, &QAction::triggered, this, [this]() { saveFile(); });
    ui->actionSave->setShortcut(QKeySequence::Save);
    
    connect(ui->actionSaveAs, &QAction::triggered, this, [this]() { saveFileAs(); });
    ui->actionSaveAs->setShortcut(QKeySequence::SaveAs);
    
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    ui->actionExit->setShortcut(QKeySequence::Quit);
    
    // Edit menu actions
    connect(ui->actionUndo, &QAction::triggered, textEdit, &CodeEditor::undo);
    ui->actionUndo->setShortcut(QKeySequence::Undo);
    
    connect(ui->actionRedo, &QAction::triggered, textEdit, &CodeEditor::redo);
    ui->actionRedo->setShortcut(QKeySequence::Redo);
    
    connect(ui->actionCut, &QAction::triggered, textEdit, &CodeEditor::cut);
    ui->actionCut->setShortcut(QKeySequence::Cut);
    
    connect(ui->actionCopy, &QAction::triggered, textEdit, &CodeEditor::copy);
    ui->actionCopy->setShortcut(QKeySequence::Copy);
    
    connect(ui->actionPaste, &QAction::triggered, textEdit, &CodeEditor::paste);
    ui->actionPaste->setShortcut(QKeySequence::Paste);
    
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    ui->actionFind->setShortcut(QKeySequence::Find);
    
    connect(ui->actionAutoCorrect, &QAction::triggered, this, &MainWindow::showAutoCorrectDialog);
    
    // View menu actions
    actionZoomIn = new QAction(tr("Zoom In"), this);
    actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    
    actionZoomOut = new QAction(tr("Zoom Out"), this);
    actionZoomOut->setShortcut(QKeySequence::ZoomOut);
    
    actionZoomReset = new QAction(tr("Reset Zoom"), this);
    actionZoomReset->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    
    actionToggleLineNumbers = new QAction(tr("Show Line Numbers"), this);
    actionToggleLineNumbers->setCheckable(true);
    actionToggleLineNumbers->setChecked(true);
    actionToggleLineNumbers->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    
    connect(actionZoomIn, &QAction::triggered, textEdit, [this]() {
        QFont f = textEdit->font();
        f.setPointSize(f.pointSize() + 1);
        textEdit->setFont(f);
    });
    connect(actionZoomOut, &QAction::triggered, textEdit, [this]() {
        QFont f = textEdit->font();
        f.setPointSize(qMax(f.pointSize() - 1, 6));
        textEdit->setFont(f);
    });
    connect(actionZoomReset, &QAction::triggered, textEdit, &CodeEditor::resetZoom);
    connect(actionToggleLineNumbers, &QAction::triggered, textEdit, [this](bool checked) {
        textEdit->setLineNumbersVisible(checked);
    });
    
    // Tools menu actions
    actionSettings = new QAction(tr("Settings"), this);
    actionSettings->setShortcut(QKeySequence::Preferences);
    
    actionToggleFolding = new QAction(tr("Toggle Code Folding"), this);
    actionToggleFolding->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    
    actionFoldAll = new QAction(tr("Fold All"), this);
    actionFoldAll->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft));
    
    actionUnfoldAll = new QAction(tr("Unfold All"), this);
    actionUnfoldAll->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight));
    
    connect(actionSettings, &QAction::triggered, textEdit, &CodeEditor::showSettingsDialog);
    connect(actionToggleFolding, &QAction::triggered, textEdit, &CodeEditor::toggleFold);
    connect(actionFoldAll, &QAction::triggered, textEdit, &CodeEditor::foldAll);
    connect(actionUnfoldAll, &QAction::triggered, textEdit, &CodeEditor::unfoldAll);
    
    // Help menu actions
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    ui->actionAbout->setShortcut(QKeySequence::HelpContents);
    
    actionAboutQt = new QAction(tr("About Qt"), this);
    connect(actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    
    // Update Undo/Redo state
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            ui->actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            ui->actionRedo, &QAction::setEnabled);
    
    // Initialize Undo/Redo state
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
}

void MainWindow::createMenus()
{
    // Add actions to existing menus from the UI file
    ui->menuView->addAction(actionZoomIn);
    ui->menuView->addAction(actionZoomOut);
    ui->menuView->addAction(actionZoomReset);
    ui->menuView->addSeparator();
    ui->menuView->addAction(actionToggleLineNumbers);
    
    ui->menuTools->addAction(actionSettings);
    ui->menuTools->addSeparator();
    ui->menuTools->addAction(actionToggleFolding);
    ui->menuTools->addAction(actionFoldAll);
    ui->menuTools->addAction(actionUnfoldAll);
    
    ui->menuHelp->addAction(actionAboutQt);
    
    // Update menu bar to ensure proper order
    menuBar()->clear();
    menuBar()->addMenu(ui->menuFile);
    menuBar()->addMenu(ui->menuEdit);
    menuBar()->addMenu(ui->menuView);
    menuBar()->addMenu(ui->menuTools);
    menuBar()->addMenu(ui->menuHelp);
}

void MainWindow::showAutoCorrectDialog()
{
    if (!autocorrectDialog) {
        autocorrectDialog = new AutoCorrectDialog(autocorrectRules, this);
    }
    
    autocorrectDialog->show();
    autocorrectDialog->raise();
    autocorrectDialog->activateWindow();
}

void MainWindow::toggleAutoCorrect()
{
    autoCorrectEnabled = !autoCorrectEnabled;
    autocorrectLabel->setText(autoCorrectEnabled ? "Auto-Correct: On" : "Auto-Correct: Off");
}

void MainWindow::handleTextChange()
{
    updateWordCount();
    
    if (!autoCorrectEnabled)
        return;
    
    // Get the current cursor
    QTextCursor cursor = textEdit->textCursor();
    
    // Only proceed if we're at a word boundary
    QString text = textEdit->toPlainText();
    if (text.isEmpty() || !text.at(text.length() - 1).isSpace())
        return;
    
    // Get the last word
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (words.isEmpty())
        return;
        
    QString lastWord = words.last();
    QString corrected = applyAutocorrect(lastWord);
    
    if (corrected != lastWord) {
        // Replace the last word with the corrected version
        cursor.movePosition(QTextCursor::PreviousWord);
        cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
        cursor.insertText(corrected);
    }
}

void MainWindow::initializeAutocorrect()
{
    // Add some common autocorrect rules
    autocorrectRules = {
        {"teh", "the"},
        {"dont", "don't"},
        {"cant", "can't"},
        {"wont", "won't"},
        {"im", "I'm"},
        {"ive", "I've"},
        {"id", "I'd"},
        {"theyre", "they're"},
        {"youre", "you're"},
        {"wasnt", "wasn't"},
        {"didnt", "didn't"},
        {"isnt", "isn't"},
        {"wouldnt", "wouldn't"},
        {"shouldnt", "shouldn't"},
        {"couldnt", "couldn't"}
    };
}

QString MainWindow::applyAutocorrect(const QString &text)
{
    return autocorrectRules.value(text.toLower(), text);
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), QDir::currentPath(),
            tr("Text Files (*.txt);;C++ Files (*.cpp *.h);;All Files (*)"),
            nullptr,
            QFileDialog::DontUseNativeDialog);
        if (!fileName.isEmpty()) {
            loadFile(fileName);
        }
    }
}

bool MainWindow::saveFile()
{
    if (currentFile.isEmpty()) {
        return saveFileAs();
    } else {
        return saveFile(currentFile);
    }
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save As"), currentFile.isEmpty() ? QDir::currentPath() : currentFile,
        tr("Text Files (*.txt);;C++ Files (*.cpp *.h);;All Files (*)"));
    if (fileName.isEmpty())
        return false;

    return saveFile(fileName);
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Text Editor"),
        tr("A simple text editor built with Qt 6"));
}

void MainWindow::setupEditor()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);
    
    textEdit->setFont(font);
}

bool MainWindow::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;
    
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Text Editor"),
                             tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    
    switch (ret) {
    case QMessageBox::Save:
        return saveFile();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::showFindDialog()
{
    if (!findDialog) {
        findDialog = new FindDialog(this);
        connect(findDialog, &FindDialog::findNext,
                this, &MainWindow::findNext);
        connect(findDialog, &FindDialog::findPrevious,
                this, &MainWindow::findPrevious);
    }
    
    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void MainWindow::findNext()
{
    if (findDialog) {
        find(findDialog->searchText(), true);
    }
}

void MainWindow::findPrevious()
{
    if (findDialog) {
        find(findDialog->searchText(), false);
    }
}

bool MainWindow::find(const QString &searchString, bool forward)
{
    if (searchString.isEmpty()) {
        statusBar()->showMessage(tr("The search string cannot be empty"), 2000);
        return false;
    }

    QTextDocument::FindFlags flags;
    if (findDialog && findDialog->caseSensitive())
        flags |= QTextDocument::FindCaseSensitively;
    if (findDialog && findDialog->wholeWords())
        flags |= QTextDocument::FindWholeWords;
    if (!forward)
        flags |= QTextDocument::FindBackward;

    QTextCursor cursor = textEdit->textCursor();
    QTextCursor found = textEdit->document()->find(searchString, cursor, flags);

    if (found.isNull()) {
        // If not found, try from the beginning/end
        cursor = QTextCursor(textEdit->document());
        if (!forward)
            cursor.movePosition(QTextCursor::End);
        found = textEdit->document()->find(searchString, cursor, flags);
        
        if (found.isNull()) {
            statusBar()->showMessage(tr("'%1' not found").arg(searchString), 2000);
            return false;
        }
    }

    textEdit->setTextCursor(found);
    return true;
}

void MainWindow::updateWordCount()
{
    QString text = textEdit->toPlainText();
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    wordCountLabel->setText(QString("Words: %1").arg(words.size()));
}

void MainWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
} 