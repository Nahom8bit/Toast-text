#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    QString searchText() const;
    QString replaceText() const;
    bool caseSensitive() const;
    bool wholeWords() const;
    bool searchBackwards() const;
    bool wrapAround() const;

public slots:
    void showMessage(const QString &message);

signals:
    void findNext();
    void findPrevious();
    void replaceRequested();
    void replaceAllRequested();

private slots:
    void enableFindButton(const QString &text);
    void replace();
    void replaceAll();

private:
    QLineEdit *searchLineEdit;
    QLineEdit *replaceLineEdit;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordsCheckBox;
    QCheckBox *searchBackwardsCheckBox;
    QCheckBox *wrapAroundCheckBox;
    QPushButton *findNextButton;
    QPushButton *findPreviousButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QPushButton *closeButton;
    QLabel *messageLabel;
};

#endif // FINDDIALOG_H 