#include <Arduino.h>
#include <ESP32Servo.h>
#include <Motor.h>
#include <PS4Controller.h>

// TODO 変数名を修正する
/* JOYSTICK */
const int DEAD_ZONE = 30;

/* MOTOR */
const int AUTOMATIC_SPEED = 127;

/*MODE*/
bool isAutoMode = false;

/*pwm dir channel*/
Motor rightMotor(26, 21, 5);
Motor leftMotor(27, 22, 6);
Motor windingMotor(13, 23, 7);

/* SERVO */
Servo continuousServo1;
Servo continuousServo2;
Servo takeServo;

const int SERVO1_PIN = 18;
const int SERVO2_PIN = 4;
const int SERVO3_PIN = 19;

int servo1_degree = 180; // 初期位置
int servo2_degree = 0;

int takeServo_degree;

/* LIMIT SWITCH */
const int SW_CENTER_PIN = 33;
const int SW_SIDE1_PIN = 32;
const int SW_SIDE2_PIN = 25;

/* MOTOR FUNCTION */
void runForwardOrBackward(int speed);
void stopMotor();

void setup() {
  Serial.begin(115200);

  /*uint8_t bt_mac[6];
  esp_read_mac(bt_mac, ESP_MAC_BT);
  Serial.printf("Bluetooth Mac Address => %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                bt_mac[0], bt_mac[1], bt_mac[2], bt_mac[3], bt_mac[4],
                bt_mac[5]);*/

  PS4.begin("08:D1:F9:37:36:FE");
  Serial.printf("ready.\n");

  continuousServo1.attach(SERVO1_PIN, 800, 2200);
  continuousServo2.attach(SERVO2_PIN, 800, 2200);
  takeServo.attach(SERVO3_PIN, 500, 2400);

  continuousServo1.write(servo1_degree);
  continuousServo2.write(servo2_degree);
  takeServo.write(20);

  pinMode(SW_CENTER_PIN, INPUT_PULLDOWN);
  pinMode(SW_SIDE1_PIN, INPUT_PULLDOWN);
  pinMode(SW_SIDE2_PIN, INPUT_PULLDOWN);
}

void loop() {

  if (!PS4.isConnected()) {
    Serial.printf("PS4 controller disconnected.\n");
    stopMotor();
    return;
  }

  /* SWITCH BETWEEN MANUAL AND AUTOMATIC */
  if (PS4.Circle()) {
    isAutoMode = !isAutoMode;
    Serial.printf("State : %d\n ", isAutoMode);
    delay(500);
  }

  /* AUTOMATICALLY MOVE FORWARD */
  if (isAutoMode) {
    runForwardOrBackward(AUTOMATIC_SPEED); //まっすぐ進む
    int sw1 = digitalRead(SW_CENTER_PIN);
    int sw2 = digitalRead(SW_SIDE1_PIN);
    int sw3 = digitalRead(SW_SIDE2_PIN);

    Serial.printf("SWITCH1 STATE : %d \r\n ", sw1);
    Serial.printf("SWITCH2 STATE : %d \r\n ", sw2);
    Serial.printf("SWITCH3 STATE : %d \r\n ", sw3);

    if (sw1 == HIGH || sw2 == HIGH || sw3 == HIGH) {
      Serial.printf("LIMIT SWITCH PRESSED.\n");
      isAutoMode = false;
      stopMotor();
      delay(250);
    }
  } else {
    if (DEAD_ZONE <= abs(PS4.RStickY())) {
      rightMotor.run(abs(PS4.RStickY()),
                     (PS4.RStickY() > 0 ? 1 : 0)); // 右モーターを動かす
    }
    if (DEAD_ZONE <= abs(PS4.LStickY())) {
      leftMotor.run(abs(PS4.LStickY()),
                    (PS4.LStickY() > 0 ? 0 : 1)); // 左モーターを動かす
    }
    if (DEAD_ZONE > abs(PS4.LStickY()) && DEAD_ZONE > abs(PS4.RStickY())) {
      stopMotor();
    }
  }

  /* MOVE SERVO */
  if (PS4.Triangle() && servo1_degree < 170) {
    servo1_degree += 5;
    servo2_degree -= 5;
    continuousServo1.write(servo1_degree); // 上げる
    continuousServo2.write(servo2_degree);
    delay(25);
  } else if (PS4.Cross() && servo1_degree > 5) {
    servo1_degree -= 5;
    servo2_degree += 5;
    continuousServo1.write(servo1_degree); // 下げる
    continuousServo2.write(servo2_degree);
    delay(25);
  }

  if (PS4.R2Value() > 10) {
    windingMotor.run(PS4.R2Value() / 2, 0); // 正転
  } else if (PS4.L2Value() > 10) {
    windingMotor.run(PS4.L2Value() / 2, 1); // 逆転
  } else {
    windingMotor.run(0, 0);
  }
  //TODO 要調整!!
  if (PS4.Right()) {
    takeServo.write(60);
    delay(25);
  }
  if (PS4.Left()) {
    takeServo.write(90);
    delay(25);
  }

  if (PS4.PSButton()) {
    ESP.restart(); // ESP32の再起動
  }
}

/* MOVE FORWARD AND BACKWARD */
void runForwardOrBackward(int speed) {
  if (speed > 0) {
    rightMotor.run(speed, 1);
    leftMotor.run(speed, 0);
    Serial.printf("RUN FORWARD\n");
  } else {
    rightMotor.run(speed, 0);
    leftMotor.run(speed, 1);
    Serial.printf("RUN BACKWARD\n");
  }
}

/* STOP MOVING */
void stopMotor() {
  rightMotor.run(0, 0);
  leftMotor.run(0, 0);
}
