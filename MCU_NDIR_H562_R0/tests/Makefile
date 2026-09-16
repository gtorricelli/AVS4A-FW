CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Werror

RADAR_SOURCES = ../task/innosent_protocol.c ../task/radar_cluster.c

.PHONY: test clean

test: test_radar_modules
	./test_radar_modules

test_radar_modules: test_radar_modules.c $(RADAR_SOURCES)
	$(CC) $(CFLAGS) -I../task $^ -lm -o $@

clean:
	$(RM) test_radar_modules
