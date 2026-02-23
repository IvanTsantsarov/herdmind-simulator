#include <QFontDatabase>
#include "dialogconsole.h"
#include "ui_dialogconsole.h"

void DialogConsole::appendText(const QString &msg, QColor col)
{
    ui->out->setTextColor(col);
    ui->out->append(msg);
}

DialogConsole::DialogConsole(const QSettings& settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogConsole)
{
    ui->setupUi(this);
    ui->out->setReadOnly(true);

    Qt::WindowFlags flags = windowFlags();
    Qt::WindowFlags closeFlag = Qt::WindowCloseButtonHint;
    flags = flags & (~closeFlag);
    setWindowFlags(flags);

    mColorBack    = QColor(settings.value("Console/ColorBack").toString());
    mColorDebug   = QColor(settings.value("Console/ColorDebug").toString());
    mColorInfo    = QColor(settings.value("Console/ColorInfo").toString());
    mColorWarning = QColor(settings.value("Console/ColorWarning").toString());
    mColorError   = QColor(settings.value("Console/ColorError").toString());
    mColorFatal   = QColor(settings.value("Console/ColorFatal").toString());

    QPalette p = ui->out->palette();
    p.setColor(QPalette::Base, mColorBack); // BG
    ui->out->setPalette(p);
    ui->out->setTextBackgroundColor(mColorBack);

    QFont fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fnt.setPointSize( ui->out->fontPointSize() );
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

bool DialogConsole::showDebug()
{
    return ui->checkDebug->isChecked();
}

