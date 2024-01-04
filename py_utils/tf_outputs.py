import tensorflow as tf

def print_output_details(model_path):
    # Load the TFLite model
    interpreter = tf.lite.Interpreter(model_path=model_path)
    interpreter.allocate_tensors()

    # Get and print output details
    output_details = interpreter.get_output_details()
    for output in output_details:
        print("Index:", output['index'], "Name:", output['name'])

if __name__ == "__main__":
    model_path = '/home/henryco/Projects/blazepose/ready/saved_model_heavy/model_float32.tflite'
    print_output_details(model_path)
