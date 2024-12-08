#include "dialogs/recoverydialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

RecoveryDialog::RecoveryDialog(const QString &filePath, const QDateTime &lastSaved, QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    createConnections();
    
    QString message = tr("An auto-saved version of '%1' from %2 was found.\n"
                        "Would you like to recover this version?")
                        .arg(filePath)
                        .arg(lastSaved.toString("yyyy-MM-dd hh:mm:ss"));
    messageLabel->setText(message);
}

void RecoveryDialog::setupUi()
{
    messageLabel = new QLabel;
    messageLabel->setWordWrap(true);
    
    recoverButton = new QPushButton(tr("&Recover"));
    discardButton = new QPushButton(tr("&Discard"));
    cancelButton = new QPushButton(tr("Cancel"));
    
    recoverButton->setDefault(true);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(recoverButton);
    buttonLayout->addWidget(discardButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(messageLabel);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
    setWindowTitle(tr("Recover Auto-saved Document"));
    setFixedWidth(400);
}

void RecoveryDialog::createConnections()
{
    connect(recoverButton, &QPushButton::clicked, this, &RecoveryDialog::recoverSession);
    connect(discardButton, &QPushButton::clicked, this, &RecoveryDialog::discardSession);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
} 