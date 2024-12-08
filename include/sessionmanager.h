#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QDateTime>

class CodeEditor;

struct SessionData {
    QString filePath;
    QString content;
    int cursorPosition;
    QDateTime lastSaved;
    bool isAutoSaved;
};

class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(CodeEditor *editor, QObject *parent = nullptr);
    ~SessionManager();

    void startAutoSave();
    void stopAutoSave();
    void setCurrentFile(const QString &filePath);
    bool hasAutoSavedSession() const;
    SessionData getLastSession() const;
    void clearAutoSavedSession();
    void saveSession();

public slots:
    void autoSave();
    void handleTextChanged();

private:
    QString getAutoSaveFilePath() const;
    QString getSessionFilePath() const;
    void loadSessionData();
    void saveSessionData();

    CodeEditor *editor;
    QTimer *autoSaveTimer;
    QString currentFilePath;
    bool documentModified;
    SessionData currentSession;
    static const int AUTO_SAVE_INTERVAL = 60000; // 1 minute
};

#endif // SESSIONMANAGER_H 