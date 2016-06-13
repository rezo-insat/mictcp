CC        := gcc
LD        := gcc

MODULES   := api apps 
SRC_DIR   := $(addprefix src/,$(MODULES)) src
BUILD_DIR := $(addprefix build/,$(MODULES)) build

SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ       := $(patsubst src/%.c,build/%.o,$(SRC))
OBJ_CLI   := $(patsubst build/apps/gateway.o,,$(patsubst build/apps/server.o,,$(OBJ)))
OBJ_SERV  := $(patsubst build/apps/gateway.o,,$(patsubst build/apps/client.o,,$(OBJ)))
OBJ_GWAY  := $(patsubst build/apps/server.o,,$(patsubst build/apps/client.o,,$(OBJ)))
INCLUDES  := include 

vpath %.c $(SRC_DIR)

define make-goal
$1/%.o: %.c
	$(CC) -m32 -g -I $(INCLUDES) -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs build/client build/server build/gateway

build/client: $(OBJ_CLI)
	$(LD) -m32 $^ -o $@ -lpthread 

build/server: $(OBJ_SERV)
	$(LD) -m32 $^ -o $@ -lpthread 

build/gateway: $(OBJ_GWAY)
	$(LD) -m32 $^ -o $@ -lpthread 

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))
