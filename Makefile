CC=cl.exe /nologo
DEF=/EHsc /MP8 $(CFLAGS)
ERRS=/Wall
DEPS_INC=/Ideps /Ideps\glfw\include /Ideps\lua-5.3.3\src
DEPS_LIBS=obj\lualib533.lib shell32.lib gdi32.lib user32.lib opengl32.lib deps\glfw\src\glfw3.lib
BIN=office.exe

LUA_OBJS=obj\lapi.obj obj\lcode.obj obj\lctype.obj obj\ldebug.obj obj\ldo.obj obj\ldump.obj obj\lfunc.obj obj\lgc.obj obj\llex.obj obj\lmem.obj obj\lobject.obj obj\lopcodes.obj obj\lparser.obj obj\lstate.obj obj\lstring.obj obj\ltable.obj obj\ltm.obj obj\lundump.obj obj\lvm.obj obj\lzio.obj obj\lauxlib.obj obj\lbaselib.obj obj\lbitlib.obj obj\lcorolib.obj obj\ldblib.obj obj\liolib.obj obj\lmathlib.obj obj\loslib.obj obj\lstrlib.obj obj\ltablib.obj obj\lutf8lib.obj obj\loadlib.obj obj\linit.obj
GLAD_OBJS=obj\glad.obj

BEDROCK_OBJS=obj\b_main.obj obj\b_occulus.obj obj\b_kronos.obj obj\b_gossip.obj obj\b_picasso.obj
OBJS=obj\main.obj

.PHONY:

clean:
  @if exist obj rmdir /S /Q obj
  @if exist build rmdir /S /Q build

prepare:
  @if not exist obj mkdir obj
  @if not exist build mkdir build

deps: $(DEPS_LIBS)

{src/}.c{obj/}.obj:
  $(CC) $(DEF) $(DEPS_INC) /c /Foobj\ $<

{deps/lua-5.3.3/src/}.c{obj/}.obj:
  $(CC) $(DEF) $(DEPS_INC) /c /Foobj\ $<

{deps/glad/}.c{obj/}.obj:
  $(CC) $(DEF) $(DEPS_INC) /c /Foobj\ $<

{src/bedrock/}.c{obj/}.obj:
  $(CC) $(DEF) $(DEPS_INC) /DBEDROCK_IMPLEMENTATION /c /Foobj\ $<

obj\lualib533.lib: $(LUA_OBJS)
	lib /nologo /machine:x64 /out:obj/lualib533.lib $(LUA_OBJS)

opengl32.lib:
gdi32.lib:
user32.lib:
shell32.lib:

build: .PHONY prepare deps $(OBJS) $(GLAD_OBJS)
	link /nologo /machine:x64 /out:build/$(BIN)  /nodefaultlib:libcmtd $(DEPS_LIBS) $(GLAD_OBJS) $(OBJS)
  xcopy /Y /E assets build

run: build
  cls
  @echo Running...
  start /B /D build build/$(BIN)

all: clean prepare build
