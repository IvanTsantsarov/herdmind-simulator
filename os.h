#ifndef OS_H
#define OS_H

#include <QString>
#include <QStringList>
#include <QProcess>


class ProcessShell;

struct ProcessContext {
    QString mOut, mErr;
    bool mIsStarted = false;
    bool mIsFinished = false;
    QProcess::ExitStatus mExitStatus;

    bool isDone(){ return mIsStarted && mIsFinished; }
    bool isCrashed(){ return mExitStatus == QProcess::CrashExit;}
};

class OS : public QObject
{
    Q_OBJECT

    QString mSuPass;
    QProcess mShellCmd;


    // for cpu
    static double mJiffy;
    static double mJiffyTime;

    struct CpuInfo
    {
        unsigned long long mIdle = 0;
        unsigned long long mBusy = 0;
        unsigned long long mTotal = 0;

    };

    CpuInfo mCpuInfo;

    static const char* skipParamsSpaces(const char* ptr, int count) {
        for( int i = 0; i < count; i ++ ) {
            while( *ptr && !isspace(*ptr) ) ptr++;
            while( *ptr && isspace(*ptr) ) ptr++;
        }
        return ptr;
    }

    void createFromRes(const QString &dirName, const QString& fileName, bool isExecutable );

public:
	OS();

	void init();
	void setSuPassword(const QString& pass){ mSuPass = pass; }
    bool runShellCmd(const QString &name, const QString& cmd, const QStringList &args = QStringList(),
                     int delayMsec = 300, ProcessContext* context = nullptr );
    QProcess *runProcess(QString cmd, bool isDetached);

    void chmod(QString path, QString mode, bool isRoot = false);
	bool shuthdown();
	bool restart();

    void restartApp();

    [[noreturn]]
    static void startAppBackground();

    static bool fileExists(const QString& filePath);

    QString rootPassword(){ return mSuPass; }

    static qulonglong memAll();
    static qulonglong memFree();
    static qulonglong memUsed();
    static double memUsedPerc();

    double cpuUsage(qint64 *busy = nullptr);

    static bool processStats(qint64 pid, unsigned long long &mem, unsigned long long &cpu, int &cpu_perc);

    static QString getEnvVar(const QString& name);
};

class ProcessShell : public QProcess
{
    Q_OBJECT
public:
    ProcessShell(const QString& name, const QString& cmd, const QStringList &args, int delayMsec, ProcessContext* context);

private:
    QString mName, mShellCmd;
    QStringList mArgs;
    QString mOutInternal, mErrInternal;
    ProcessContext *mContext = nullptr;

private slots:

    void onReadyReadStdout();
    void onReadyReadError();
    void onShellCmd();

};


extern OS gOS;

#endif // OS_H
