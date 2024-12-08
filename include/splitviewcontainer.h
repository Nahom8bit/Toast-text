#ifndef SPLITVIEWCONTAINER_H
#define SPLITVIEWCONTAINER_H

#include <QWidget>
#include <QSplitter>
#include <QVector>

class CodeEditor;
class QVBoxLayout;

class SplitViewContainer : public QWidget
{
    Q_OBJECT

public:
    explicit SplitViewContainer(QWidget *parent = nullptr);
    ~SplitViewContainer();

    enum SplitOrientation {
        Horizontal,
        Vertical
    };

    // Split operations
    CodeEditor* splitView(CodeEditor* editor, SplitOrientation orientation);
    void removeSplit(CodeEditor* editor);
    void closeAllSplits();

    // Synchronization
    void setSyncScrolling(bool enabled);
    void setSyncEditing(bool enabled);
    void setFileSync(bool enabled);

    // Getters
    QVector<CodeEditor*> getEditors() const;
    CodeEditor* getCurrentEditor() const;
    bool isSyncScrollingEnabled() const { return syncScrolling; }
    bool isSyncEditingEnabled() const { return syncEditing; }
    bool isFileSyncEnabled() const { return fileSync; }

public slots:
    void handleEditorScrolled(int value);
    void handleEditorTextChanged();
    void handleEditorFileChanged(const QString& path);
    void updateCurrentEditor(CodeEditor* editor);

signals:
    void currentEditorChanged(CodeEditor* editor);
    void splitAdded(CodeEditor* editor);
    void splitRemoved(CodeEditor* editor);

private:
    QSplitter* mainSplitter;
    QVector<CodeEditor*> editors;
    CodeEditor* currentEditor;
    bool syncScrolling;
    bool syncEditing;
    bool fileSync;

    void setupConnections(CodeEditor* editor);
    void removeConnections(CodeEditor* editor);
    void synchronizeScrolling(CodeEditor* source, int value);
    void synchronizeEditing(CodeEditor* source, const QString& text);
    void synchronizeFile(CodeEditor* source, const QString& path);
    QSplitter* findParentSplitter(CodeEditor* editor) const;
    void cleanupSplitters();
    void updateSplitterSizes();
};

#endif // SPLITVIEWCONTAINER_H 