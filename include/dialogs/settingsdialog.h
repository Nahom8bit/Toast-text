#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

class QTabWidget;
class QFontComboBox;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QColorDialog;
class QPushButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    // Getters for settings
    QString getFont() const;
    int getFontSize() const;
    int getTabSize() const;
    bool getAutoIndent() const;
    bool getLineWrapping() const;
    QString getTheme() const;
    bool getAutoSave() const;
    int getAutoSaveInterval() const;
    QColor getEditorBackground() const;
    QColor getEditorForeground() const;
    QColor getLineNumberBackground() const;
    QColor getLineNumberForeground() const;
    QColor getCurrentLineColor() const;

public slots:
    void accept() override;
    void reject() override;

private slots:
    void loadSettings();
    void saveSettings();
    void resetToDefaults();
    void updatePreview();
    void updateColorButtons();
    void chooseEditorBackgroundColor();
    void chooseEditorForegroundColor();
    void chooseLineNumberBackgroundColor();
    void chooseLineNumberForegroundColor();
    void chooseCurrentLineColor();

private:
    void createGeneralTab();
    void createEditorTab();
    void createThemeTab();
    void createAutoSaveTab();
    void setupConnections();
    void applySettings();

    QTabWidget *tabWidget;
    
    // General settings widgets
    QFontComboBox *fontComboBox;
    QSpinBox *fontSizeSpinBox;
    QComboBox *themeComboBox;
    
    // Editor settings widgets
    QSpinBox *tabSizeSpinBox;
    QCheckBox *autoIndentCheckBox;
    QCheckBox *lineWrappingCheckBox;
    
    // Theme settings widgets
    QPushButton *editorBackgroundButton;
    QPushButton *editorForegroundButton;
    QPushButton *lineNumberBackgroundButton;
    QPushButton *lineNumberForegroundButton;
    QPushButton *currentLineButton;
    QWidget *previewWidget;
    
    // Auto-save settings widgets
    QCheckBox *autoSaveCheckBox;
    QSpinBox *autoSaveIntervalSpinBox;
    
    // Color settings
    QColor editorBackgroundColor;
    QColor editorForegroundColor;
    QColor lineNumberBackgroundColor;
    QColor lineNumberForegroundColor;
    QColor currentLineColor;
    
    QSettings settings;
};

#endif // SETTINGSDIALOG_H 