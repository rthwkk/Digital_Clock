#include <Wire.h>
#include <RTClib.h>
#include <TM1637Display.h>

// Define RTC and 7-segment display
RTC_DS3231 rtc;
#define CLK D6  // GPIO12
#define DIO D5  // GPIO14
TM1637Display display(CLK, DIO);

bool colonState = true; // To toggle the colon
unsigned long previousMillis = 0;
const long tempInterval = 20000; // 20 seconds interval
const long tempDisplayDuration = 2000; // 2 seconds duration
bool showTemperature = false;

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
    unsigned long currentMillis = millis();
    
    if (showTemperature && (currentMillis - previousMillis >= tempDisplayDuration)) {
        showTemperature = false; // Switch back to time display after 2 seconds
    }
    
    if (!showTemperature && (currentMillis - previousMillis >= tempInterval)) {
        previousMillis = currentMillis;
        showTemperature = true; // Show temperature every 20 seconds
    }
    
    if (showTemperature) {
        float tempC = rtc.getTemperature(); // Get temperature in Celsius
        int tempDisplay = (int)tempC * 100; // Convert to 4-digit format (XX.00)
        display.showNumberDec(tempDisplay, false);
        Serial.printf("Temperature: %.2f°C\n", tempC);
    } else {
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
    }
    
    delay(1000); // Update every second
}
