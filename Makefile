CC	= clang
TARGET 	= aarch64-linux-gnu
SYSROOT	= $(HOME)/odroid_sysroot
FLAGS 	= --target=$(TARGET) --sysroot=$(SYSROOT) -fuse-ld=lld -z notext -lz --verbose
UFLAGS	= -O0 -Wall -gdwarf-4
SESNAME	= benchmark
HOST 	= engelnet.ddns.net

SOURCE_FILES 	= benchmark.c gift.c gift_sliced.c gift_neon.c
TARGET_OUT 	= benchmark

.PHONY: all clean

all: run

# auto run on remote tmux session named $(SESNAME)
run: deploy
	ssh bastian@$(HOST) -p 65534 "tmux send -t $(SESNAME).0 './benchmark' ENTER"

deploy: $(TARGET_OUT)
	rsync -av -e 'ssh -p 65534' --progress $(TARGET_OUT) bastian@$(HOST):/home/bastian/$(TARGET_OUT)

$(TARGET_OUT): $(SOURCE_FILES)
	$(CC) $(FLAGS) $(UFLAGS) -o $@ $^

clean:
	rm -f $(TARGET_OUT)
