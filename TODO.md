TODO
===
## Game
The actual game
* Initial text rendering?
* Ingame graphical debugging?

## Bedrock
* Take a pass and ensure safe memory functions are used (string functions etc)

### Picasso
Graphics, pipeline
* Shader management (need more work)
* Buffer management
* Image management
* Keep track of gl state, atleast for buffers, textures, shaders

### Archivist module
File management
* Opening/reading

### Muse
Lua wrapper
* Config via lua? or simple ini files
* Lua wrapper

### Input handling

### Synapse
Networking

### Occulus
Memory introspector
* Issue when reusing addresses!! Rethink the simple addition of allocation logging
