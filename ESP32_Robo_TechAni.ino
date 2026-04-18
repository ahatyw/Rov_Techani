#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>

// ⚠️ EDIT YOUR HOTSPOT DETAILS HERE ⚠️
const char* ssid = "M08ESP";
const char* password = "12345678";

unsigned int localUdpPort = 4210;
WiFiUDP Udp;
char incomingPacket[255]; 

// L298N Motor Pins
const int ENA = 14, IN1 = 27, IN2 = 26;
const int IN3 = 25, IN4 = 33, ENB = 32;

// Servo Pins
const int basePin = 5, shoulderPin = 18, elbowPin = 19, clawPin = 21;
Servo baseServo, shoulderServo, elbowServo, clawServo;

// Sensor & Buzzer Pins
const int TRIG_PIN = 23;
const int ECHO_FRONT = 34, ECHO_BACK = 35, ECHO_LEFT = 36, ECHO_RIGHT = 39;
const int BUZZER = 22;

bool autoMode = false;

void setup() {
  Serial.begin(115200);

  // Motor Init
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);
  
  // Sensor Init
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_FRONT, INPUT); pinMode(ECHO_BACK, INPUT);
  pinMode(ECHO_LEFT, INPUT); pinMode(ECHO_RIGHT, INPUT);
  pinMode(BUZZER, OUTPUT);

  // Servo Init (ESP32 specific timers)
  ESP32PWM::allocateTimer(0); ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2); ESP32PWM::allocateTimer(3);
  baseServo.setPeriodHertz(50); shoulderServo.setPeriodHertz(50);
  elbowServo.setPeriodHertz(50); clawServo.setPeriodHertz(50);
  baseServo.attach(basePin, 500, 2400); shoulderServo.attach(shoulderPin, 500, 2400);
  elbowServo.attach(elbowPin, 500, 2400); clawServo.attach(clawPin, 500, 2400);
  
  // Initial Arm Position
  baseServo.write(90); shoulderServo.write(90); elbowServo.write(90); clawServo.write(10);

  // Network Init
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Udp.begin(localUdpPort);
  Serial.println("\n--- TECHANI MAIN BRAIN ONLINE ---");
  Serial.print("MAIN IP: ");
  Serial.println(WiFi.localIP());
}

// Read Distance (Returns cm, timeouts at 100cm to prevent freezing)
int getDist(int echoPin) {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long dur = pulseIn(echoPin, HIGH, 6000); 
  if (dur == 0) return 100; 
  return dur * 0.034 / 2;
}

// Drive Motor function
void drive(int leftSpeed, int rightSpeed) {
  if(leftSpeed >= 0) { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); }
  else { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); leftSpeed = -leftSpeed; }
  
  if(rightSpeed >= 0) { digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
  else { digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); rightSpeed = -rightSpeed; }

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

void loop() {
  // 1. Network Parsing
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(incomingPacket, 255);
    if (len > 0) incomingPacket[len] = 0;
    String cmd = String(incomingPacket);

    if (cmd.startsWith("MOTOR") && !autoMode) {
      int c1 = cmd.indexOf(','); int c2 = cmd.lastIndexOf(',');
      int lSpeed = cmd.substring(c1 + 1, c2).toInt();
      int rSpeed = cmd.substring(c2 + 1).toInt();
      drive(lSpeed, rSpeed);
    } 
    else if (cmd.startsWith("ARM") && !autoMode) {
      int c1 = cmd.indexOf(','); int c2 = cmd.indexOf(',', c1+1);
      int c3 = cmd.indexOf(',', c2+1); int c4 = cmd.lastIndexOf(',');
      baseServo.write(cmd.substring(c1+1, c2).toInt());
      shoulderServo.write(cmd.substring(c2+1, c3).toInt());
      elbowServo.write(cmd.substring(c3+1, c4).toInt());
      clawServo.write(cmd.substring(c4+1).toInt());
    }
    else if (cmd.startsWith("MODE")) {
      autoMode = (cmd.charAt(5) == 'A');
      if(autoMode) drive(0,0); // Immediate stop for safety
    }
  }

  // 2. Sensor Reads
  int dF = getDist(ECHO_FRONT); int dB = getDist(ECHO_BACK);
  int dL = getDist(ECHO_LEFT);  int dR = getDist(ECHO_RIGHT);

  // 3. Proximity Safety Alarm
  if (dF < 7 || dB < 7) digitalWrite(BUZZER, HIGH);
  else digitalWrite(BUZZER, LOW);

  // 4. Auto Avoidance Logic
  if (autoMode) {
    if (dF > 10) {
      drive(150, 150); // Safe to move forward
    } else {
      drive(0, 0); delay(300); // Stop, evaluate
      if (dL > dR) { drive(-150, 150); delay(400); } // Obstacle right, turn left
      else { drive(150, -150); delay(400); } // Obstacle left, turn right
    }
  }
}