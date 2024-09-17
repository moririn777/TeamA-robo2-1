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
bool is_auto_mode = false;

/*pwm dir channel*/
Motor RightMotor(26, 21, 5);
Motor LeftMotor(27, 22, 6);
Motor WindingMotor(13, 23, 7);

/* SERVO */
Servo ContinuousServo1;
Servo ContinuousServo2;
Servo TakeServo;

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


void setup() {
  Serial.begin(115200);

  /*uint8_t bt_mac[6];
  esp_read_mac(bt_mac, ESP_MAC_BT);
  Serial.printf("Bluetooth Mac Address => %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                bt_mac[0], bt_mac[1], bt_mac[2], bt_mac[3], bt_mac[4],
                bt_mac[5]);*/

  PS4.begin("08:D1:F9:37:36:FE");
  Serial.printf("ready.\n");

  ContinuousServo1.attach(SERVO1_PIN, 800, 2200);
  ContinuousServo2.attach(SERVO2_PIN, 800, 2200);
  TakeServo.attach(SERVO3_PIN, 500, 2400);

  ContinuousServo1.write(servo1_degree);
  ContinuousServo2.write(servo2_degree);
  TakeServo.write(20);

  pinMode(SW_CENTER_PIN, INPUT_PULLDOWN);
  pinMode(SW_SIDE1_PIN, INPUT_PULLDOWN);
  pinMode(SW_SIDE2_PIN, INPUT_PULLDOWN);
}

void loop() {

  if (!PS4.isConnected()) {
    Serial.printf("PS4 controller disconnected.\n");
    RightMotor.run(0,0);
    LeftMotor.run(0,0);
    WindingMotor.run(0,0);
    return;
  }

  /* SWITCH BETWEEN MANUAL AND AUTOMATIC */
  if (PS4.Circle()) {
    is_auto_mode = !is_auto_mode;
    Serial.printf("State : %d\n ", is_auto_mode);
    delay(500);
  }

  /* AUTOMATICALLY MOVE FORWARD */
  if (is_auto_mode) {
    runForwardOrBackward(AUTOMATIC_SPEED); //まっすぐ進む
    int sw1 = digitalRead(SW_CENTER_PIN);
    int sw2 = digitalRead(SW_SIDE1_PIN);
    int sw3 = digitalRead(SW_SIDE2_PIN);

    Serial.printf("SWITCH1 STATE : %d \r\n ", sw1);
    Serial.printf("SWITCH2 STATE : %d \r\n ", sw2);
    Serial.printf("SWITCH3 STATE : %d \r\n ", sw3);

    if (sw1 == HIGH || sw2 == HIGH || sw3 == HIGH) {
      Serial.printf("LIMIT SWITCH PRESSED.\n");
      is_auto_mode = false;
      RightMotor.run(0,0);
      LeftMotor.run(0,0);
      delay(250);
    }
  } else {
    if (DEAD_ZONE <= abs(PS4.RStickY())) {
      RightMotor.run(abs(PS4.RStickY()),
                     (PS4.RStickY() > 0 ? 1 : 0)); // 右モーターを動かす
    }else{
      RightMotor.run(0,0);
    }
    if (DEAD_ZONE <= abs(PS4.LStickY())) {
      LeftMotor.run(abs(PS4.LStickY()),
                    (PS4.LStickY() > 0 ? 0 : 1)); // 左モーターを動かす
    }else{
      LeftMotor.run(0,0);
    }
  }

  /* MOVE SERVO */
  if (PS4.Triangle() && servo1_degree < 170) {
    servo1_degree += 5;
    servo2_degree -= 5;
    ContinuousServo1.write(servo1_degree); // 上げる
    ContinuousServo2.write(servo2_degree);
    delay(25);
  } else if (PS4.Cross() && servo1_degree > 5) {
    servo1_degree -= 5;
    servo2_degree += 5;
    ContinuousServo1.write(servo1_degree); // 下げる
    ContinuousServo2.write(servo2_degree);
    delay(25);
  }

  if (PS4.R2Value() > 10) {
    WindingMotor.run(PS4.R2Value() / 2, 0); // 正転
  } else if (PS4.L2Value() > 10) {
    WindingMotor.run(PS4.L2Value() / 2, 1); // 逆転
  } else {
    WindingMotor.run(0, 0);
  }
  //TODO 要調整!!
  if (PS4.Right()) {
    TakeServo.write(60);
    delay(25);
  }
  if (PS4.Left()) {
    TakeServo.write(90);
    delay(25);
  }

  if (PS4.PSButton()) {
    ESP.restart(); // ESP32の再起動
  }
}

/* MOVE FORWARD AND BACKWARD */
void runForwardOrBackward(int speed) {
  if (speed > 0) {
    RightMotor.run(speed, 1);
    LeftMotor.run(speed, 0);
    Serial.printf("RUN FORWARD\n");
  } else {
    RightMotor.run(speed, 0);
    LeftMotor.run(speed, 1);
    Serial.printf("RUN BACKWARD\n");
  }
}

