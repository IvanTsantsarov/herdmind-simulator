#include <QDebug>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <termio.h>
#include <QMetaType>

#include "console_reader.h"

void ConsoleReader::init(bool isEcho){

    tcgetattr(0, &mOldSettings);
    mNewSettings = mOldSettings;
    mNewSettings.c_lflag |= ~ICANON; // disable buffered i/o
    mNewSettings.c_lflag &= isEcho ? ECHO : ~ECHO; // set echo mode
    tcsetattr(0, TCSANOW, &mNewSettings); // use these new terminal i/o settings now

}

void ConsoleReader::backspace(int count)
{
    for( int i = 0; i < count; i ++) {
        putchar(0x08); // backspace
        putchar(0x20); // space
        putchar(0x08); // backspace again
    }
}


ConsoleReader::ConsoleReader(bool isEcho)
{
    init(isEcho);
    mBuffer.reserve(TERMINAL_BUFFER_SIZE);
}

ConsoleReader::~ConsoleReader() {
    reset();
}

void printValue(char key)
{
    QString keyStr;
    keyStr.setNum(key, 16);
    qDebug() <<  keyStr;
}

void ConsoleReader::run() {

    char combination[2] = {0};

    // qRegisterMetaType<ConsoleReaderMeta>("ConsoleReaderMeta");

    while(!mIsGoodbye)
    {
        const char key = getChar();

        // printValue(key);

        switch (key) {

        // Enter
        case 0x0D:
        case 0x0A:
            // qDebug() << "Enter pressed";
            if( mBuffer.trimmed().isEmpty() ) {
                continue;
            }
            putchar(0x0A);
            putchar(0x0D);
            emit keyPressed(Key::Enter);
            emit textChanged();
            break;

        case 0x7F: // Backspace
            backspace();
            mBuffer.remove(-1, 1);
            emit keyPressed(Key::Backspace);
            emit textChanged();
            break;

        case 0x09: // Tab
            emit keyPressed(Key::Tab);
            break;

        case 0x1B: // Escape

            // qDebug() << ftell(stdin);

            combination[0] = getChar();
            // qDebug() << "read two";

            if( EOF != combination[0]  ) {
                combination[1] = getchar();
                // qDebug() << "read two";
                if( EOF != combination[1] ) {
                    switch (combination[1]) {
                        case 'A' : emit keyPressed(Key::Up); break;
                        case 'B' : emit keyPressed(Key::Down); break;
                        case 'C' : emit keyPressed(Key::Right); break;
                        case 'D' : emit keyPressed(Key::Left); break;
                    }
                }
            }

            break;

        default:
            if( 0x20 <= key || 0x7E >= key ) {
                putchar(key);
                mBuffer.append(key);
                emit textChanged();
            }
        }
    }
}

QString ConsoleReader::read() {
    QString result = mBuffer;
    mBuffer.clear();
    return result;
}

const QString &ConsoleReader::peak()
{
    return mBuffer;
}

void ConsoleReader::clearLine()
{
    backspace(mBuffer.length());
    mBuffer.clear();
}

void ConsoleReader::replace(const QString &newLine)
{
    clearLine();
    place(newLine);
}

void ConsoleReader::place(const QString &str)
{
    int len = str.length();
    for( int i = 0; i < len; i ++) {
        putchar(str[i].toLatin1());
    }

    mBuffer = str;
}

void ConsoleReader::bye()
{
    mIsGoodbye = true;
    terminate();
}

char ConsoleReader::getChar()
{
    mIsWaitingInput = true;
    char ch = getchar();
    mIsWaitingInput = false;
    return ch;
}
