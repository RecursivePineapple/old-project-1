
-include env.mk

C_SRCS = \
	$(shell find src -type f -name '*.cpp') \
	$(shell find src -type f -name '*.cc') \
	$(shell find src -type f -name '*.c')

SRV_SRCS      = $(shell grep -l -E "^\#pragma target server$$" $(C_SRCS))
SRV_TEST_SRCS = $(shell grep -l -E "^\#pragma target server-test$$" $(C_SRCS))
SRV_DBG_SRCS  = $(shell grep -l -E "^\#pragma target server-debug$$" $(C_SRCS))
SRP_SRCS      = $(shell grep -l -E "^\#pragma target scratchpad$$" $(C_SRCS))

BINS = bin/server bin/scratchpad

ifeq "$(DEBUG)" "1"
SRV_SRCS    += $(SRV_DBG_SRCS)
endif

##############

PKGS	   += openssl libpq librabbitmq curlpp

LIBS	   += -Lbin -lz -luuid -lusockets
INCLS	   += -Isrc -I/usr/include/postgresql/14/server

INCLS      +=  $(shell pkg-config --cflags $(PKGS))
LIBS       +=  $(shell pkg-config --libs $(PKGS))

PBCPPFLAGS += -fPIC
CPPFLAGS   += -fPIC -std=c++17
CFLAGS	   += -fPIC
LDFLAGS    += -pthread -fuse-ld=mold
DEFINES    += -DPROTOBUF_INLINE_NOT_IN_HEADERS=0 -DBOOST_ALL_DYN_LINK

CPPFLAGS   += $(DEFINES)
CFLAGS     += $(DEFINES)

CWARNINGS  += -pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
		-Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op \
		-Wmissing-declarations -Wmissing-include-dirs \
		-Wredundant-decls -Wshadow -Wsign-conversion -Werror \
		-Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused \
		-Wuninitialized -Wno-error=strict-overflow -Wno-unknown-pragmas

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
CPPFLAGS   += -DDEBUG -g -O0 -fprofile-arcs -ftest-coverage
CFLAGS     += -DDEBUG -g -O0 -fprofile-arcs -ftest-coverage
LDFLAGS    += -Wl,--export-dynamic --coverage
else
CPPFLAGS   += -O3
CFLAGS     += -O3
endif

CC         := clang-11
CXX        := clang++-11

CPPFLAGS   += $(SANITIZERS) $(INCLS) -fno-omit-frame-pointer -Wno-unknown-warning-option -Wunused-command-line-argument
CFLAGS     += $(SANITIZERS) $(INCLS) -fno-omit-frame-pointer -Wno-unknown-warning-option -Wunused-command-line-argument
LDFLAGS    += $(SANITIZERS) -Wl,--as-needed $(LIBS)

##############

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

SRV_OBJS      = $(patsubst src/%,obj/%.o,$(SRV_SRCS))
SRP_OBJS      = $(patsubst src/%,obj/%.o,$(SRP_SRCS))
SRV_TEST_OBJS = $(patsubst src/%,obj/%.o,$(SRV_TEST_SRCS))
DEPS          = $(patsubst src/%,dep/%.d,$(SRCS))

##############

.PHONY: all clean run run-tests watch-tests gen-report watch-server watch-docker-server
.PRECIOUS: $(OBJS)

all:
	@$(MAKE) -s -j16 deps
	@$(MAKE) -s -j16 bins

clean:
	- $(RM) -r bin obj dep coverage

run: bin/server
	-bin/server

run-tests: bin/server-test
	@find obj -name "*.gcda" -exec rm {} \;
	@find obj -name "*.gcdo" -exec rm {} \;
	@echo " [RUN] bin/server-test"
	@bin/server-test

watch-tests:
	@tput reset
	@while true; do inotifywait -e modify,create,delete -r src >/dev/null 2>/dev/null && tput reset && $(MAKE) run-tests gen-report; done

watch-server:
	@tput reset
	@while true; do inotifywait -e modify,create,delete -r src >/dev/null 2>/dev/null && tput reset && $(MAKE) -j16 bin/server; done

watch-docker-server:
	@tput reset
	@while true; do inotifywait -e modify,create,delete -r src >/dev/null 2>/dev/null && tput reset && $(MAKE) -j16 bin/server && docker cp bin/server magegame_server_1:/opt/server/bin/ && docker restart magegame_server_1; done

coverage/lcov.info: bin/server-test $(shell find obj -name "*.gcd*" -type f 2>/dev/null)
	@mkdir -p coverage
	@lcov --directory . --base-directory . --gcov-tool misc/gcov.sh --capture -o coverage/lcov_raw.info >/dev/null
	@lcov --remove coverage/lcov_raw.info -o coverage/lcov.info '/usr/*' 'c++/*' 'cppunit/*' '*/src/Utils/jsmn.hpp' >/dev/null

gen-report: coverage/lcov.info
	@genhtml $^ --demangle-cpp --output-directory coverage/report

bins: $(BINS)

deps: $(DEPS)

bin/server: $(SRV_OBJS)
	@mkdir -p $(shell dirname $@)
	@echo "  [LD] $@"
	@$(CXX) -o $@ $^ $(LDFLAGS)

bin/scratchpad: $(SRP_OBJS)
	@mkdir -p $(shell dirname $@)
	@echo "  [LD] $@"
	@$(CXX) -o $@ $^ $(LDFLAGS)

bin/server-test: $(SRV_TEST_OBJS)
	@mkdir -p $(shell dirname $@)
	@echo "  [LD] $@"
	@$(CXX) -o $@ $^ $(LDFLAGS) -lcppunit

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

obj/%.cpp.o: src/%.cpp
	@mkdir -p $(shell dirname $@)
	@echo "  [CC] $<"
	@$(CXX) $(CPPFLAGS) -o $@ -c $<

obj/%.c.o: src/%.c
	@mkdir -p $(shell dirname $@)
	@echo "  [CC] $<"
	@$(CC) $(CFLAGS) -o $@ -c $<

include gen.mk

ifneq "$(MAKECMDGOALS)" "clean"
-include $(DEPS)
endif
