import tensorflow as tf

print("\n")

pb_file_path = '/home/henryco/Projects/blazepose/ready/saved_model_heavy/model_float32.pb'

# Load the frozen graph
graph = tf.Graph()
with graph.as_default():
    graph_def = tf.compat.v1.GraphDef()
    with tf.io.gfile.GFile(pb_file_path, 'rb') as f:
        graph_def.ParseFromString(f.read())
        tf.import_graph_def(graph_def, name='')

# Start a session
with tf.compat.v1.Session(graph=graph) as sess:
    # Print the input nodes
    for op in graph.get_operations():
        if op.type == 'Placeholder':
            print("--> Itg: " + op.name, op.outputs[0].shape, op.outputs[0].dtype)

    # Print the output nodes
    ops = graph.get_operations()
    output_ops = [op for op in ops if op.outputs]
    for op in output_ops:
        print("--> O: " + op.name, op.outputs[0].shape, op.outputs[0].dtype)