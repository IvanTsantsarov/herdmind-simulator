#ifndef CONSOLE_READER_H
#define CONSOLE_READER_H

#include <QThread>
#include <termios.h>

#define TERMINAL_BUFFER_SIZE 1024
#define TERMINAL_BUFFER_SIZE_BIG TERMINAL_BUFFER_SIZE * 30

/*
struct ConsoleReaderMeta
{
    int id;
    QString name;
};

Q_DECLARE_METATYPE(ConsoleReaderMeta)
*/

class ConsoleReader : public QThread
{
    Q_OBJECT

public:
    typedef enum{
        Enter = 0,
        Backspace,
        Tab,
        Escape,
        Up,
        Down,
        Right,
        Left
    }Key;

private:

    QString mBuffer;

    bool mIsGoodbye = false;
    bool mIsWaitingInput = false;

    struct termios mOldSettings;
    struct termios mNewSettings;

    void init(bool isEcho);

    void reset() {
        tcsetattr(0, TCSANOW, &mOldSettings);
    }

    void backspace(int count = 1);

    char getChar();

    void run();

signals:
    void keyPressed(ConsoleReader::Key key);
    void textChanged();

public:

    ConsoleReader(bool isEcho);
    ~ConsoleReader();

   QString read();
   const QString& peak();

   void clearLine();
   void replace(const QString& newLine);
   void place(const QString& str);
   void bye();

   inline bool isWaitingInput(){ return mIsWaitingInput; }
};


extern ConsoleReader gConsoleReader;

#endif  // CONSOLEREADER_H

