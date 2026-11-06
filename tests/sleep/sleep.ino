

#define SLEEP_SEC 10

class Session {
  static RTC_DATA_ATTR uint32_t mBootCount;
public: 
  static bool isFirstBoot(){ return 0 == Session::mBootCount; };
  static uint32_t bootCount(){ return Session::mBootCount; };
  static void incBootCount(){ Session::mBootCount++; }

};


RTC_DATA_ATTR uint32_t Session::mBootCount = 0;


void setup() {
  Serial.begin(115200);
  while(!Serial);
  if( Session::isFirstBoot() ) {
    Serial.println("\n***********");  
  }
  Serial.printf("Boot count=%d\n", Session::bootCount());

  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

  Session::incBootCount();

  delay(SLEEP_SEC * 1000);

  Serial.println("Going into sleep...");
  
  esp_sleep_enable_timer_wakeup(1000000ULL * SLEEP_SEC);
  esp_deep_sleep_start();
}
