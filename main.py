import cv2
from ultralytics import YOLO

model = YOLO('yolo12s.pt')
cap = cv2.VideoCapture(0)

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
  cv2.imshow('result', result[0].plot())

  if cv2.waitKey(1) & 0xFF == ord('q'):
    break

cap.release()
cv2.destroyAllWindows() 
