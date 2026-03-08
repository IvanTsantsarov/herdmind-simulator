#ifndef TOOLS_H
#define TOOLS_H

#include <QSettings>
#include <QDateTime>

class Tools
{

public:
    Tools();

    static float clamped(float val, float min = 0.0f, float max = 1.0f);

    static int gen(int minVal, int maxVal);
    inline static int gen(int maxVal) { return gen(0, maxVal); }
    static QByteArray genHex(int bytesCount);

    static QByteArray fileRead(const QString& path, bool* isOk = nullptr);
    static bool fileExists(const QString& path);
    static bool fileWrite(const QString& path, const QByteArray& content, bool isOverwrite = true);
    static QDateTime deviceTimestamp(uint32_t tstamp);
    static QString deviceTimestampString(uint32_t tstamp);
};

extern Tools* gSimTools;

#endif // SIMTOOLS_H
