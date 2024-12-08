#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QMessageBox>

class FileManager : public QObject {
    Q_OBJECT

public:
    explicit FileManager(QObject *parent = nullptr);
    
    bool openFile(const QString &path, QString &content);
    bool saveFile(const QString &path, const QString &content);
    
    static QString showOpenDialog(QWidget *parent = nullptr);
    static QString showSaveDialog(QWidget *parent = nullptr);
    
    QStringList getRecentFiles() const;
    void addRecentFile(const QString &path);
    void clearRecentFiles();

signals:
    void fileOpened(const QString &path);
    void fileSaved(const QString &path);
    void error(const QString &message);

private:
    static const int MaxRecentFiles = 10;
    QStringList recentFiles;
    
    void loadRecentFiles();
    void saveRecentFiles();
    QString readFile(const QString &path);
    bool writeFile(const QString &path, const QString &content);
};

#endif 