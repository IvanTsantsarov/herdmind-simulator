#include <QIODevice>
#include <QRandomGenerator>
#include <QFile>

#include "tools.h"

Tools::Tools()
{
}


int Tools::gen(int minVal, int maxVal)
{
    return QRandomGenerator::global()->bounded(minVal, maxVal);
}

QByteArray Tools::genHex(int bytesCount)
{
    QByteArray result;
    int charsCount = bytesCount*2;
    result.reserve(charsCount);

    for( auto i = 0; i < charsCount; i ++) {
        int v = gen(0, 15 );
        int ch = v < 10 ? v + '0' : v + 'A' - 10;
        result.append(ch);
    }

    return result;
}

QByteArray Tools::fileRead(const QString &path, bool *isOk)
{
    QFile file( path );
    if ( file.open(QIODevice::ReadOnly) )
    {
        if( isOk ) {
            *isOk = true;
        }
        return file.readAll();
    }

    if( isOk ) {
        *isOk = false;
    }

    return QByteArray();
}

bool Tools::fileExists(const QString &path)
{
    bool isFileExisting = QFile::exists(path);
    return isFileExisting;
}

bool Tools::fileWrite(const QString &path, const QByteArray &content, bool isOverwrite)
{
    if( !isOverwrite && QFile::exists(path) ) {
        return true;
    }

    QFile file( path );
    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << content;
        return true;
    }

    return false;
}

QDateTime Tools::deviceTimestamp(uint32_t tstamp)
{
    return QDateTime(QDate(2000, 1, 1), QTime(0, 0)).addSecs(tstamp);
}

QString Tools::deviceTimestampString(uint32_t tstamp)
{
    QDateTime dt = deviceTimestamp(tstamp);
    return dt.toString("[hh:mm:ss dd:MM:yyyy]");
}


