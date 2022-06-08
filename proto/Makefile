# Makefile for dolphindaq Google Protocol Buffer templates
# Requires protoc, protoc-c, and nanonb
# On Ubuntu 20.x:
#      sudo apt install protobuf-compiler # CPP
#      sudo apt install protobuf-c-compiler # C
#      sudo apt install python3 python3-protobuf # Python
#      sudo apt install nanopb # NanoPB C

all : python c cpp nanopb

compiled_% : 
	mkdir -p $@

compiled_c/%.pb-c.c: %.proto compiled_c
	protoc -I=. --python_out=compiled_c $<

compiled_c/%.pb-c.h : compiled_c/%.pb-c.c

python : *.proto compiled_python
	touch compiled_python/__init__.py
	protoc -I=. --python_out=compiled_python *.proto

#compiled_python/
c : *.proto compiled_c
	protoc -I=. --c_out=compiled_c *.proto

cpp : *.proto compiled_cpp
	protoc -I=. --cpp_out=compiled_cpp *.proto

nanopb : *.proto compiled_nanopb
	protoc -I=. --nanopb_out=compiled_nanopb *.proto

.PHONY: clean

clean :
	rm -r compiled_cpp compiled_python compiled_c compiled_nanopb