#include <Wire.h>
#include <RTClib.h>
#include <TM1637Display.h>

// Define RTC and 7-segment display
RTC_DS3231 rtc;
#define CLK D4  // GPIO12
#define DIO D3  // GPIO14
TM1637Display display(CLK, DIO);

bool colonState = true;
unsigned long previousMillis = 0;
const long interval = 20000;
const long displayDuration = 2000;
bool showTemperature = false;

// Button pin
#define BTN_HOUR D7 // GPIO0

// Button handling variables
const long debounceDelay = 50;   // Debounce time (50ms)
const long holdTime = 700;       // Time before rapid increase (0.7s)
const long repeatRate = 500;     // Auto-increment every 500ms if held
unsigned long lastPressTime = 0;

// Colon blinking interval
unsigned long lastBlinkTime = 0;
const long blinkInterval = 1000; // Blink every 1 second

void setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1);

    pinMode(BTN_HOUR, INPUT_PULLUP);  // Set as input with pull-up

    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set to compile time
    }

    display.setBrightness(7);
}

void loop() {
    unsigned long currentMillis = millis();

    handleButtonPress();  // Call function to handle button logic

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        showTemperature = true;
    }

    if (showTemperature) {
        float tempC = rtc.getTemperature();
        int tempDisplay = (int)tempC * 100;
        display.showNumberDec(tempDisplay, false);
        Serial.printf("Temperature: %.2f°C\n", tempC);

        delay(displayDuration);
        showTemperature = false;
    }

    if (currentMillis - lastBlinkTime >= blinkInterval) {  
        lastBlinkTime = currentMillis;
        colonState = !colonState;  // Toggle colon every 1 second
        updateDisplay();  // Update only the colon
    }
}

void handleButtonPress() {
    if (digitalRead(BTN_HOUR) == LOW) {  
        if (millis() - lastPressTime > debounceDelay) {
            adjustTime();  // First increment immediately
            lastPressTime = millis();
        }

        // Keep incrementing at a slower rate when holding the button
        while (digitalRead(BTN_HOUR) == LOW) {
            if (millis() - lastPressTime > holdTime) {  
                adjustTime();
                lastPressTime = millis() + repeatRate;  // Slower auto-increment (500ms)
            }
        }
    }
}

void adjustTime() {
    DateTime now = rtc.now();
    int hour = now.hour();

    hour = (hour + 1) % 24;  // Increment hour, wrap at 24

    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, now.minute(), now.second()));
    Serial.printf("Adjusted Time: %02d:%02d\n", hour, now.minute());

    updateDisplay();  // Update display immediately after changing time
}

void updateDisplay() {
    DateTime now = rtc.now();
    int hour = now.hour();
    int minute = now.minute();

    int displayTime = (hour * 100) + minute;

    // Only toggle the colon without clearing the display
    display.showNumberDecEx(displayTime, colonState ? 0b01000000 : 0b00000000, false);
}
