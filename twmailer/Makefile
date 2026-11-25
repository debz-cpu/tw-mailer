all: twmailer-server

twmailer-server: src/server/main.cpp
	g++ -std=c++14 -Wall -Werror -o twmailer-server src/server/main.cpp

clean:
	rm -f twmailer-server

.PHONY: all clean