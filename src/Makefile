CFLAGS=-I/home/fran/local/include/lttoolbox-3.3 -I/usr/local/include/lttoolbox-3.3
LDFLAGS=-L/home/fran/local/lib -L/usr/local/lib -llttoolbox3
transducer: transducer.cpp
	g++ -ggdb $(CFLAGS) -Wall transducer.cpp -o transducer $(LDFLAGS)

processor: processor.cpp
	g++ -ggdb $(CFLAGS) -Wall processor.cpp -o processor $(LDFLAGS)

all: transducer processor

test:
	./transducer
