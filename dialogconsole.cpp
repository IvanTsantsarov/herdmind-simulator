#include <QDateTime>
#include <QCloseEvent>
#include <QFontDatabase>
#include "mainwindow.h"
#include "dialogconsole.h"
#include "ui_dialogconsole.h"

#define CONSOLE_STRING_FORMAT "[hh:mm:ss.zzz] "

void DialogConsole::appendText(const QString &msg, QColor col)
{
    QString wholeMsg;
    QString dateTimeStr = QDateTime::currentDateTime().toString(CONSOLE_STRING_FORMAT);
    int len = msg.length();

    if( len > CONSOLE_COLOR_STRING_LENGHT && '#' == msg[0] ) {
        QString color = msg.left(CONSOLE_COLOR_STRING_LENGHT);
        ui->out->setTextColor(color);
        wholeMsg = QString("%1%2")
                       .arg(dateTimeStr)
                       .arg(msg.right(len - CONSOLE_COLOR_STRING_LENGHT));
    }else {
        ui->out->setTextColor(col);
        wholeMsg = QString("%1%2")
                       .arg(dateTimeStr)
                       .arg(msg);
    }

    ui->out->append(wholeMsg);
}

void DialogConsole::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    gMainWindow->onConsoleClose();
}

DialogConsole::DialogConsole(const QSettings& settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogConsole)
{
    ui->setupUi(this);
    ui->out->setReadOnly(true);

    mColorBack    = QColor(settings.value("Console/ColorBack").toString());
    mColorDebug   = QColor(settings.value("Console/ColorDebug").toString());
    mColorInfo    = QColor(settings.value("Console/ColorInfo").toString());
    mColorWarning = QColor(settings.value("Console/ColorWarning").toString());
    mColorError   = QColor(settings.value("Console/ColorError").toString());
    mColorFatal   = QColor(settings.value("Console/ColorFatal").toString());

    ui->checkDebug->setChecked( settings.value("Console/isDebug").toBool() );

    QPalette p = ui->out->palette();
    p.setColor(QPalette::Base, mColorBack); // BG
    ui->out->setPalette(p);
    ui->out->setTextBackgroundColor(mColorBack);

    QFont fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->out->setFont( fnt );
}

DialogConsole::~DialogConsole()
{
    delete ui;
}

void DialogConsole::fatal(const QString &msg)
{
    appendText(msg, mColorFatal);
}

void DialogConsole::error(const QString &msg)
{
    appendText(msg, mColorError);
}

void DialogConsole::warning(const QString &msg)
{
    appendText(msg, mColorWarning);
}

void DialogConsole::info(const QString &msg)
{
    appendText(msg, mColorInfo);
}

void DialogConsole::debug(const QString &msg)
{
    appendText(msg, mColorDebug);
}

bool DialogConsole::isDebugInfo()
{
    return ui->checkDebug->isChecked();
}

void DialogConsole::setDebugInfo(bool is)
{
    ui->checkDebug->setChecked(is);
}


void DialogConsole::on_btnClear_clicked()
{
    ui->out->clear();
}


void DialogConsole::on_btnSend_clicked()
{

}

