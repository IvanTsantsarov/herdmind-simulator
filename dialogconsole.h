#ifndef DIALOGCONSOLE_H
#define DIALOGCONSOLE_H

#include <QSettings>
#include <QDialog>

namespace Ui {
class DialogConsole;
}

class DialogConsole : public QDialog
{
    Q_OBJECT

    QColor mColorBack, mColorDebug, mColorInfo,
        mColorWarning, mColorError, mColorFatal;

    void appendText(const QString& msg, QColor col);
public:
    explicit DialogConsole(const QSettings& settings, QWidget *parent = nullptr);
    ~DialogConsole();
    void fatal(const QString& msg);
    void error(const QString& msg);
    void warning(const QString& msg);
    void info(const QString& msg);
    void debug(const QString& msg);

    bool showDebug();

private:
    Ui::DialogConsole *ui;
};

#endif // DIALOGCONSOLE_H
