#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sysinfo.h>


#include <QThread>
#include <QTimer>
#include <QDir>
#include <QMap>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QCoreApplication>
#include <QProcess>

#include "os.h"

#include "macros_str.h"

#define FILE_USERS "users"

#define UPDATE_SOUNDS_INTERVAL 1000


OS gOS;



OS::OS()
{
}

void OS::createFromRes(const QString& dirName, const QString& fileName, bool isExecutable)
{
    // copy update script in current directory
    QFile file(fileName);
    QFile fileResource( STR2(":/res/%1%2",dirName, fileName) );

    if( !file.exists() ) {
        if( !fileResource.copy(fileName) ) {
            qDebug() << STR1( "Error: cannot copy %1",fileName);
        }else if( isExecutable ){
#ifdef Q_OS_LINUX
            runShellCmd( "chmod", STR1("chmod a+x %1", fileName) );
#endif
        }
    }
}

void OS::init()
{
    // populate cpu info struct
    cpuUsage();
}

void OS::chmod(QString path, QString mode, bool isRoot)
{
    Q_UNUSED(isRoot);
    runShellCmd( "chmod", QString("chmod %1 %2").arg(mode).arg(path));
}

bool OS::shuthdown()
{
    return gOS.runShellCmd("shutdown", "shutdown -P now");
}

bool OS::restart()
{
    return gOS.runShellCmd("restart", "shutdown -r now");
}


bool OS::fileExists(const QString &filePath)
{
    return QFile::exists(filePath);
}

qulonglong OS::memAll()
{
    struct sysinfo meminfo;
    sysinfo(&meminfo);
    return meminfo.totalram * meminfo.mem_unit;
}

qulonglong OS::memFree()
{
    struct sysinfo meminfo;
    sysinfo(&meminfo);
    return meminfo.freeram * meminfo.mem_unit;
}

qulonglong OS::memUsed()
{
    struct sysinfo meminfo;
    sysinfo(&meminfo);
    qulonglong total = meminfo.totalram - meminfo.freeram;
    total *= meminfo.mem_unit;
    return total;
}

double OS::memUsedPerc()
{
    struct sysinfo meminfo;
    sysinfo(&meminfo);
    double total = 100.0 * static_cast<double>(meminfo.totalram - meminfo.freeram) /
            static_cast<double>(meminfo.totalram);
    return qBound( 0.0, total, 100.0 );
}

double OS::cpuUsage(qint64* busy)
{
    QFile file("/proc/stat");
    if( !file.open(QIODevice::ReadOnly) ) {
        return 0;
    }
    QByteArray line = file.readLine();
    file.close();

    unsigned long long int usertime, nicetime, systemtime, idletime;
    unsigned long long int ioWait, irq, softIrq, steal, guest, guestnice;

    ioWait = irq = softIrq = steal = guest = guestnice = 0;
    sscanf(line.data(), "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
           &usertime, &nicetime, &systemtime, &idletime, &ioWait, &irq, &softIrq, &steal, &guest, &guestnice);

    usertime = usertime - guest;
    nicetime = nicetime - guestnice;

    CpuInfo info, infoDelta;
    info.mBusy = usertime + nicetime + systemtime + irq + softIrq + steal;
    info.mIdle = idletime + ioWait;
    info.mTotal = info.mBusy + info.mIdle;

    infoDelta.mIdle = info.mIdle - mCpuInfo.mIdle;
    infoDelta.mBusy = info.mBusy - mCpuInfo.mBusy;
    infoDelta.mTotal = info.mTotal - mCpuInfo.mTotal;

    if( busy ) {
        *busy = infoDelta.mBusy;
    }

    mCpuInfo = info;

    return qBound( 0.0, static_cast<double>(infoDelta.mBusy) / static_cast<double>(infoDelta.mTotal) * 100.0, 100.0 );
}

bool OS::processStats(qint64 pid, unsigned long long& mem, unsigned long long& cpu, int &cpu_perc)
{
    QFile file( QString("/proc/%1/stat").arg(pid) );
    if( !file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray line = file.readLine();
    file.close();

    const char* ptr = line.data();

    ptr = skipParamsSpaces(ptr, 13);
    Q_ASSERT(*ptr);
    unsigned long long utime, stime, cutime, cstime;
    sscanf(ptr, "%16llu %16llu %16llu %16llu", &utime, &stime, &cutime, &cstime);
    unsigned long long newCpu = utime + stime + cutime + cstime;

    cpu_perc = qRound( static_cast<double>(newCpu - cpu) * 100.0
                       / static_cast<double>(sysconf(_SC_CLK_TCK)) );

    if( cpu_perc < 0 )  {
        cpu_perc = 0;
    }

    cpu = newCpu;

    ptr = skipParamsSpaces(ptr, 10);
    Q_ASSERT(*ptr);
    sscanf(ptr, "%16llu ", &mem);
    mem *= static_cast<unsigned long long>(sysconf(_SC_PAGESIZE));

    return true;
}

QString OS::getEnvVar(const QString &name)
{
    return qgetenv(name.toLatin1());
    // return qEnvironmentVariable(name.toLatin1());

}

bool OS::runShellCmd(const QString& name, const QString &cmd, int delayMsec, ProcessContext *context)
{
    qDebug() << "[OS]Running" << name << cmd;

    new ProcessShell(name, cmd, delayMsec, context);
    return true;
}

ProcessShell::ProcessShell(const QString &name, const QString &cmd, int delayMsec, ProcessContext *context) :
    QProcess (), mName(name), mShellCmd(cmd), mContext(context)
{
    connect( this, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStdout()) );
    connect( this, SIGNAL(readyReadStandardError()), this, SLOT(onReadyReadError()) );

    connect(this, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=](int , QProcess::ExitStatus status){

        if( mContext ) {
             mContext->mIsFinished = true;
             mContext->mOut.append(mOutInternal);
             mContext->mErr.append(mErrInternal);
             mContext->mExitStatus = status;
        }

        this->deleteLater();
    });

    connect(this, (&QProcess::started), [=](){

        if( mContext ) {
             mContext->mIsStarted = true;
        }
    });


    if( delayMsec ) {
        QTimer::singleShot( delayMsec, this, SLOT(onShellCmd()) );
    }else {
        onShellCmd();
    }
}

void ProcessShell::onShellCmd()
{
    start( mShellCmd );
    qDebug() << STR1("runShellCmd %1 started", mShellCmd);
}


void ProcessShell::onReadyReadStdout()
{
    QByteArray ba = readAllStandardOutput();
    QString strUtf8 = QString::fromUtf8(ba);
    // mOutInternal.append(strUtf8);
    qInfo() << STR1("[%1]", mName) << strUtf8;
    // gLog.info(2, str);
}

void ProcessShell::onReadyReadError()
{
    QByteArray ba = readAllStandardError();
    QString strUtf8 = QString::fromUtf8(ba);
    // mErrInternal.append(strUtf8);
    qCritical() << STR1("[%1]", mName) << strUtf8;
}


QProcess* OS::runProcess(QString cmd, bool isDetached)
{
    QProcess* process = new QProcess();

    if( isDetached ) {
        process->startDetached(cmd);
    } else {
        process->start(cmd);
    }

    return process;
}


void OS::startAppBackground()
{
    QStringList args( QCoreApplication::applicationDirPath() );
    QProcess::startDetached( QCoreApplication::applicationFilePath(), args );
    QCoreApplication::quit();
    exit(0);
}

void OS::restartApp()
{
    QCoreApplication::exit(1);
}

