CFLAGS= -I/usr/local/include/lttoolbox-3.3
LDFLAGS= -L/usr/local/lib -llttoolbox3

transducer2: ../src/transducer2.cpp
	g++ -ggdb $(CFLAGS) -Wall ../src/transducer2.cpp -o $@ $(LDFLAGS)

all: transducer2

test:
	./transducer2

clean:
	rm transducer2
	rm -rf transducer2.dSYM
	find . -name "*.fst" -type f -delete
