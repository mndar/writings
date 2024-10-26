from ultralytics import YOLO
import numpy as np

class YoloInference:
    def __init__(self):
        print ("Initializing Yolo Model")
        self.model = YOLO("yolov8n.pt")
    
    def infer(self, buf_bytearray, extra_data):
        (stream_type, model, width, height) = extra_data
        print (f"height: {height} width: {width}")
        image = np.frombuffer(buf_bytearray.read(height*width*3), dtype=np.uint8).reshape(height,width,3)
        
        names = self.model.names

        results = self.model.predict(image)
        result = results[0]

        print(f"Objects Detected: {len(result.boxes)}")
        
        boxes = []
        for box in result.boxes:
            name = names[box.cls[0].item()]
            x = int(box.xyxy[0][0].item())
            y = int(box.xyxy[0][1].item())
            w = int(box.xyxy[0][2].item()) - x
            h = int(box.xyxy[0][3].item()) - y
            conf = box.conf[0].item()
            box_info = {'name': name, 'conf': conf, 'xywh': {'x':x, 'y': y, 'w': w, 'h': h} };
            boxes.append(box_info)
        reply = {'detections': len(result.boxes), 'boxes': boxes}
        return reply