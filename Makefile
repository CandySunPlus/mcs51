CC = sdcc
PACK = packihx
MFLAGS = --std-c11 --model-small

TARGET_DIR = build
OBJECTS = led num

BUILD_TARGETS := $(foreach n,$(OBJECTS),$(addprefix $(TARGET_DIR)/$(n)/,$(n)))

all: $(addsuffix .hex,$(BUILD_TARGETS))

$(addsuffix .hex,$(BUILD_TARGETS)): $(addsuffix .ihx,$(BUILD_TARGETS))
	rm -rf $@
	$(PACK) $< > $@

$(addsuffix .ihx,$(BUILD_TARGETS)): $(addsuffix .c,$(OBJECTS))
	mkdir -p $(dir $@)
	$(CC) $(MFLAGS) -o $(dir $@) $<


.PHONY: clean
clean:
	rm -rf $(TARGET_DIR)/*
