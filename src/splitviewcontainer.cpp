#include "splitviewcontainer.h"
#include "editor.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QtWidgets/QScrollBar>

SplitViewContainer::SplitViewContainer(QWidget *parent)
    : QWidget(parent), mainSplitter(new QSplitter(this)), currentEditor(nullptr),
      syncScrolling(true), syncEditing(true), fileSync(true)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mainSplitter);
}

SplitViewContainer::~SplitViewContainer()
{
}

CodeEditor* SplitViewContainer::splitView(CodeEditor* editor, SplitOrientation orientation)
{
    if (!editor || !editors.contains(editor)) {
        return nullptr;
    }

    // Create new editor
    CodeEditor* newEditor = new CodeEditor(this);
    editors.append(newEditor);

    // Copy content and settings
    newEditor->setPlainText(editor->toPlainText());
    if (fileSync) {
        newEditor->setFilePath(editor->getFilePath());
    }

    // Find the splitter that contains the source editor
    QSplitter* parentSplitter = findParentSplitter(editor);
    if (!parentSplitter) {
        parentSplitter = mainSplitter;
    }

    // Set orientation
    parentSplitter->setOrientation(orientation == Horizontal ? Qt::Horizontal : Qt::Vertical);

    // Add the new editor to the splitter
    int index = parentSplitter->indexOf(editor);
    if (index != -1) {
        parentSplitter->insertWidget(index + 1, newEditor);
    } else {
        parentSplitter->addWidget(newEditor);
    }

    // Setup connections
    setupConnections(newEditor);

    // Update splitter sizes
    updateSplitterSizes();

    emit splitAdded(newEditor);
    return newEditor;
}

void SplitViewContainer::removeSplit(CodeEditor* editor)
{
    if (!editor || !editors.contains(editor) || editors.size() <= 1) {
        return;
    }

    editors.removeOne(editor);
    if (currentEditor == editor) {
        currentEditor = editors.first();
    }

    editor->setParent(nullptr);
    editor->deleteLater();

    cleanupSplitters();
    updateSplitterSizes();

    emit splitRemoved(editor);
}

void SplitViewContainer::closeAllSplits()
{
    while (editors.size() > 1) {
        removeSplit(editors.last());
    }
}

void SplitViewContainer::setSyncScrolling(bool enabled)
{
    syncScrolling = enabled;
}

void SplitViewContainer::setSyncEditing(bool enabled)
{
    syncEditing = enabled;
}

void SplitViewContainer::setFileSync(bool enabled)
{
    fileSync = enabled;
}

QVector<CodeEditor*> SplitViewContainer::getEditors() const
{
    return editors;
}

void SplitViewContainer::handleEditorScrolled(int value)
{
    if (!syncScrolling) return;

    CodeEditor* source = qobject_cast<CodeEditor*>(sender());
    if (!source) return;

    synchronizeScrolling(source, value);
}

void SplitViewContainer::handleEditorTextChanged()
{
    if (!syncEditing) return;

    CodeEditor* source = qobject_cast<CodeEditor*>(sender());
    if (!source) return;

    synchronizeEditing(source, source->toPlainText());
}

void SplitViewContainer::handleEditorFileChanged(const QString& path)
{
    if (!fileSync) return;

    CodeEditor* source = qobject_cast<CodeEditor*>(sender());
    if (!source) return;

    synchronizeFile(source, path);
}

void SplitViewContainer::updateCurrentEditor(CodeEditor* editor)
{
    if (currentEditor != editor && editors.contains(editor)) {
        currentEditor = editor;
        emit currentEditorChanged(editor);
    }
}

void SplitViewContainer::setupConnections(CodeEditor* editor)
{
    connect(editor->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &SplitViewContainer::handleEditorScrolled);
    connect(editor, &CodeEditor::textChanged,
            this, &SplitViewContainer::handleEditorTextChanged);
    connect(editor, &CodeEditor::filePathChanged,
            this, &SplitViewContainer::handleEditorFileChanged);
}

void SplitViewContainer::removeConnections(CodeEditor* editor)
{
    disconnect(editor->verticalScrollBar(), &QScrollBar::valueChanged,
              this, &SplitViewContainer::handleEditorScrolled);
    disconnect(editor, &CodeEditor::textChanged,
              this, &SplitViewContainer::handleEditorTextChanged);
    disconnect(editor, &CodeEditor::filePathChanged,
              this, &SplitViewContainer::handleEditorFileChanged);
}

void SplitViewContainer::synchronizeScrolling(CodeEditor* source, int value)
{
    for (CodeEditor* editor : editors) {
        if (editor != source) {
            editor->verticalScrollBar()->setValue(value);
        }
    }
}

void SplitViewContainer::synchronizeEditing(CodeEditor* source, const QString& text)
{
    for (CodeEditor* editor : editors) {
        if (editor != source) {
            int scrollPos = editor->verticalScrollBar()->value();
            editor->setPlainText(text);
            editor->verticalScrollBar()->setValue(scrollPos);
        }
    }
}

void SplitViewContainer::synchronizeFile(CodeEditor* source, const QString& path)
{
    for (CodeEditor* editor : editors) {
        if (editor != source) {
            editor->setFilePath(path);
        }
    }
}

QSplitter* SplitViewContainer::findParentSplitter(CodeEditor* editor) const
{
    QWidget* parent = editor->parentWidget();
    while (parent) {
        QSplitter* splitter = qobject_cast<QSplitter*>(parent);
        if (splitter) {
            return splitter;
        }
        parent = parent->parentWidget();
    }
    return nullptr;
}

void SplitViewContainer::cleanupSplitters()
{
    QList<QSplitter*> splitters;
    QList<QWidget*> widgets;
    
    // Find all splitters and widgets
    QList<QWidget*> stack;
    stack.append(mainSplitter);
    
    while (!stack.isEmpty()) {
        QWidget* widget = stack.takeFirst();
        QSplitter* splitter = qobject_cast<QSplitter*>(widget);
        
        if (splitter) {
            splitters.append(splitter);
            for (int i = 0; i < splitter->count(); ++i) {
                stack.append(splitter->widget(i));
            }
        } else {
            widgets.append(widget);
        }
    }
    
    // Remove empty splitters
    for (QSplitter* splitter : splitters) {
        if (splitter != mainSplitter && splitter->count() == 0) {
            splitter->setParent(nullptr);
            splitter->deleteLater();
        }
    }
}

void SplitViewContainer::updateSplitterSizes()
{
    QList<QSplitter*> splitters;
    splitters.append(mainSplitter);
    
    for (QSplitter* splitter : splitters) {
        QList<int> sizes;
        int widgetCount = splitter->count();
        if (widgetCount > 0) {
            int size = splitter->orientation() == Qt::Horizontal ?
                      splitter->width() : splitter->height();
            int sizePerWidget = size / widgetCount;
            for (int i = 0; i < widgetCount; ++i) {
                sizes.append(sizePerWidget);
            }
            splitter->setSizes(sizes);
        }
    }
} 