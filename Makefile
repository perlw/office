CC=clang
DEF=-std=gnu99
ERRS=-Wall -Wno-missing-braces
DEPS_INC=-Ideps -Ideps/glfw/include -Ideps/lua-5.3.3/src
DEPS_LIBS=obj/bedrock.a obj/lualib533.a obj/glad.o deps/glfw/build/src/libglfw3.a
BIN=office

.PHONY:

clean:
	rm -rf obj build

prepare:
	mkdir -p ./obj
	mkdir -p ./build

# +lua
LUA_OBJS=obj/lapi.o obj/lcode.o obj/lctype.o obj/ldebug.o obj/ldo.o obj/ldump.o obj/lfunc.o obj/lgc.o obj/llex.o obj/lmem.o obj/lobject.o obj/lopcodes.o obj/lparser.o obj/lstate.o obj/lstring.o obj/ltable.o obj/ltm.o obj/lundump.o obj/lvm.o obj/lzio.o obj/lauxlib.o obj/lbaselib.o obj/lbitlib.o obj/lcorolib.o obj/ldblib.o obj/liolib.o obj/lmathlib.o obj/loslib.o obj/lstrlib.o obj/ltablib.o obj/lutf8lib.o obj/loadlib.o obj/linit.o

$(LUA_OBJS): obj/%.o:deps/lua-5.3.3/src/%.c
	$(CC) $(DEF) -fPIC -c $< -o $@

obj/lualib533.a: $(LUA_OBJS)
	ar rcs obj/lualib533.a $(LUA_OBJS)
# -lua

obj/glad.o: obj/%.o:deps/glad/%.c
	$(CC) $(DEPS_INC) -c $< -o $@

# +bedrock
BEDROCK_SRCS=$(wildcard src/bedrock/*.c src/bedrock/*/*.c)
BEDROCK_RAWOBJS=$(subst /,-,$(BEDROCK_SRCS:src/%.c=%.o))
BEDROCK_OBJS=$(BEDROCK_RAWOBJS:%.o=obj/%.o)

$(BEDROCK_OBJS): $(BEDROCK_SRCS)
	$(eval SRCFILE=$(subst obj/,src/,$@))
	$(eval SRCFILE=$(subst -,/,$(SRCFILE)))
	$(eval SRCFILE=$(subst .o,.c,$(SRCFILE)))
	$(CC) $(ERRS) $(DEF) $(CFLAGS) $(DEPS_INC) -Isrc/bedrock -c $(SRCFILE) -o $@

obj/bedrock.a: $(BEDROCK_OBJS)
	ar rcs obj/bedrock.a $(BEDROCK_OBJS)
# -bedrock

# +game
OBJS=obj/main.o

deps: $(DEPS_LIBS)

$(OBJS): obj/%.o:src/%.c
	$(CC) $(ERRS) $(DEF) $(CFLAGS) $(DEPS_INC) -c $< -o $@

build: .PHONY prepare deps $(OBJS)
	$(CC) $(ERRS) $(DEF) -o build/$(BIN) $(OBJS) $(DEPS_LIBS) -lX11 -lXrandr -lXinerama -lXxf86vm -lXcursor -ldl -lGL -lm -lpthread -ldl -lrt
	cp -r assets/* build
# -game

run: build
	clear
	echo Running...
	cd build && ./$(BIN)

all: clean prepare build
