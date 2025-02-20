#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1);  // ESP8266 I2C (SDA = D2, SCL = D1)

    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Sets RTC to current PC time
    }
}

void loop() {
    DateTime now = rtc.now();  // Get current time from RTC

    Serial.printf("Time: %02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
    delay(1000);
}
