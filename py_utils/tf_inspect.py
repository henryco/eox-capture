import tensorflow as tf

print("\n")

pb_file_path = '/home/henryco/Projects/blazepose/ready/saved_model_heavy/model_float32.pb'

# Load the protobuf file from the disk and parse it to retrieve the unserialized graph_def
with tf.io.gfile.GFile(pb_file_path, "rb") as f:
    graph_def = tf.compat.v1.GraphDef()
    graph_def.ParseFromString(f.read())

# Import the graph_def into a new Graph and return it
with tf.compat.v1.Session() as sess:
    # The name var will prefix every op/nodes in your graph
    # Since we don't know this information, we leave it empty
    tf.import_graph_def(graph_def, name='')

    # Print the operations in the graph
    for op in tf.compat.v1.get_default_graph().get_operations():
        print("--> " + op.name)
