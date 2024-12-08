#include "dialogs/autocorrectdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

AutoCorrectDialog::AutoCorrectDialog(QMap<QString, QString>& rules, QWidget *parent)
    : QDialog(parent)
    , autocorrectRules(rules)
{
    setupUi();
    loadRules();
}

void AutoCorrectDialog::setupUi()
{
    rulesTable = new QTableWidget(0, 2);
    rulesTable->setHorizontalHeaderLabels({"Type This", "Replace With"});
    rulesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    rulesTable->verticalHeader()->hide();
    
    addButton = new QPushButton(tr("Add Rule"));
    removeButton = new QPushButton(tr("Remove Rule"));
    saveButton = new QPushButton(tr("Save"));
    closeButton = new QPushButton(tr("Close"));
    
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(closeButton);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(rulesTable);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
    setWindowTitle(tr("Auto-Correct Rules"));
    resize(400, 300);
    
    connect(addButton, &QPushButton::clicked, this, &AutoCorrectDialog::addRule);
    connect(removeButton, &QPushButton::clicked, this, &AutoCorrectDialog::removeRule);
    connect(saveButton, &QPushButton::clicked, this, &AutoCorrectDialog::saveRules);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void AutoCorrectDialog::loadRules()
{
    rulesTable->setRowCount(0);
    for (auto it = autocorrectRules.begin(); it != autocorrectRules.end(); ++it) {
        int row = rulesTable->rowCount();
        rulesTable->insertRow(row);
        rulesTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        rulesTable->setItem(row, 1, new QTableWidgetItem(it.value()));
    }
}

void AutoCorrectDialog::addRule()
{
    int row = rulesTable->rowCount();
    rulesTable->insertRow(row);
    rulesTable->setItem(row, 0, new QTableWidgetItem(""));
    rulesTable->setItem(row, 1, new QTableWidgetItem(""));
    rulesTable->setCurrentCell(row, 0);
    rulesTable->editItem(rulesTable->item(row, 0));
}

void AutoCorrectDialog::removeRule()
{
    int currentRow = rulesTable->currentRow();
    if (currentRow >= 0) {
        rulesTable->removeRow(currentRow);
    }
}

void AutoCorrectDialog::saveRules()
{
    autocorrectRules.clear();
    for (int row = 0; row < rulesTable->rowCount(); ++row) {
        QString key = rulesTable->item(row, 0)->text().trimmed();
        QString value = rulesTable->item(row, 1)->text().trimmed();
        
        if (!key.isEmpty() && !value.isEmpty()) {
            autocorrectRules[key] = value;
        }
    }
    
    emit rulesChanged();
    QMessageBox::information(this, tr("Auto-Correct"), tr("Rules saved successfully."));
} 