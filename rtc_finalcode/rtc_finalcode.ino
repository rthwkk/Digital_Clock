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
const long interval = 30000;
const long displayDuration = 2000;
bool showTemperature = false;

// Button pins
#define BTN_HOUR D7  // GPIO0
#define BTN_MINUTE D6 // GPIO2

// Debounce settings
const long debounceDelay = 50;
const long holdTime = 700;  // Hold time before auto-increment
const long repeatRate = 500; // Auto-increment every 500ms when held

// Button state tracking
bool lastHourState = HIGH;
bool lastMinuteState = HIGH;
unsigned long lastHourPressTime = 0;
unsigned long lastMinutePressTime = 0;
bool hourHeld = false;
bool minuteHeld = false;

// Colon blinking interval
unsigned long lastBlinkTime = 0;
const long blinkInterval = 1000; // 1 second blink rate

void setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1);

    pinMode(BTN_HOUR, INPUT_PULLUP);
    pinMode(BTN_MINUTE, INPUT_PULLUP);

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

    handleButtonPress(BTN_HOUR, lastHourPressTime, lastHourState, hourHeld, true);
    handleButtonPress(BTN_MINUTE, lastMinutePressTime, lastMinuteState, minuteHeld, false);

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
        colonState = !colonState;
        updateDisplay();
    }
}

void handleButtonPress(int buttonPin, unsigned long &lastPressTime, bool &lastButtonState, bool &buttonHeld, bool isHour) {
    bool currentButtonState = digitalRead(buttonPin);

    if (currentButtonState == LOW && lastButtonState == HIGH) {  // Button just pressed
        delay(debounceDelay); // Debounce delay
        if (digitalRead(buttonPin) == LOW) { // Confirm it's still pressed
            adjustTime(isHour);
            lastPressTime = millis();
            buttonHeld = true;
        }
    } 
    else if (currentButtonState == LOW && buttonHeld) {  // Button is held
        if (millis() - lastPressTime > holdTime) {  // Start auto-increment after hold
            adjustTime(isHour);
            lastPressTime = millis() + repeatRate;
        }
    } 
    else if (currentButtonState == HIGH) {  // Button released
        buttonHeld = false;
    }

    lastButtonState = currentButtonState;
}

void adjustTime(bool isHour) {
    DateTime now = rtc.now();
    int hour = now.hour();
    int minute = now.minute();

    if (isHour) {
        hour = (hour + 1) % 24;
    } else {
        minute = (minute + 1) % 60;
    }

    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, 0));
    Serial.printf("Adjusted Time: %02d:%02d\n", hour, minute);

    updateDisplay();
}

void updateDisplay() {
    DateTime now = rtc.now();
    int hour = now.hour();
    int minute = now.minute();

    int displayTime = (hour * 100) + minute;
    display.showNumberDecEx(displayTime, colonState ? 0b01000000 : 0b00000000, true);
}
