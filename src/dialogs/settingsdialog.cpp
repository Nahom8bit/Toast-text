#include "dialogs/settingsdialog.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QColorDialog>
#include <QStyleFactory>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), settings("TextEditor", "Settings")
{
    setWindowTitle(tr("Settings"));
    setModal(true);

    tabWidget = new QTabWidget;
    createGeneralTab();
    createEditorTab();
    createThemeTab();
    createAutoSaveTab();

    QPushButton *okButton = new QPushButton(tr("OK"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    QPushButton *resetButton = new QPushButton(tr("Reset to Defaults"));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);

    setupConnections();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::createGeneralTab()
{
    QWidget *generalTab = new QWidget;
    QGridLayout *layout = new QGridLayout;

    // Font settings
    QGroupBox *fontGroup = new QGroupBox(tr("Font"));
    QGridLayout *fontLayout = new QGridLayout;
    fontComboBox = new QFontComboBox;
    fontSizeSpinBox = new QSpinBox;
    fontSizeSpinBox->setRange(6, 72);
    
    fontLayout->addWidget(new QLabel(tr("Font:")), 0, 0);
    fontLayout->addWidget(fontComboBox, 0, 1);
    fontLayout->addWidget(new QLabel(tr("Size:")), 1, 0);
    fontLayout->addWidget(fontSizeSpinBox, 1, 1);
    fontGroup->setLayout(fontLayout);

    // Theme settings
    QGroupBox *themeGroup = new QGroupBox(tr("Theme"));
    QVBoxLayout *themeLayout = new QVBoxLayout;
    themeComboBox = new QComboBox;
    themeComboBox->addItems(QStringList() << "Light" << "Dark" << "Custom");
    themeLayout->addWidget(themeComboBox);
    themeGroup->setLayout(themeLayout);

    layout->addWidget(fontGroup, 0, 0);
    layout->addWidget(themeGroup, 1, 0);
    layout->setRowStretch(2, 1);
    
    generalTab->setLayout(layout);
    tabWidget->addTab(generalTab, tr("General"));
}

void SettingsDialog::createEditorTab()
{
    QWidget *editorTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    // Editor settings
    QGroupBox *editorGroup = new QGroupBox(tr("Editor Settings"));
    QGridLayout *editorLayout = new QGridLayout;
    
    tabSizeSpinBox = new QSpinBox;
    tabSizeSpinBox->setRange(1, 8);
    autoIndentCheckBox = new QCheckBox(tr("Auto-indent"));
    lineWrappingCheckBox = new QCheckBox(tr("Line wrapping"));
    
    editorLayout->addWidget(new QLabel(tr("Tab size:")), 0, 0);
    editorLayout->addWidget(tabSizeSpinBox, 0, 1);
    editorLayout->addWidget(autoIndentCheckBox, 1, 0, 1, 2);
    editorLayout->addWidget(lineWrappingCheckBox, 2, 0, 1, 2);
    
    editorGroup->setLayout(editorLayout);
    layout->addWidget(editorGroup);
    layout->addStretch();
    
    editorTab->setLayout(layout);
    tabWidget->addTab(editorTab, tr("Editor"));
}

void SettingsDialog::createThemeTab()
{
    QWidget *themeTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    // Color settings
    QGroupBox *colorGroup = new QGroupBox(tr("Colors"));
    QGridLayout *colorLayout = new QGridLayout;
    
    editorBackgroundButton = new QPushButton;
    editorForegroundButton = new QPushButton;
    lineNumberBackgroundButton = new QPushButton;
    lineNumberForegroundButton = new QPushButton;
    currentLineButton = new QPushButton;
    
    colorLayout->addWidget(new QLabel(tr("Editor Background:")), 0, 0);
    colorLayout->addWidget(editorBackgroundButton, 0, 1);
    colorLayout->addWidget(new QLabel(tr("Editor Text:")), 1, 0);
    colorLayout->addWidget(editorForegroundButton, 1, 1);
    colorLayout->addWidget(new QLabel(tr("Line Number Background:")), 2, 0);
    colorLayout->addWidget(lineNumberBackgroundButton, 2, 1);
    colorLayout->addWidget(new QLabel(tr("Line Numbers:")), 3, 0);
    colorLayout->addWidget(lineNumberForegroundButton, 3, 1);
    colorLayout->addWidget(new QLabel(tr("Current Line:")), 4, 0);
    colorLayout->addWidget(currentLineButton, 4, 1);
    
    colorGroup->setLayout(colorLayout);

    // Preview
    QGroupBox *previewGroup = new QGroupBox(tr("Preview"));
    QVBoxLayout *previewLayout = new QVBoxLayout;
    previewWidget = new QWidget;
    previewWidget->setMinimumHeight(100);
    previewLayout->addWidget(previewWidget);
    previewGroup->setLayout(previewLayout);

    layout->addWidget(colorGroup);
    layout->addWidget(previewGroup);
    layout->addStretch();
    
    themeTab->setLayout(layout);
    tabWidget->addTab(themeTab, tr("Theme"));
}

void SettingsDialog::createAutoSaveTab()
{
    QWidget *autoSaveTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    QGroupBox *autoSaveGroup = new QGroupBox(tr("Auto Save"));
    QGridLayout *autoSaveLayout = new QGridLayout;
    
    autoSaveCheckBox = new QCheckBox(tr("Enable auto-save"));
    autoSaveIntervalSpinBox = new QSpinBox;
    autoSaveIntervalSpinBox->setRange(1, 60);
    autoSaveIntervalSpinBox->setSuffix(tr(" minutes"));
    
    autoSaveLayout->addWidget(autoSaveCheckBox, 0, 0, 1, 2);
    autoSaveLayout->addWidget(new QLabel(tr("Save interval:")), 1, 0);
    autoSaveLayout->addWidget(autoSaveIntervalSpinBox, 1, 1);
    
    autoSaveGroup->setLayout(autoSaveLayout);
    layout->addWidget(autoSaveGroup);
    layout->addStretch();
    
    autoSaveTab->setLayout(layout);
    tabWidget->addTab(autoSaveTab, tr("Auto Save"));
}

void SettingsDialog::setupConnections()
{
    connect(editorBackgroundButton, &QPushButton::clicked,
            this, &SettingsDialog::chooseEditorBackgroundColor);
    connect(editorForegroundButton, &QPushButton::clicked,
            this, &SettingsDialog::chooseEditorForegroundColor);
    connect(lineNumberBackgroundButton, &QPushButton::clicked,
            this, &SettingsDialog::chooseLineNumberBackgroundColor);
    connect(lineNumberForegroundButton, &QPushButton::clicked,
            this, &SettingsDialog::chooseLineNumberForegroundColor);
    connect(currentLineButton, &QPushButton::clicked,
            this, &SettingsDialog::chooseCurrentLineColor);
    
    // Update preview when settings change
    connect(themeComboBox, &QComboBox::currentTextChanged,
            this, &SettingsDialog::updatePreview);
}

void SettingsDialog::loadSettings()
{
    // Load font settings
    QString fontFamily = settings.value("font/family", "Consolas").toString();
    int fontSize = settings.value("font/size", 11).toInt();
    fontComboBox->setCurrentFont(QFont(fontFamily));
    fontSizeSpinBox->setValue(fontSize);

    // Load editor settings
    tabSizeSpinBox->setValue(settings.value("editor/tabSize", 4).toInt());
    autoIndentCheckBox->setChecked(settings.value("editor/autoIndent", true).toBool());
    lineWrappingCheckBox->setChecked(settings.value("editor/lineWrapping", true).toBool());

    // Load theme settings
    themeComboBox->setCurrentText(settings.value("theme/name", "Light").toString());
    editorBackgroundColor = settings.value("theme/editorBackground", QColor(Qt::white)).value<QColor>();
    editorForegroundColor = settings.value("theme/editorForeground", QColor(Qt::black)).value<QColor>();
    lineNumberBackgroundColor = settings.value("theme/lineNumberBackground", QColor(Qt::lightGray)).value<QColor>();
    lineNumberForegroundColor = settings.value("theme/lineNumberForeground", QColor(Qt::black)).value<QColor>();
    currentLineColor = settings.value("theme/currentLine", QColor(Qt::yellow).lighter(160)).value<QColor>();

    // Load auto-save settings
    autoSaveCheckBox->setChecked(settings.value("autoSave/enabled", false).toBool());
    autoSaveIntervalSpinBox->setValue(settings.value("autoSave/interval", 5).toInt());

    updatePreview();
}

void SettingsDialog::saveSettings()
{
    // Save font settings
    settings.setValue("font/family", fontComboBox->currentFont().family());
    settings.setValue("font/size", fontSizeSpinBox->value());

    // Save editor settings
    settings.setValue("editor/tabSize", tabSizeSpinBox->value());
    settings.setValue("editor/autoIndent", autoIndentCheckBox->isChecked());
    settings.setValue("editor/lineWrapping", lineWrappingCheckBox->isChecked());

    // Save theme settings
    settings.setValue("theme/name", themeComboBox->currentText());
    settings.setValue("theme/editorBackground", editorBackgroundColor);
    settings.setValue("theme/editorForeground", editorForegroundColor);
    settings.setValue("theme/lineNumberBackground", lineNumberBackgroundColor);
    settings.setValue("theme/lineNumberForeground", lineNumberForegroundColor);
    settings.setValue("theme/currentLine", currentLineColor);

    // Save auto-save settings
    settings.setValue("autoSave/enabled", autoSaveCheckBox->isChecked());
    settings.setValue("autoSave/interval", autoSaveIntervalSpinBox->value());

    settings.sync();
}

void SettingsDialog::resetToDefaults()
{
    // Reset font settings
    fontComboBox->setCurrentFont(QFont("Consolas"));
    fontSizeSpinBox->setValue(11);

    // Reset editor settings
    tabSizeSpinBox->setValue(4);
    autoIndentCheckBox->setChecked(true);
    lineWrappingCheckBox->setChecked(true);

    // Reset theme settings
    themeComboBox->setCurrentText("Light");
    editorBackgroundColor = Qt::white;
    editorForegroundColor = Qt::black;
    lineNumberBackgroundColor = Qt::lightGray;
    lineNumberForegroundColor = Qt::black;
    currentLineColor = QColor(Qt::yellow).lighter(160);

    // Reset auto-save settings
    autoSaveCheckBox->setChecked(false);
    autoSaveIntervalSpinBox->setValue(5);

    updatePreview();
}

void SettingsDialog::updatePreview()
{
    QString theme = themeComboBox->currentText();
    if (theme == "Light") {
        editorBackgroundColor = Qt::white;
        editorForegroundColor = Qt::black;
        lineNumberBackgroundColor = Qt::lightGray;
        lineNumberForegroundColor = Qt::black;
        currentLineColor = QColor(Qt::yellow).lighter(160);
    } else if (theme == "Dark") {
        editorBackgroundColor = QColor(45, 45, 45);
        editorForegroundColor = Qt::white;
        lineNumberBackgroundColor = QColor(60, 60, 60);
        lineNumberForegroundColor = Qt::lightGray;
        currentLineColor = QColor(Qt::blue).darker(400);
    }

    updateColorButtons();
    // Update preview widget
    QString style = QString("background-color: %1; color: %2;")
                       .arg(editorBackgroundColor.name())
                       .arg(editorForegroundColor.name());
    previewWidget->setStyleSheet(style);
}

void SettingsDialog::updateColorButtons()
{
    QString buttonStyle = "QPushButton { background-color: %1; }";
    editorBackgroundButton->setStyleSheet(buttonStyle.arg(editorBackgroundColor.name()));
    editorForegroundButton->setStyleSheet(buttonStyle.arg(editorForegroundColor.name()));
    lineNumberBackgroundButton->setStyleSheet(buttonStyle.arg(lineNumberBackgroundColor.name()));
    lineNumberForegroundButton->setStyleSheet(buttonStyle.arg(lineNumberForegroundColor.name()));
    currentLineButton->setStyleSheet(buttonStyle.arg(currentLineColor.name()));
}

void SettingsDialog::chooseEditorBackgroundColor()
{
    QColor color = QColorDialog::getColor(editorBackgroundColor, this);
    if (color.isValid()) {
        editorBackgroundColor = color;
        updatePreview();
    }
}

void SettingsDialog::chooseEditorForegroundColor()
{
    QColor color = QColorDialog::getColor(editorForegroundColor, this);
    if (color.isValid()) {
        editorForegroundColor = color;
        updatePreview();
    }
}

void SettingsDialog::chooseLineNumberBackgroundColor()
{
    QColor color = QColorDialog::getColor(lineNumberBackgroundColor, this);
    if (color.isValid()) {
        lineNumberBackgroundColor = color;
        updatePreview();
    }
}

void SettingsDialog::chooseLineNumberForegroundColor()
{
    QColor color = QColorDialog::getColor(lineNumberForegroundColor, this);
    if (color.isValid()) {
        lineNumberForegroundColor = color;
        updatePreview();
    }
}

void SettingsDialog::chooseCurrentLineColor()
{
    QColor color = QColorDialog::getColor(currentLineColor, this);
    if (color.isValid()) {
        currentLineColor = color;
        updatePreview();
    }
}

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::reject()
{
    loadSettings(); // Restore previous settings
    QDialog::reject();
}

// Getter implementations
QString SettingsDialog::getFont() const { return fontComboBox->currentFont().family(); }
int SettingsDialog::getFontSize() const { return fontSizeSpinBox->value(); }
int SettingsDialog::getTabSize() const { return tabSizeSpinBox->value(); }
bool SettingsDialog::getAutoIndent() const { return autoIndentCheckBox->isChecked(); }
bool SettingsDialog::getLineWrapping() const { return lineWrappingCheckBox->isChecked(); }
QString SettingsDialog::getTheme() const { return themeComboBox->currentText(); }
bool SettingsDialog::getAutoSave() const { return autoSaveCheckBox->isChecked(); }
int SettingsDialog::getAutoSaveInterval() const { return autoSaveIntervalSpinBox->value(); }
QColor SettingsDialog::getEditorBackground() const { return editorBackgroundColor; }
QColor SettingsDialog::getEditorForeground() const { return editorForegroundColor; }
QColor SettingsDialog::getLineNumberBackground() const { return lineNumberBackgroundColor; }
QColor SettingsDialog::getLineNumberForeground() const { return lineNumberForegroundColor; }
QColor SettingsDialog::getCurrentLineColor() const { return currentLineColor; } 