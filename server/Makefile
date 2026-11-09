# Build from this folder. Needs httplib.h in this folder (cpp-httplib, single header).
# The backend is compiled from DSA-Project-2 untouched; its main.cpp (the test driver) is NOT used.
BACKEND ?= ../DSA-Project-2

# Windows (MinGW): httplib needs a Windows 10 target, winsock, and static linking to avoid missing-DLL exits.
ifeq ($(OS),Windows_NT)
  PLATFORM_FLAGS = -D_WIN32_WINNT=0x0A00 -static -lws2_32
endif

server: server.cpp server_core.h httplib.h $(BACKEND)/Trie.cpp $(BACKEND)/RedBlackTree.cpp
	g++ -std=c++17 -O2 -I$(BACKEND) -I. server.cpp $(BACKEND)/Trie.cpp $(BACKEND)/RedBlackTree.cpp -o server -pthread $(PLATFORM_FLAGS)

httplib.h:
	curl -L -o httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

clean:
	rm -f server server.exe
