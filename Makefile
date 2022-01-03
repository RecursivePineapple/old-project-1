
-include env.mk

C_SRCS = \
	$(shell find src -type f -name '*.cpp') \
	$(shell find src -type f -name '*.cc') \
	$(shell find src -type f -name '*.c')

SRV_SRCS = \
	src/main.cpp \
	$(shell grep -l -E "^\#pragma target server" $(C_SRCS))

SRV_DBG_SRCS = \
	src/Utils/profiler.cpp \
	src/Utils/base64.cpp \
	$(shell grep -l -E "^\#pragma target server-debug" $(C_SRCS))

SRP_SRCS = \
	src/scratchpad.cpp \
	src/Utils/profiler.cpp

PROTOS =

BINS = bin/server bin/scratchpad

ifeq "$(DEBUG)" "1"
SRV_SRCS    += $(SRV_DBG_SRCS)
endif

##############

PKGS	   += openssl libpq

LIBS	   += -Lbin -l:libusockets.a -lz -luuid -lpthread
INCLS	   += -Isrc -I/usr/include/postgresql/14/server

INCLS      +=  $(shell pkg-config --cflags $(PKGS))
LIBS       +=  $(shell pkg-config --libs $(PKGS))

PBCPPFLAGS += -fPIC
CPPFLAGS   += -fPIC -std=c++17 -fdiagnostics-color -flto
CFLAGS	   += -fPIC -fdiagnostics-color -flto
LDFLAGS    += -shared -fdiagnostics-color -flto -fuse-ld=llvm-link-11

CWARNINGS  += -pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
		-Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op \
		-Wmissing-declarations -Wmissing-include-dirs \
		-Wredundant-decls -Wshadow -Wsign-conversion -Werror \
		-Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused \
		-Wuninitialized -Wno-error=strict-overflow -Wno-unknown-pragmas \
		-DPROTOBUF_INLINE_NOT_IN_HEADERS=0

CPPFLAGS   += $(CWARNINGS) -Wctor-dtor-privacy -Wnoexcept -Wold-style-cast \
		-Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel

CFLAGS     += $(CWARNINGS)

ifeq "$(MAKECMDGOALS)" "test"
DEBUG      ?= 1
endif
ifeq "$(MAKECMDGOALS)" "debug"
DEBUG      ?= 1
endif

ifeq "$(DEBUG)" "1"
CPPFLAGS   += -DDEBUG -g -O0
CFLAGS     += -DDEBUG -g -O0
LDFLAGS    += -Wl,--export-dynamic
else
CPPFLAGS   += -O3
CFLAGS     += -O3
endif

ifeq "$(USE_CLANG)" "1"
CPPFLAGS   += -fno-omit-frame-pointer -Wno-unknown-warning-option -Wunused-command-line-argument
CFLAGS     += -fno-omit-frame-pointer -Wno-unknown-warning-option -Wunused-command-line-argument
CC         := clang-11
CXX        := clang++-11
endif

CPPFLAGS   += $(INCLS)
CFLAGS     += $(INCLS)
LDFLAGS   += -Wl,--as-needed $(LIBS)

##############

PBGENS = \
	$(patsubst src/%.proto, src/generated/%.pb.cc, $(PROTOS)) \
	$(patsubst src/%.proto, src/generated/%.pb.h , $(PROTOS))

HEADERS = \
	$(shell find src -type f -name '*.h') \
	$(shell find src -type f -name '*.hpp')

SRCS = \
	$(shell find src -type f -name '*.cpp') \
	$(shell find src -type f -name '*.cc') \
	$(shell find src -type f -name '*.c') \
	$(shell find src -type f -name '*.h') \
	$(shell find src -type f -name '*.hpp') \
	$(PBGENS)

SRV_OBJS   = $(patsubst src/%,obj/%.o,$(SRV_SRCS))
SRP_OBJS   = $(patsubst src/%,obj/%.o,$(SRP_SRCS))
GCHS       = $(patsubst src/%,src/%.gch,$(shell grep -l -E "^\#pragma precompile-gch" $(HEADERS)))
DEPS       = $(patsubst src/%,dep/%.d,$(SRCS))

##############

.PHONY: all clean run
.PRECIOUS: $(OBJS) $(PBGENS)

all:
	@$(MAKE) -s deps gchs
	@$(MAKE) -s bins

clean:
	- $(RM) $(BINS) $(DEPS) $(SRV_OBJS) $(SRP_OBJS) $(PBGENS) $(GCHS)

run: bin/server
	-PRIVATE_KEY=server.key PUBLIC_KEY=server.crt bin/server

bins: $(BINS)

deps: $(DEPS)

gchs: $(GCHS)

bin/server: $(SRV_OBJS)
	@mkdir -p $(shell dirname $@)
	@echo "  [LD] $@"
	echo $(CXX) -o $@ $^ $(LDFLAGS)
	@llvm-link-11 -o $@ $^ $(patsubst -l%,%,$(LIBS))

bin/scratchpad: $(SRP_OBJS)
	mkdir -p $(shell dirname $@)
	@echo "  [LD] $@"
	$(CXX) -o $@ $^ $(LDFLAGS)

dep/%.cpp.d: src/%.cpp
	@mkdir -p $(shell dirname $@)
	@echo " [DEP] $<"
	@$(CXX) $(CPPFLAGS) -MG -MM $< -MQ $(patsubst src/%,obj/%.o,$<) -MF $@

dep/%.h.d: src/%.h
	@mkdir -p $(shell dirname $@)
	@echo " [DEP] $<"
	@$(CXX) $(CPPFLAGS) -MG -MM $< -MQ $(patsubst src/%,obj/%.o,$<) -MF $@

dep/%.hpp.d: src/%.hpp
	@mkdir -p $(shell dirname $@)
	@echo " [DEP] $<"
	@$(CXX) $(CPPFLAGS) -MG -MM $< -MQ $(patsubst src/%,obj/%.o,$<) -MF $@

dep/%.c.d: src/%.c
	@mkdir -p $(shell dirname $@)
	@echo " [DEP] $<"
	@$(CC) $(CFLAGS) -MG -MM $< -MQ $(patsubst src/%,obj/%.o,$<) -MF $@

src/%.gch: src/%
	@mkdir -p $(shell dirname $@)
	@echo " [GCH] $<"
	@$(CXX) $(CPPFLAGS) $< -o $@

obj/%.cpp.o: src/%.cpp
	@mkdir -p $(shell dirname $@)
	@echo "  [CC] $<"
	@$(CXX) $(CPPFLAGS) -o $@ -c $<

obj/%.c.o: src/%.c
	@mkdir -p $(shell dirname $@)
	@echo "  [CC] $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

obj/%.pb.cc.o: src/%.pb.cc
	@mkdir -p $(shell dirname $@)
	@echo "[PBCC] $<"
	@$(CXX) $(PBCPPFLAGS) -Wno-a -o $@ -c $<

src/generated/%.pb.cc: src/%.proto
	@mkdir -p $(shell dirname $@)
	@echo "[PBUF] $<"
	@cd src && protoc --cpp_out=generated $(patsubst src/%, %, $<)
	@sed -i 's|#include\s*"src/|#include "./|g' $@

src/generated/%.pb.h: src/generated/%.pb.cc

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
