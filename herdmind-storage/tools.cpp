#include <QIODevice>
#include <QRandomGenerator>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>

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

bool Tools::fileCompare(const QString &pathFile1, const QString &pathFile2, bool* isOk )
{
    QFile f1(pathFile1);
    QFile f2(pathFile2);

    if(isOk) *isOk = true;

    if( f1.size() != f2.size() ) {
        return false;
    }

    QByteArray data1 = Tools::fileRead(pathFile1, isOk);
    if( isOk && !*isOk) return false;

    QByteArray data2 = Tools::fileRead(pathFile2, isOk);
    if( isOk && !*isOk) return false;

    const QByteArray md51 = QCryptographicHash::hash(data1, QCryptographicHash::Md5);
    const QByteArray md52 = QCryptographicHash::hash(data2, QCryptographicHash::Md5);

    return md51 == md52;
}

// returns true if pathFile1 is newer than pathFile2
bool Tools::fileIsNewer(const QString &pathFile1, const QString &pathFile2)
{
    QFileInfo file1(pathFile1);
    QFileInfo file2(pathFile2);

    return file1.lastModified() > file2.lastModified();
}

bool Tools::fileRestoreResources(const QString &fileName)
{
    QString srcPath = "://" + fileName;
    bool areDifferent = false;
    bool isExists = false;

    if( !QFile::exists(fileName) ) {
        areDifferent = true;
    }else {
        isExists = true;
        bool isOk = false;
        areDifferent = !Tools::fileCompare(srcPath, fileName, &isOk);
        if( !isOk ) {
            qCritical() << "Files" << srcPath << fileName << "error";
            return false;
        }
    }

    if( !areDifferent ) {
        return true;
    }

    if( !Tools::fileIsNewer(srcPath, fileName) ) {
        qInfo() << "Destination file" << fileName << "is newer than" << srcPath << "and will be overwritten.";
        return true;
    }

    QFile f(srcPath);

    if( isExists ) {
        if( !QFile::remove(fileName)) {
            qCritical() << "Error restoring res file - destination file not writable:" << fileName;
            return false;
        }
    }

    if( !f.copy(fileName) ) {
        qCritical() << "Error restoring res file" << fileName << f.errorString();
        return false;
    }

    qInfo() << "File" << fileName << " restored from resources.";

    return true;
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


