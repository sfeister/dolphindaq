## Protobuffer Templates

Templates for Google Protobuffers.
See "How do I start?" at https://developers.google.com/protocol-buffers

For Python in particular:
https://developers.google.com/protocol-buffers/docs/pythontutorial

For C++:
https://developers.google.com/protocol-buffers/docs/cpptutorial

Arrays are encoded by repetition, e.g.
https://stackoverflow.com/questions/7408203/how-to-add-a-int-array-in-protobuf-message

### Compiling protobuffers example (for Python)
```bash
protoc -I=. --python_out=. ./diode.proto
```

### Using protobuffers example (for Python)
pip install protobuf

trace = diode_pb2.Trace()
trace.shot_num = 5
trace.trace[:] = np.arange(10)