# Makefile for dolphindaq Google Protocol Buffer templates
# Requires protoc, protoc-c, and nanopb
# The nanopb portion utilizes the folders above this directory.
# On Ubuntu 20.x:
#      sudo apt install protobuf-compiler # CPP
#      sudo apt install python3 python3-protobuf # Python
#      sudo apt install nanopb # NanoPB C

all : python cpp nanopb arduino

compiled_% : 
	mkdir -p $@
	
python : *.proto compiled_python
	touch compiled_python/__init__.py
	protoc -I=. --python_out=compiled_python *.proto

cpp : *.proto compiled_cpp
	protoc -I=. --cpp_out=compiled_cpp *.proto

arduino : *.proto nanopb # Folder suitable to copy into Arduino's library folder
	mkdir -p compiled_arduino/ddaqproto
	touch compiled_arduino/ddaqproto/ddaqproto.h # a stub
	cp -r compiled_nanopb/Inc/* compiled_arduino/ddaqproto/
	cp -r compiled_nanopb/Src/* compiled_arduino/ddaqproto/
	cp -r compiled_nanopb/Src/google/protobuf/*.c compiled_arduino/ddaqproto/
	cp -r compiled_nanopb/Inc/google/protobuf/*.h compiled_arduino/ddaqproto/

nanopb : *.proto nanopb_src nanopb_wkt
	mkdir -p compiled_nanopb/Inc
	mkdir -p compiled_nanopb/Src
	protoc -I=. -I=../protobuf-21_2/src/google/protobuf --nanopb_out=compiled_nanopb *.proto
	mv compiled_nanopb/*.h compiled_nanopb/Inc/
	mv compiled_nanopb/*.c compiled_nanopb/Src/

nanopb_src : # Copies over certain bits of nanopb source code and headers (those which need to be included on your embedded system)
	mkdir -p compiled_nanopb/Inc
	mkdir -p compiled_nanopb/Src
	cp ../nanopb-0_4_1/*.h compiled_nanopb/Inc/
	cp ../nanopb-0_4_1/*.c compiled_nanopb/Src/

nanopb_wkt : # One or more Google Well Known Types, compiled here for NanoPB (if WKTs are used in your *.proto, these also need to be compiled and included on your embedded system)
	mkdir -p compiled_nanopb/Inc/google/protobuf
	mkdir -p compiled_nanopb/Src/google/protobuf
	protoc -I=../protobuf-21_2/src/google/protobuf --nanopb_out=compiled_nanopb/Src/google/protobuf ../protobuf-21_2/src/google/protobuf/*.proto
	cp compiled_nanopb/Src/google/protobuf/*.h compiled_nanopb/Inc/google/protobuf/ # [cp is sic] - we are making two copies of these headers for the includes to work without modification, as written by nanopb

.PHONY: clean

clean :
	rm -r compiled_cpp compiled_python compiled_nanopb