#include <TM1637Display.h>

#define CLK D5
#define DIO D6

TM1637Display display(CLK, DIO);

void setup() {
  display.setBrightness(7);  // Brightness: 0 (dim) to 7 (bright)
}

void loop() {
  for (int i = 0; i <= 9999; i++) {
    // Turn ON the colon and display the number
    display.showNumberDecEx(i, 0b01000000, true);
    delay(250);

    // Turn OFF the colon while keeping the number
    display.showNumberDecEx(i, 0b00000000, true);
    delay(250);
  }
}
