#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QRegularExpression>
#include <QDialog>
#include <QSettings>

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

    QSettings& mSettings;
    QWidget* mLastErrorWidget = nullptr;
    QColor mLastErrorColor;

    QColor setColor(QWidget* w, const QColor& newColor);

    static const QRegularExpression mRegexProfileId;
    static const QRegularExpression mRegexId;
    static const QRegularExpression mRegexKey;

    void loadSettings();
    void saveSettings();

    void error(const QString& str, QWidget *w);
public:
    explicit DialogSettings(QSettings& s, QWidget *parent);
    ~DialogSettings();

    bool checkValues();

private slots:

    void on_btnCancel_clicked();

    void on_btnOk_clicked();

private:
    Ui::DialogSettings *ui;
};




#endif // DIALOGSETTINGS_H
