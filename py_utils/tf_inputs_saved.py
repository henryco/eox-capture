import tensorflow as tf

print("\n")

pb_file_path = '/home/henryco/Projects/blazepose/ready/saved_model_heavy/'

# Load the model
model = tf.saved_model.load(pb_file_path)

# List input information
for input in model.signatures['serving_default'].inputs:
    print("--> " + input.name, input.shape, input.dtype)