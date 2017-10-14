Office
===
Exploratory implementation of fake ascii rendering and gameplay

# Compiling
Fetch submodules first.
```bash
git submodule init
git submodule update
```

## Prepare build
> Requires >=cmake 3.0
* Options:
* OCCULUS_LOG_MEM (OFF by default)
* RECTIFY_DEBUG (OFF by default)
* LUA_BRIDGE_DEBUG (OFF by default)

```bash
mkdir build
cd build
cmake ..
```

## Compiling
```bash
make
```
