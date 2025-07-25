#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LDR_PIN A2
const int threshold = 800; // Calibrate this value for your specific LDR and light conditions

unsigned long ledStartTime = 0;
unsigned long ledDuration = 0;
unsigned long lastBitReceivedTime = 0; // Renamed for clarity: tracks the last time a bit was successfully added

String bitBuffer = "";
String message = "";

const unsigned long messageGap = 1000000; // 1 second gap (in microseconds) to signify end of message

// Initialize LCD (Address 0x27 is common, change if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  pinMode(LDR_PIN, INPUT);

  // LCD setup
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Receiver Ready");

  Serial.println("Receiver ready...");
}

void loop() {
  int ldrValue = analogRead(LDR_PIN);
  unsigned long currentTime = micros();

  // Detect light ON (LED ON) - LDR value drops
  if (ldrValue < threshold) {
    if (ledStartTime == 0) { // If LED was previously OFF, record start time
      ledStartTime = currentTime;
    }
  }
  // Light OFF (LED OFF) - LDR value rises
  else {
    if (ledStartTime != 0) { // If LED was previously ON, calculate duration
      ledDuration = currentTime - ledStartTime;
      ledStartTime = 0; // Reset for next pulse

      // Only accept valid pulses within a defined range
      // Adjust these values based on your transmitter's pulse durations
      if (ledDuration > 100000 && ledDuration < 500000) { // Example range: 0.1ms to 0.5ms
        char bit = (ledDuration > 250000) ? '1' : '0'; // If duration > 0.25ms, it's a '1', else '0'
        bitBuffer += bit;
        lastBitReceivedTime = currentTime; // Update time of last received bit
      }
    }
  }

  // Convert 8 bits to a character
  if (bitBuffer.length() == 8) {
    // Convert binary string (e.g., "01000001") to its ASCII character ('A')
    char c = 0;
    for (int i = 0; i < 8; i++) {
      if (bitBuffer.charAt(i) == '1') {
        c = (c << 1) | 1; // Shift left and set the last bit to 1
      } else {
        c = (c << 1) | 0; // Shift left and set the last bit to 0
      }
    }
    message += c;
    bitBuffer = ""; // Clear buffer for the next character
  }

  // After a significant gap, display the received message
  if (message.length() > 0 && (currentTime - lastBitReceivedTime > messageGap)) {
    Serial.print("Message Received: ");
    Serial.println(message);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Received Msg:");
    lcd.setCursor(0, 1);
    lcd.print(message);

    message = ""; // Clear for the next incoming message
  }
}
