#include "sound.h"

#define PIN_SOUND 45

Sound::Sound(){}

void Sound::setup()
{
    // Attach pin at 2000 Hz with 8-bit resolution (0–255)
    ledcAttach(PIN_SOUND, 2000, 8);
    kill();
}


void Sound::kill()
{
    // Stop sound
    ledcWrite(PIN_SOUND, 0);
}

void Sound::execute()
{
    play("A4B4A4B4A4B4A4B4", 100);
}

void Sound::makeSound(int freq, int msec)
{

    // Update frequency on the already-attached pin
    ledcChangeFrequency(PIN_SOUND, freq, 8);

    // Start sound
    ledcWrite(PIN_SOUND, mVolume);
    if( msec > 0)
    {
        delay(msec);
        kill();
    }
}

float Sound::getNoteFreq(NoteSign note, int octave, NoteSuffix suffix)
{
    int steps = 0;
    switch(note)
    {
    case NoteSign_A: steps = 0; break;
    case NoteSign_B: steps = 2; break;
    case NoteSign_C: steps = 3; break;
    case NoteSign_D: steps = 5; break;
    case NoteSign_E: steps = 7; break;
    case NoteSign_F: steps = 8; break;
    case NoteSign_G: steps = 10; break;
    }

    int posNote = octave * 12 + steps;
    int dist = posNote - 48;

    switch(suffix)
    {
    case NoteSuffix_Bemol: dist --; break;
    case NoteSuffix_Diez: dist ++; break;
    default: break;
    }


    return FREQ_A * powf(1.059463094359f, dist);
    // return FREQ_A * powf(2.0f, (float)dist/12.0f);
}


void Sound::beep(char tone, int msec)
{
    auto isNote = tone >= 'A' && tone <= 'G';
    bool isBemol = tone >= 'a' && tone <= 'g';

    if( !isNote && !isBemol ) {
        return;
    }
    
    NoteSuffix suffix = isBemol ? NoteSuffix_Bemol : NoteSuffix_None;
    NoteSign note = (NoteSign)((int)NoteSign_A + (int)tone - (int)(isBemol ? 'a' : 'A'));

    beep(note, suffix, msec);
}

void Sound::beep(NoteSign note, NoteSuffix suffix, int msec)
{
#ifdef ONPC
    // gSim.beep(getNoteFreq(note, mOctave), msec);
#else
    makeSound(getNoteFreq(note, mOctave, suffix), msec);
#endif

}

int Sound::playNote(char tone, char octave, int msec)
{
    if( tone == '-')
    {
        delay(msec);
        return 1;
    }

    if( octave >= '1' && octave <= '8' )
        setOctave( octave - '0');

    beep(tone, msec);
    return 2;
}


void Sound::play(const char* music, int msec)
{
    int octave = mOctave;
    int len = strlen(music);
    if( len % 2 || len < 2 )
    {
        Serial.println("Sound:Notes sequences length must be even (note+octave). For example:A3B3A4F4");
        return;
    }


    len = 0;
    for( int i = 0; music[i]; i += len ) {
        len = playNote(music[i], music[i+1], msec);
    }


    setOctave(octave);
}