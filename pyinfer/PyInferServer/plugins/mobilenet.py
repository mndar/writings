import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras.applications import imagenet_utils
from tensorflow.keras.utils import img_to_array
import numpy as np
from PIL import Image

class MobilenetInference:
    def __init__(self):
        print ("Initializing Mobilenet")
        self.mobile = keras.applications.mobilenet.MobileNet()

    def infer (self,buf_bytearray, extra_data):
        (stream_type, model, width, height) = extra_data

        image = self.read_image(buf_bytearray, width, height)
        kimage = self.prepare_mobilenet_image (image)
        predictions = self.mobile.predict(kimage)
        results = imagenet_utils.decode_predictions(predictions)
        #print(results)

        recogs = []
        for result in results[0]:
            (ident, cls, conf) = result
            recog = {'class':cls, 'conf':float(conf)}
            recogs.append(recog)

        reply = {'recognitions':recogs}
        return reply
    
    def prepare_mobilenet_image(self, img_bytes):
        #img = img_bytes.convert('RGB')
        img = img_bytes.resize((224,224), Image.NEAREST)
        img_array = img_to_array(img)

        img_array_expanded_dims = np.expand_dims(img_array, axis=0)
        return tf.keras.applications.mobilenet.preprocess_input(img_array_expanded_dims)
        
    def read_image(self, buf_bytearray, width, height):
        nparray = np.frombuffer(buf_bytearray.read(), dtype=np.uint8).reshape(height,width,3)
        image = Image.fromarray(nparray)
        return image