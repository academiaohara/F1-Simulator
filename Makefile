CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror
COMMON = src/f1_data.c src/f1_data_gen.c

.PHONY: test clean

test: build/f1_data_test build/f1_weekend_test build/f1_race_test build/f1_ai_test build/f1_car_test
	./build/f1_data_test
	./build/f1_weekend_test
	./build/f1_race_test
	./build/f1_ai_test
	./build/f1_car_test

build:
	mkdir -p build

build/f1_data_test: tests/f1_data_test.c src/f1_data.c src/f1_data_gen.c | build
	$(CC) $(CFLAGS) $^ -o $@

build/f1_weekend_test: tests/f1_weekend_test.c src/f1_game.c $(COMMON) | build
	$(CC) $(CFLAGS) $^ -o $@

build/f1_race_test: tests/f1_race_test.c src/f1_race.c src/f1_game.c $(COMMON) | build
	$(CC) $(CFLAGS) $^ -o $@

build/f1_ai_test: tests/f1_ai_test.c src/f1_ai.c src/f1_race.c src/f1_game.c $(COMMON) | build
	$(CC) $(CFLAGS) $^ -o $@

build/f1_car_test: tests/f1_car_test.c src/f1_car.c | build
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f build/f1_*_test
