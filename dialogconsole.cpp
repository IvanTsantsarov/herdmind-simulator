#include <QDateTime>
#include <QCloseEvent>
#include <QFontDatabase>
#include <QTextDocument>
#include <QTextBlock>
#include "mainwindow.h"
#include "dialogconsole.h"
#include "ui_dialogconsole.h"

#define CONSOLE_STRING_FORMAT "[hh:mm:ss.zzz] "

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

    mIconError = QIcon("://error.svg");
    mIconFatal = QIcon("://fatal.svg");


    // mMessagesCountRE = QRegularExpression(R"(^\s\(\d+\)$)");
}

DialogConsole::~DialogConsole()
{
    delete ui;
}

void DialogConsole::fatal(const QString &msg)
{
    int block = appendText(msg, mColorFatal);
    addImportant(block, msg, true);

}

void DialogConsole::error(const QString &msg)
{
    mErrorsCount++;
    int block = appendText(msg, mColorError);
    addImportant(block, msg, false);
}

void DialogConsole::warning(const QString &msg)
{
    mWarningsCount ++;
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
    ui->comboImportant->clear();
    mLastMessage = "";
    mLastMessageCount = 0;
}


// returns current block
int DialogConsole::appendText(const QString &msg, QColor col)
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
    return ui->out->document()->blockCount() - 1;
}

void DialogConsole::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    gMainWindow->onConsoleClose();
}

void DialogConsole::addImportant(int block, const QString& txt, bool isFatal)
{
    if( mLastMessage.isEmpty() || txt != mLastMessage) {
        mLastMessageCount = 1;
        mLastMessage = txt;
        ui->comboImportant->addItem(isFatal ? mIconFatal : mIconError, txt, block);
        return;
    }

    ui->comboImportant->setItemText(ui->comboImportant->count()-1, QString("%1 (%2)").arg(txt).arg(++mLastMessageCount));
}


void DialogConsole::on_btnGo_clicked()
{
    int row = ui->comboImportant->currentIndex();
    if( row < 0 ) {
        return;
    }

    int blockIndex = ui->comboImportant->currentData().toInt();
    QTextBlock block = ui->out->document()->findBlockByNumber(blockIndex);
    if (block.isValid()) {
        QTextCursor cursor(block);
        ui->out->setTextCursor(cursor);
        ui->out->ensureCursorVisible();
    }
}


void DialogConsole::on_comboImportant_currentTextChanged(const QString &arg1)
{
    ui->btnGo->setEnabled(ui->comboImportant->count());
}

