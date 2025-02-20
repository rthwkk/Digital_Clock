#include <Wire.h>
#include <RTClib.h>
#include <TM1637Display.h>

// Define RTC and 7-segment display
RTC_DS3231 rtc;
#define CLK D6  // GPIO12
#define DIO D5  // GPIO14
TM1637Display display(CLK, DIO);

bool colonState = true; // To toggle the colon

void setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1); // ESP8266 I2C (SDA = D2, SCL = D1)

    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    // Check if RTC lost power, then set time from the system
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set time to compile time
    }

    // Initialize TM1637 display
    display.setBrightness(7);  // Set brightness (0-7)
}

void loop() {
    DateTime now = rtc.now(); // Get current time
    int hour = now.hour();
    int minute = now.minute();

    Serial.printf("Time: %02d:%02d\n", hour, minute);

    // Convert time to 4-digit format (HHMM)
    int displayTime = (hour * 100) + minute;

    // Toggle colon every second
    if (colonState) {
        display.showNumberDecEx(displayTime, 0b01000000, true); // Colon ON
    } else {
        display.showNumberDec(displayTime, true); // Colon OFF
    }

    colonState = !colonState; // Flip colon state

    delay(1000); // Wait for 1 second
}
