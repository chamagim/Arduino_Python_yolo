#include <Servo.h>

Servo servo;
const int servoPin = 9;
int currentAngle = 0;
int targetAngle = 0;

// 부드럽게 이동하기 위한 설정 (필요시 조정)
const unsigned long moveIntervalMs = 20; // 업데이트 간격 (ms)
const int stepSize = 1; // 한 번에 이동할 각도
unsigned long lastMoveMillis = 0;
// 감지(아두이노가 "기린" 또는 S명령을 받은) 후 자동 복귀 타임아웃
const unsigned long DETECT_TIMEOUT_MS = 3000; // 밀리초 단위, 필요시 조정
unsigned long lastDetectMillis = 0; // 마지막으로 감지 신호를 받은 시각

void setup() {
  Serial.begin(9600);
  servo.attach(servoPin);
  servo.write(0);
}

String readLine() {
  String s = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') break;
    if (c != '\r') s += c;
  }
  return s;
}

void loop() {
  if (Serial.available()) {
    String line = readLine();
    line.trim();
    if (line.length() == 0) return;

    // 예: 'S90' -> 목표 각도 설정, 'G' -> 임의 동작
    if (line.charAt(0) == 'S') {
      int angle = line.substring(1).toInt();
      angle = constrain(angle, 0, 180);
      targetAngle = angle; // 즉시 쓰지 않고 목표로 설정
      lastDetectMillis = millis(); // S명령도 감지로 간주하여 타임아웃 리셋
      // 응답만 보내서 수신 확인
      Serial.print("OK ");
      Serial.println(targetAngle);
    } else if (line == "G") {
      // 기린 감지 신호 처리 예: 목표 각도 변경
      targetAngle = 90;
      lastDetectMillis = millis(); // G 수신 시에도 타임아웃 리셋
      Serial.println("G_RECEIVED");
    }
    else if (line == "PING") {
      Serial.println("PONG");
    }
  }

  // 현재 각도와 목표 각도 사이를 부드럽게 이동
  unsigned long now = millis();
  if (now - lastMoveMillis >= moveIntervalMs) {
    lastMoveMillis = now;
    if (currentAngle < targetAngle) {
      int diff = targetAngle - currentAngle;
      int step = min(stepSize, diff);
      currentAngle += step;
      servo.write(currentAngle);
    } else if (currentAngle > targetAngle) {
      int diff = currentAngle - targetAngle;
      int step = min(stepSize, diff);
      currentAngle -= step;
      servo.write(currentAngle);
    }
  }

  // 감지 타임아웃 검사: 마지막 감지 이후 일정 시간이 지나면 목표를 0도로 설정
  if (lastDetectMillis != 0) {
    unsigned long now2 = millis();
    if (now2 - lastDetectMillis > DETECT_TIMEOUT_MS) {
      targetAngle = 0;
      lastDetectMillis = 0; // 타임아웃 처리 후 리셋
    }
  }
}
