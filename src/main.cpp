#include <Arduino.h>
#include <ESP32Servo.h>
#include <Motor.h>
#include <PS4Controller.h>

// TODO 変数名を修正する
/* JOYSTICK */
const int DEAD_ZONE = 30;

/* MOTOR */
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
int takeServo_degree = 20;

uint8_t right_pwm = 0;
uint8_t left_pwm = 0;

const int DEBOUNCE_DELAY = 50;
/*丸ボタン*/
bool circle_pressed = false;
uint32_t circle_debounce_time = 0;
/*三角ボタン*/
uint32_t triangle_debounce_time = 0;
/*バツボタン*/
uint32_t cross_debounce_time = 0;

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
  TakeServo.write(takeServo_degree);
}

void loop() {

  if (!PS4.isConnected()) {
    Serial.printf("PS4 controller disconnected.\n");
    RightMotor.run(0, 0);
    LeftMotor.run(0, 0);
    WindingMotor.run(0, 0);
    return;
  }

  if (DEAD_ZONE <= abs(PS4.RStickY())) {
    right_pwm = abs(PS4.RStickY());
    if (PS4.R1()) {
      right_pwm /= 2; // R1を押しているときPWMが半分になる
    }
    RightMotor.run(right_pwm,
                   (PS4.RStickY() > 0 ? 1 : 0)); // 右モーターを動かす
  } else {
    RightMotor.run(0, 0);
  }
  if (DEAD_ZONE <= abs(PS4.LStickY())) {
    left_pwm = abs(PS4.LStickY());
    if (PS4.R1()) {
      left_pwm /= 2;
    }
    LeftMotor.run(left_pwm,
                  (PS4.LStickY() > 0 ? 0 : 1)); // 左モーターを動かす
  } else {
    LeftMotor.run(0, 0);
  }

  /* MOVE SERVO */
  if (PS4.Triangle() && servo1_degree < 175) {
    if (millis() - triangle_debounce_time > DEBOUNCE_DELAY) {
      servo1_degree += 5;
      servo2_degree -= 5;
      ContinuousServo1.write(servo1_degree); // 上げる
      ContinuousServo2.write(servo2_degree);
    }
    triangle_debounce_time = millis();
  } else if (PS4.Cross() && servo1_degree > 5) {
    if (millis() - cross_debounce_time > DEBOUNCE_DELAY) {
      servo1_degree -= 5;
      servo2_degree += 5;
      ContinuousServo1.write(servo1_degree); // 下げる
      ContinuousServo2.write(servo2_degree);
    }
    cross_debounce_time = millis();
  }

  if (PS4.R2Value() > 10) {
    WindingMotor.run(PS4.R2Value() / 2, 0); // 正転
  } else if (PS4.L2Value() > 10) {
    WindingMotor.run(PS4.L2Value() / 2, 1); // 逆転
  } else {
    WindingMotor.run(0, 0);
  }

  if (PS4.Right()) {
    TakeServo.write(0);
  }
  if (PS4.Left()) {
    TakeServo.write(60);
  }

  if (PS4.PSButton()) {
    ESP.restart(); // ESP32の再起動
  }
}