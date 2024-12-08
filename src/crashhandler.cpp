#include "crashhandler.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QProcess>
#include <csignal>
#include <cstdlib>
#include <execinfo.h>

QString CrashHandler::s_logFilePath;
bool CrashHandler::s_debugMode = false;

void CrashHandler::initialize()
{
    // Set up log file path
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    dir.mkpath("logs");
    s_logFilePath = dir.filePath(QString("logs/crash_%1.log")
                                .arg(QDateTime::currentDateTime()
                                .toString("yyyy-MM-dd_hh-mm-ss")));

    // Install signal handlers
    signal(SIGSEGV, handleCrash);
    signal(SIGABRT, handleCrash);
    signal(SIGFPE, handleCrash);
    signal(SIGILL, handleCrash);

    LOG_DEBUG("Crash handler initialized");
    LOG_DEBUG("Log file: " + s_logFilePath);
    LOG_DEBUG("System info: " + getSystemInfo());
}

void CrashHandler::handleCrash(int signal)
{
    QString crashMessage = QString("\n=== CRASH DETECTED ===\n")
                          + "Signal: " + QString::number(signal) + "\n"
                          + "Time: " + QDateTime::currentDateTime().toString() + "\n"
                          + "Stack trace:\n" + getStackTrace() + "\n"
                          + "System info:\n" + getSystemInfo() + "\n"
                          + "==================\n";

    writeToLog(crashMessage);
    
    // Ensure the log is written before exiting
    QFile logFile(s_logFilePath);
    logFile.flush();
    
    // In debug mode, we'll abort to get a core dump
    if (s_debugMode) {
        abort();
    } else {
        exit(signal);
    }
}

void CrashHandler::logMessage(const QString& message, const QString& function,
                            const QString& file, int line)
{
    QString logMessage = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
    
    if (!function.isEmpty()) {
        logMessage += "[" + function + "] ";
    }
    if (!file.isEmpty() && line != -1) {
        logMessage += QString("(%1:%2) ").arg(file).arg(line);
    }
    
    logMessage += message + "\n";
    writeToLog(logMessage);
}

QString CrashHandler::getLogFilePath()
{
    return s_logFilePath;
}

void CrashHandler::enableDebugMode(bool enable)
{
    s_debugMode = enable;
    LOG_DEBUG(QString("Debug mode %1").arg(enable ? "enabled" : "disabled"));
}

void CrashHandler::ensureLogDirectoryExists()
{
    QFileInfo fileInfo(s_logFilePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

void CrashHandler::writeToLog(const QString& message)
{
    ensureLogDirectoryExists();
    
    QFile logFile(s_logFilePath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&logFile);
        stream << message;
        stream.flush();
        logFile.close();
    }
    
    if (s_debugMode) {
        qDebug().noquote() << message.trimmed();
    }
}

QString CrashHandler::getStackTrace()
{
    QString stackTrace;
    void* array[50];
    int size = backtrace(array, 50);
    char** messages = backtrace_symbols(array, size);

    for (int i = 0; i < size && messages != nullptr; ++i) {
        stackTrace += QString("%1: %2\n").arg(i).arg(messages[i]);
    }

    free(messages);
    return stackTrace;
}

QString CrashHandler::getSystemInfo()
{
    QString info;
    info += "Application: " + QCoreApplication::applicationName() + "\n";
    info += "Version: " + QCoreApplication::applicationVersion() + "\n";
    info += "Qt Version: " + QString(qVersion()) + "\n";
    info += "OS: " + QSysInfo::prettyProductName() + "\n";
    info += "CPU Architecture: " + QSysInfo::currentCpuArchitecture() + "\n";
    info += "Kernel Type: " + QSysInfo::kernelType() + "\n";
    info += "Kernel Version: " + QSysInfo::kernelVersion() + "\n";
    
    return info;
} 