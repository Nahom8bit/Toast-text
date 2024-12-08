#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QString>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

class CrashHandler
{
public:
    static void initialize();
    static void handleCrash(int signal);
    static void logMessage(const QString& message, const QString& function = QString(), 
                          const QString& file = QString(), int line = -1);
    static QString getLogFilePath();
    static void enableDebugMode(bool enable);

private:
    static QString s_logFilePath;
    static bool s_debugMode;
    static void ensureLogDirectoryExists();
    static void writeToLog(const QString& message);
    static QString getStackTrace();
    static QString getSystemInfo();
};

// Macro for easy logging
#define LOG_DEBUG(msg) CrashHandler::logMessage(msg, __FUNCTION__, __FILE__, __LINE__)
#define LOG_ERROR(msg) CrashHandler::logMessage(QString("ERROR: ") + msg, __FUNCTION__, __FILE__, __LINE__)
#define LOG_WARNING(msg) CrashHandler::logMessage(QString("WARNING: ") + msg, __FUNCTION__, __FILE__, __LINE__)

#endif // CRASHHANDLER_H 