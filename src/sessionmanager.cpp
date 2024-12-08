#include "sessionmanager.h"
#include "editor.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTextStream>
#include <QApplication>

SessionManager::SessionManager(CodeEditor *editor, QObject *parent)
    : QObject(parent)
    , editor(editor)
    , autoSaveTimer(new QTimer(this))
    , documentModified(false)
{
    autoSaveTimer->setInterval(AUTO_SAVE_INTERVAL);
    connect(autoSaveTimer, &QTimer::timeout, this, &SessionManager::autoSave);
    connect(editor, &CodeEditor::textChanged, this, &SessionManager::handleTextChanged);
    
    loadSessionData();
}

SessionManager::~SessionManager()
{
    if (documentModified) {
        saveSession();
    }
}

void SessionManager::startAutoSave()
{
    autoSaveTimer->start();
}

void SessionManager::stopAutoSave()
{
    autoSaveTimer->stop();
}

void SessionManager::setCurrentFile(const QString &filePath)
{
    currentFilePath = filePath;
    currentSession.filePath = filePath;
    saveSessionData();
}

QString SessionManager::getAutoSaveFilePath() const
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(basePath);
    return basePath + "/autosave.txt";
}

QString SessionManager::getSessionFilePath() const
{
    QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(basePath);
    return basePath + "/session.json";
}

void SessionManager::autoSave()
{
    if (!documentModified) {
        return;
    }

    QFile file(getAutoSaveFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();
        
        currentSession.content = editor->toPlainText();
        currentSession.cursorPosition = editor->textCursor().position();
        currentSession.lastSaved = QDateTime::currentDateTime();
        currentSession.isAutoSaved = true;
        
        saveSessionData();
        documentModified = false;
    }
}

void SessionManager::handleTextChanged()
{
    documentModified = true;
}

bool SessionManager::hasAutoSavedSession() const
{
    return currentSession.isAutoSaved && QFile::exists(getAutoSaveFilePath());
}

SessionData SessionManager::getLastSession() const
{
    return currentSession;
}

void SessionManager::clearAutoSavedSession()
{
    QFile::remove(getAutoSaveFilePath());
    currentSession.isAutoSaved = false;
    saveSessionData();
}

void SessionManager::saveSession()
{
    autoSave();
    saveSessionData();
}

void SessionManager::loadSessionData()
{
    QFile file(getSessionFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject obj = doc.object();
        
        currentSession.filePath = obj["filePath"].toString();
        currentSession.cursorPosition = obj["cursorPosition"].toInt();
        currentSession.lastSaved = QDateTime::fromString(obj["lastSaved"].toString(), Qt::ISODate);
        currentSession.isAutoSaved = obj["isAutoSaved"].toBool();
        
        file.close();
        
        // Load auto-saved content if it exists
        if (currentSession.isAutoSaved) {
            QFile autoSaveFile(getAutoSaveFilePath());
            if (autoSaveFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&autoSaveFile);
                currentSession.content = in.readAll();
                autoSaveFile.close();
            }
        }
    }
}

void SessionManager::saveSessionData()
{
    QJsonObject obj;
    obj["filePath"] = currentSession.filePath;
    obj["cursorPosition"] = currentSession.cursorPosition;
    obj["lastSaved"] = currentSession.lastSaved.toString(Qt::ISODate);
    obj["isAutoSaved"] = currentSession.isAutoSaved;
    
    QJsonDocument doc(obj);
    QFile file(getSessionFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
} 