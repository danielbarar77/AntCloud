CFLAGS = -Wall

all: build/client build/server make_worker_copy

make_worker_copy: build/worker
	cp build/worker build/worker_dir/worker
	cp build/worker build/worker2_dir/worker
	rm build/worker

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
	rm build/client build/server build/worker
	rm build/worker_dir/worker
	rm build/worker2_dir/worker

.PHONY: all clean make_worker_copy