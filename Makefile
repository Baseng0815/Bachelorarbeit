CC	= clang
TARGET 	= aarch64-linux-gnu
SYSROOT	= $(HOME)/odroid_sysroot
FLAGS 	= --target=$(TARGET) --sysroot=$(SYSROOT) -fuse-ld=lld -z notext -lz --verbose
UFLAGS	= -O0 -Wall

SOURCE_FILES 	= gift.c gift.S
TARGET_OUT 	= gift

.PHONY: all clean

all: deploy

deploy: $(TARGET_OUT)
	rsync -av -e 'ssh -p 65534' --progress $(TARGET_OUT) bastian@odroid:/home/bastian/$(TARGET_OUT)

$(TARGET_OUT): $(SOURCE_FILES)
	$(CC) $(FLAGS) $(UFLAGS) -o $@ $^

clean:
	rm -f $(TARGET_OUT)
