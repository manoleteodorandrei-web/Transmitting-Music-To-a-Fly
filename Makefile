CC=gcc
CFLAGS=-Wall -g -O2
LIBS=-lfftw3 -lm

all: fly_simulation

mapped_connections.csv: connnections.csv mapper.py
	python3 mapper.py

fly_simulation: main.c connecttome.c audio_pipeline.c
	$(CC) $(CFLAGS) -o fly_simulation main.c connecttome.c audio_pipeline.c $(LIBS)

clean:
	rm -f fly_simulation mapped_connections.csv