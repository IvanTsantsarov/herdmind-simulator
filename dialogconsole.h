#ifndef DIALOGCONSOLE_H
#define DIALOGCONSOLE_H

#include <QSettings>
#include <QRegularExpression>
#include <QDialog>

#define CONSOLE_COLOR_STRING_LENGHT 7

class QCloseEvent;

namespace Ui {
class DialogConsole;
}

class DialogConsole : public QDialog
{
    Q_OBJECT

    QString mLastMessage;
    int mLastMessageCount = 0;
    QIcon mIconFatal, mIconError;

    QColor mColorBack, mColorDebug, mColorInfo,
        mColorWarning, mColorError, mColorFatal;

    int mWarningsCount, mErrorsCount;


    int appendText(const QString& msg, QColor col);

    void closeEvent(QCloseEvent* e);
    void addImportant(int block,
                      const QString &txt,
                      bool isFatal);

public:
    explicit DialogConsole(const QSettings& settings, QWidget *parent = nullptr);
    ~DialogConsole();
    void fatal(const QString& msg);
    void error(const QString& msg);
    void warning(const QString& msg);
    void info(const QString& msg);
    void debug(const QString& msg);

    bool isDebugInfo();
    void setDebugInfo(bool is);
    inline bool warningsCount(){ return mWarningsCount; }
    inline bool errorsCount(){ return mErrorsCount; }

private slots:
    void on_btnClear_clicked();


    void on_btnGo_clicked();

    void on_comboImportant_currentTextChanged(const QString &arg1);

private:
    Ui::DialogConsole *ui;
};

#endif // DIALOGCONSOLE_H
