#ifndef RECOVERYDIALOG_H
#define RECOVERYDIALOG_H

#include <QDialog>
#include <QDateTime>

class QLabel;
class QPushButton;

class RecoveryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecoveryDialog(const QString &filePath, const QDateTime &lastSaved, QWidget *parent = nullptr);

signals:
    void recoverSession();
    void discardSession();

private:
    void setupUi();
    void createConnections();

    QLabel *messageLabel;
    QPushButton *recoverButton;
    QPushButton *discardButton;
    QPushButton *cancelButton;
};

#endif // RECOVERYDIALOG_H 