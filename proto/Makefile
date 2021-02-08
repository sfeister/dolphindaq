proto : *.proto
	mkdir -p compiled_cpp
	mkdir -p compiled_c
	mkdir -p compiled_python
	touch compiled_python/__init__.py
	protoc -I=. --cpp_out=compiled_cpp --python_out=compiled_python *.proto
	protoc-c -I=. --c_out=compiled_c *.proto

clean :
	rm -r compiled_cpp compiled_python compiled_c