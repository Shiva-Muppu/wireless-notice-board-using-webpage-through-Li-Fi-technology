#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi credentials
const char* ssid = "amar";
const char* password = "12345678";

ESP8266WebServer server(80);

// For ESP8266 boards like NodeMCU or Wemos D1 Mini, D1 typically maps to GPIO5.
// If D1 still gives an error after selecting the board, you can try using GPIO5 directly:
// const int ledPin = 5; 
const int ledPin = D2; // This line is correct once the ESP8266 board is selected.

// Pulse durations for Li-Fi (adjust these based on your receiver sensitivity)
const int durationZero = 200;   // Binary 0 = 200ms ON
const int durationOne  = 300;   // Binary 1 = 300ms ON
const int interBitDelay = 100;  // Delay between bits (LED OFF)
const int interCharDelay = 500; // Delay between characters (LED OFF)

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  pinMode(ledPin, OUTPUT); // Set the LED pin as an output
  digitalWrite(ledPin, LOW); // Ensure LED is off initially

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Print the assigned IP address

  // Define server routes
  server.on("/", handleRoot);     // Handle requests to the root URL "/"
  server.on("/send", handleSend); // Handle requests to "/send"
  server.begin(); // Start the web server
  Serial.println("Server started");
}

void loop() {
  server.handleClient(); // Handle incoming client requests
}

// Handler for the root URL: displays a simple HTML form
void handleRoot() {
  String html =
    "<html><body><h2>Li-Fi Transmitter</h2>"
    "<form action=\"/send\">"
    "Message: <input name=\"msg\" type=\"text\" placeholder=\"Enter your message\" />" // Added placeholder
    "<input type=\"submit\" value=\"Send\" />"
    "</form></body></html>";
  server.send(200, "text/html", html); // Send the HTML response
}

// Handler for the /send URL: processes the message from the form
void handleSend() {
  if (server.hasArg("msg")) { // Check if the "msg" argument exists
    String message = server.arg("msg"); // Get the message
    Serial.println("Sending message: " + message);
    sendData(message); // Call function to transmit the message via LED
    server.send(200, "text/html", "Message sent! <a href='/'>Back</a>"); // Confirmation message
  } else {
    server.send(400, "text/plain", "No message provided"); // Error if no message
  }
}

// Function to send data via LED using ON/OFF pulses (Li-Fi transmission)
void sendData(String msg) {
  // Iterate through each character in the message
  for (int i = 0; i < msg.length(); i++) {
    char c = msg.charAt(i); // Get the current character
    Serial.print("Char: ");
    Serial.print(c);
    Serial.print(" | ASCII: ");
    Serial.println((int)c); // Print ASCII value for debugging

    // Transmit each bit of the character (from MSB to LSB)
    for (int b = 7; b >= 0; b--) {
      bool bit = (c >> b) & 1; // Extract the bit

      digitalWrite(ledPin, HIGH); // Turn LED ON to signal a bit
      if (bit == 1) {
        delay(durationOne);  // LED ON for binary 1 (longer pulse)
      } else {
        delay(durationZero); // LED ON for binary 0 (shorter pulse)
      }
      digitalWrite(ledPin, LOW); // Turn LED OFF after pulse
      delay(interBitDelay);  // Short OFF delay between bits
    }

    delay(interCharDelay); // Longer OFF delay between characters
  }
  Serial.println("Message transmission complete."); // Indicate end of transmission
}
