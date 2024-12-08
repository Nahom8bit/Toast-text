#include <QtWidgets/QApplication>
#include "mainwindow.h"
#include "crashhandler.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application info
    QApplication::setApplicationName("Text Editor");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("TextEditor");
    QApplication::setOrganizationDomain("texteditor.org");
    
    // Initialize crash handler
    CrashHandler::initialize();
#ifdef QT_DEBUG
    CrashHandler::enableDebugMode(true);
#endif
    
    LOG_DEBUG("Application started");
    
    MainWindow w;
    w.show();
    
    int result = app.exec();
    LOG_DEBUG("Application exiting with code: " + QString::number(result));
    return result;
} 