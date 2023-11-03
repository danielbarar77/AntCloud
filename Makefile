CFLAGS = -Wall

all: build/client build/server build/worker clean

build/client: build/client.o
	@echo "Linking client executable"
	gcc $(CFLAGS) build/client.o -o build/client

build/server: build/server.o
	@echo "Linking server executable"
	gcc $(CFLAGS) build/server.o -o build/server

build/worker: build/worker.o
	@echo "Linking worker executable"
	gcc $(CFLAGS) build/worker.o -o build/worker

build/client.o: src/client.c
	@echo "Compiling client source"
	gcc $(CFLAGS) -c src/client.c -o build/client.o

build/server.o: src/server.c
	@echo "Compiling server source"
	gcc $(CFLAGS) -c src/server.c -o build/server.o

build/worker.o: src/worker.c
	@echo "Compiling worker source"
	gcc $(CFLAGS) -c src/worker.c -o build/worker.o

clean:
	@echo "Cleaning up"
	rm build/client.o build/server.o build/worker.o
