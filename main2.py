import cv2
import time
import serial
from ultralytics import YOLO

# 모델과 카메라 설정
model = YOLO('yolo12s.pt')
cap = cv2.VideoCapture(0)

# 아두이노 시리얼 설정 — 실제 포트로 바꿔주세요 (예: 'COM3')
SERIAL_PORT = 'COM3'
BAUDRATE = 9600

try:
    ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
    time.sleep(2)  # 아두이노 리셋/초기화 대기
    print(f"[INFO] Opened serial {SERIAL_PORT} @ {BAUDRATE}")
except Exception as e:
    ser = None
    print(f"[WARN] Could not open serial port {SERIAL_PORT}: {e}")

target_classes = ["giraffe"]
while True:
  if not cap.isOpened():
    break
  ret, frame = cap.read()
  if not ret:
    break

  result = model(frame)
  detected_class_names = []
  if result[0].boxes is not None:
    for box in result[0].boxes:
        cls_id = int(box.cls[0])
        cls_name = model.names[cls_id]
        detected_class_names.append(cls_name)

  # 특정 물체가 있는지 확인
  found = any(cls in detected_class_names for cls in target_classes)

  if found:
    print("특정 객체가 감지되었습니다:", detected_class_names)
    # 여기에 아두이노로 신호를 전송합니다. 예: 서보 90도 명령 'S90\n'
    if ser:
      try:
        ser.write(b'S90\n')
        ser.flush()
        # 선택적으로 아두이노의 응답을 읽을 수 있습니다
        # resp = ser.readline().decode('utf-8', errors='ignore').strip()
        # print('[ARDUINO]', resp)
      except Exception as e:
        print('[ERROR] Failed to write to serial:', e)

  cv2.imshow('result', result[0].plot())

  if cv2.waitKey(1) & 0xFF == ord('q'):
    break

cap.release()
cv2.destroyAllWindows()
if ser:
  try:
    ser.close()
  except Exception:
    pass
