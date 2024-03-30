## Protobuffer Templates

Templates for Google Protobuffers.
See "How do I start?" at https://developers.google.com/protocol-buffers

For Python in particular:
https://developers.google.com/protocol-buffers/docs/pythontutorial

For C++:
https://developers.google.com/protocol-buffers/docs/cpptutorial

Arrays are encoded by repetition, e.g.
https://stackoverflow.com/questions/7408203/how-to-add-a-int-array-in-protobuf-message

### Compiling these protobuffers, example
Makefile for dolphindaq Google Protocol Buffer templates

Requires protoc, protoc-c, and nanopb

The nanopb portion utilizes the folders above this directory.

On Ubuntu 20.x (or Windows Subsystem for Linux), get the packages:
      sudo apt install protobuf-compiler # CPP; I was using 3.6.1
      sudo apt install python3 python3-protobuf # Python
      sudo apt install nanopb # NanoPB C; I was using 0.4.1

And then, in this folder, run:
      make
      
### Using protobuffers example (for Python)
pip install protobuf

trace = diode_pb2.Trace()
trace.shot_num = 5
trace.trace[:] = np.arange(10)