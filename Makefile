CFLAGS = -Wall

all: build/client build/server build/worker

build/client: src/client.c src/base64.c
	@echo "Linking client executable"
	gcc $(CFLAGS) $^ -o $@ -g
	gcc $(CFLAGS) $^ -o $@ -g

build/server: src/server.c src/svnet.c
	@echo "Linking server executable"
	gcc $(CFLAGS) $^ -o $@ -g

build/worker: src/worker.c src/base64.c
	@echo "Linking worker executable"
	gcc $(CFLAGS) $^ -o $@ -g

clean:
	@echo "Cleaning up"
	rm build/*

.PHONY: all clean