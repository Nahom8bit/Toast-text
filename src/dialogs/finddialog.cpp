#include "dialogs/finddialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Find and Replace"));
    setModal(false);

    // Create widgets
    searchLineEdit = new QLineEdit;
    replaceLineEdit = new QLineEdit;
    caseSensitiveCheckBox = new QCheckBox(tr("&Case sensitive"));
    wholeWordsCheckBox = new QCheckBox(tr("&Whole words only"));
    searchBackwardsCheckBox = new QCheckBox(tr("Search &backwards"));
    wrapAroundCheckBox = new QCheckBox(tr("&Wrap around"));
    
    findNextButton = new QPushButton(tr("Find &Next"));
    findPreviousButton = new QPushButton(tr("Find &Previous"));
    replaceButton = new QPushButton(tr("&Replace"));
    replaceAllButton = new QPushButton(tr("Replace &All"));
    closeButton = new QPushButton(tr("Close"));
    
    messageLabel = new QLabel;
    messageLabel->setStyleSheet("QLabel { color: red }");
    messageLabel->hide();

    // Set default states
    findNextButton->setDefault(true);
    findNextButton->setEnabled(false);
    findPreviousButton->setEnabled(false);
    replaceButton->setEnabled(false);
    replaceAllButton->setEnabled(false);

    // Create layouts
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGridLayout *topLayout = new QGridLayout;
    QHBoxLayout *bottomLayout = new QHBoxLayout;

    // Add widgets to layouts
    topLayout->addWidget(new QLabel(tr("Find what:")), 0, 0);
    topLayout->addWidget(searchLineEdit, 0, 1);
    topLayout->addWidget(new QLabel(tr("Replace with:")), 1, 0);
    topLayout->addWidget(replaceLineEdit, 1, 1);

    // Create options group
    QGroupBox *optionsGroup = new QGroupBox(tr("Options"));
    QVBoxLayout *optionsLayout = new QVBoxLayout;
    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addWidget(searchBackwardsCheckBox);
    optionsLayout->addWidget(wrapAroundCheckBox);
    optionsGroup->setLayout(optionsLayout);

    // Add buttons to bottom layout
    bottomLayout->addWidget(findNextButton);
    bottomLayout->addWidget(findPreviousButton);
    bottomLayout->addWidget(replaceButton);
    bottomLayout->addWidget(replaceAllButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(closeButton);

    // Combine all layouts
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(optionsGroup);
    mainLayout->addWidget(messageLabel);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);

    // Connect signals and slots
    connect(searchLineEdit, &QLineEdit::textChanged,
            this, &FindDialog::enableFindButton);
    connect(findNextButton, &QPushButton::clicked,
            this, &FindDialog::findNext);
    connect(findPreviousButton, &QPushButton::clicked,
            this, &FindDialog::findPrevious);
    connect(replaceButton, &QPushButton::clicked,
            this, &FindDialog::replace);
    connect(replaceAllButton, &QPushButton::clicked,
            this, &FindDialog::replaceAll);
    connect(closeButton, &QPushButton::clicked,
            this, &QDialog::close);
}

void FindDialog::enableFindButton(const QString &text)
{
    bool enable = !text.isEmpty();
    findNextButton->setEnabled(enable);
    findPreviousButton->setEnabled(enable);
    replaceButton->setEnabled(enable);
    replaceAllButton->setEnabled(enable);
    messageLabel->hide();
}

QString FindDialog::searchText() const
{
    return searchLineEdit->text();
}

QString FindDialog::replaceText() const
{
    return replaceLineEdit->text();
}

bool FindDialog::caseSensitive() const
{
    return caseSensitiveCheckBox->isChecked();
}

bool FindDialog::wholeWords() const
{
    return wholeWordsCheckBox->isChecked();
}

bool FindDialog::searchBackwards() const
{
    return searchBackwardsCheckBox->isChecked();
}

bool FindDialog::wrapAround() const
{
    return wrapAroundCheckBox->isChecked();
}

void FindDialog::showMessage(const QString &message)
{
    messageLabel->setText(message);
    messageLabel->show();
}

void FindDialog::replace()
{
    emit replaceRequested();
}

void FindDialog::replaceAll()
{
    emit replaceAllRequested();
} 