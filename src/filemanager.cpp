#include "filemanager.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QStandardPaths>

FileManager::FileManager(QObject *parent)
    : QObject(parent)
{
    loadRecentFiles();
}

bool FileManager::openFile(const QString &path, QString &content)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit error(tr("Cannot open file %1: %2")
                  .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }

    QTextStream in(&file);
    content = in.readAll();
    
    if (file.error() != QFile::NoError) {
        emit error(tr("Error reading from file %1: %2")
                  .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }

    addRecentFile(path);
    emit fileOpened(path);
    return true;
}

bool FileManager::saveFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit error(tr("Cannot write file %1: %2")
                  .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out << content;
    
    if (file.error() != QFile::NoError) {
        emit error(tr("Error writing to file %1: %2")
                  .arg(QDir::toNativeSeparators(path), file.errorString()));
        return false;
    }

    addRecentFile(path);
    emit fileSaved(path);
    return true;
}

QString FileManager::showOpenDialog(QWidget *parent)
{
    return QFileDialog::getOpenFileName(parent,
                                      QObject::tr("Open File"),
                                      QDir::currentPath(),
                                      QObject::tr("Text Files (*.txt);;All Files (*)"));
}

QString FileManager::showSaveDialog(QWidget *parent)
{
    return QFileDialog::getSaveFileName(parent,
                                      QObject::tr("Save File"),
                                      QDir::currentPath(),
                                      QObject::tr("Text Files (*.txt);;All Files (*)"));
}

QStringList FileManager::getRecentFiles() const
{
    return recentFiles;
}

void FileManager::addRecentFile(const QString &path)
{
    QString canonicalPath = QFileInfo(path).canonicalFilePath();
    
    recentFiles.removeAll(canonicalPath);
    recentFiles.prepend(canonicalPath);
    
    while (recentFiles.size() > MaxRecentFiles)
        recentFiles.removeLast();
        
    saveRecentFiles();
}

void FileManager::clearRecentFiles()
{
    recentFiles.clear();
    saveRecentFiles();
}

void FileManager::loadRecentFiles()
{
    QSettings settings;
    recentFiles = settings.value("recentFiles").toStringList();
}

void FileManager::saveRecentFiles()
{
    QSettings settings;
    settings.setValue("recentFiles", recentFiles);
} 