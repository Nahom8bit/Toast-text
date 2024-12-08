#ifndef AUTOCORRECTDIALOG_H
#define AUTOCORRECTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QMap>

class AutoCorrectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AutoCorrectDialog(QMap<QString, QString>& rules, QWidget *parent = nullptr);
    
signals:
    void rulesChanged();

private slots:
    void addRule();
    void removeRule();
    void saveRules();

private:
    void setupUi();
    void loadRules();
    
    QTableWidget *rulesTable;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *saveButton;
    QPushButton *closeButton;
    QMap<QString, QString>& autocorrectRules;
};

#endif // AUTOCORRECTDIALOG_H 