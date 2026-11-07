
#define SLEEP_SEC 10
#define PIN_LED 35

int secCount;
uint64_t msecCount;

class Session {
  static RTC_DATA_ATTR uint32_t mBootCount;
public: 
  static bool isFirstBoot(){ return 0 == Session::mBootCount; };
  static uint32_t bootCount(){ return Session::mBootCount; };
  static void incBootCount(){ Session::mBootCount++; }

};


RTC_DATA_ATTR uint32_t Session::mBootCount = 0;

void blink(int ms) {
    digitalWrite(PIN_LED, HIGH); 
    delay(ms);
    digitalWrite(PIN_LED, LOW); 
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(115200);
  // while(!Serial);
  if( Session::isFirstBoot() ) {
    Serial.println("\n=========>");  
  }
  Serial.printf("Boot count=%d\n", Session::bootCount());
  blink(10);

  msecCount = millis();
  secCount = 0;

}


void loop() {
  // put your main code here, to run repeatedly:

  if( (millis() - msecCount) < 1000 ) {
    delay(1);
    return;
  }
  
  msecCount = millis();
  secCount ++;
  Serial.println(secCount);
  if( secCount < SLEEP_SEC) {
      blink(10);
      return;
  }
  

  Session::incBootCount();

  Serial.println("Going into sleep...");
  blink(100);
  
  esp_sleep_enable_timer_wakeup(1000000ULL * SLEEP_SEC);
  esp_deep_sleep_start();
}
