proto : *.proto
	mkdir -p compiled_cpp
	mkdir -p compiled_python
	protoc -I=. --cpp_out=compiled_cpp --python_out=compiled_python *.proto
clean :
	rm -r compiled_cpp compiled_python