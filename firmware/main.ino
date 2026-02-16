/*
  Papaya Mini Rover Firmware

  This sketch configures the ESP32‑CAM to host a web server that streams
  video and accepts joystick commands for driving the rover.  Two L9110S
  driver boards control six N20 motors via PWM.  Adjust pin assignments
  and credentials as needed.
*/

#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials (set your own)
const char* ssid     = "PapayaRover";
const char* password = "papaya123";

WebServer server(80);

// Motor pins (left and right sets)
const int motorPins[6][2] = {
  {13, 12}, // left front motor A/B
  {15, 14}, // left middle
  {16, 17}, // left rear
  {2, 4},   // right front
  {5, 18},  // right middle
  {19, 21}  // right rear
};

void setup() {
  Serial.begin(115200);
  // Initialize motor pins
  for (int i = 0; i < 6; i++) {
    pinMode(motorPins[i][0], OUTPUT);
    pinMode(motorPins[i][1], OUTPUT);
  }
  // Start Wi‑Fi AP
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/drive", HTTP_POST, handleDrive);
  server.begin();
}

// Handle root page
void handleRoot() {
  server.send(200, "text/plain", "Papaya Rover ready. Send POST /drive with JSON body {\"l\":int, \"r\":int}.");
}

// Parse drive commands (simple JSON with left and right speed -255..255)
void handleDrive() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Body required");
    return;
  }
  String body = server.arg("plain");
  int comma = body.indexOf(',');
  int l = body.substring( body.indexOf('l') + 2, comma).toInt();
  int r = body.substring( body.lastIndexOf(':') + 1, body.indexOf('}')).toInt();
  drive(l, r);
  server.send(200, "text/plain", "OK");
}

// Drive all motors on one side
void setSide(int side, int speed) {
  speed = constrain(speed, -255, 255);
  for (int i = 0; i < 3; i++) {
    int a = motorPins[side*3 + i][0];
    int b = motorPins[side*3 + i][1];
    if (speed >= 0) {
      analogWrite(a, speed);
      analogWrite(b, 0);
    } else {
      analogWrite(a, 0);
      analogWrite(b, -speed);
    }
  }
}

void drive(int leftSpeed, int rightSpeed) {
  setSide(0, leftSpeed);
  setSide(1, rightSpeed);
}

void loop() {
  server.handleClient();
}
