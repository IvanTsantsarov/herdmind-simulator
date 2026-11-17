#include "sound.h"

Sound snd;

void d(int secs = 1) {
  delay(secs * 1000);
}

void setup()
{
  snd.setup();
  Serial.begin(115200);
}

void loop()
{
  d();
  snd.play("A4C4E4A5E4F4e4", 200);
  snd.play("E4", 600);
  d(2);
  snd.play("A4C4E4A5E4F4e4E4", 200);
  snd.play("A4F4E4D4", 200);
  delay(300);
  snd.play("D4E4D4C4", 200);
  delay(200);
  snd.play("b4", 600);
  delay(100);
  snd.play("D4", 600);
  delay(100);
  snd.play("F4", 200);
  delay(100);
  snd.play("b4", 600);
  snd.play("A4", 100);
  d(2);
  snd.play("A4", 100);
  delay(100);
  snd.play("A4", 600);
  delay(50);
  snd.play("A4", 100);
  delay(50);
  snd.play("A4", 100);
  delay(300);
  snd.play("A4", 800);
  d(30);

}