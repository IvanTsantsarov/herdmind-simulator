#ifndef SOUND_H
#define SOUND_H

#include "signaling.h"


#define FREQ_A 440.0f

enum NoteSign
{
    NoteSign_A = 0,
    NoteSign_B = 1,
    NoteSign_C = 2,
    NoteSign_D = 3,
    NoteSign_E = 4,
    NoteSign_F = 5,
    NoteSign_G = 6
};

enum NoteSuffix
{
    NoteSuffix_None = 0,
    NoteSuffix_Bemol = -1,
    NoteSuffix_Diez = 1
};

class Sound : public Signaling
{
    int mOctave = 4;
    uint8_t mVolume = 128; // 128 is max
public:
    Sound(Collar* c);
    virtual ~Sound() { }
    void setup();
    const char* name() { return "Sound"; }

    void kill();
    void execute(uint8_t mode = 0);

    void setVolume(uint8_t v){ mVolume = v;}

    float getNoteFreq(NoteSign note, int octave, NoteSuffix suffix = NoteSuffix_None);
    void makeSound(int freq, int msec);

    void beep(char tone, int msec = -1);
    void beep(NoteSign note, int msec = -1);
    void setOctave(int octave){ mOctave = octave; }
    // void setVolume(char vol){ mVolume = vol; }

    void playNote(char tone, char octave, int msec = -1);
    void play(const char* music, int msec);
};

#endif // SOUND_H
