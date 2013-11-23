CFLAGS = `pkg-config --cflags opencv` -pthread -Wall -g -std=c++11
LDFLAGS = `pkg-config --libs opencv` -pthread 
LIBS = `pkg-config --libs opencv` -lm
CC = g++
OBJ = Detector.o

%.o: %.cpp 
		$(CC) -c -o $@ $< $(CFLAGS)

detector: $(OBJ) 
	$(CC) -o $@ $^ $(LDFLAGS)
evaluator:
	$(CC) -o evaluator Evaluator.cpp $(CFLAGS) $(LDFLAGS)
All:detector

clean:
	rm -f *.o detector
