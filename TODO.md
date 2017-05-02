TODO
===
## Game
The actual game
* Move slightly away from pure ascii but not quite full tile-game
  * Rename asciilayer to tilelayer? Keep it but create similar for graphical tiles?
  * Multiple tile-layers
    * Status effects etc
    * Move moveables to own layer? Smooth movement? Too much?
* Bedrock modules should not be interdependant
  * Potentially pull occulus out of bedrock?
* Asset management to not load same asset multiple times
  * Textures
  * Shaders

## Bedrock
Tests?

### Picasso
Graphics, pipeline
* Fullscreen support

### Archivist module
File management
* Opening/reading

### Muse
Lua wrapper
* Cleanups
* Go over error management
* Creation of modules

### Sound/Music
Boombox/Cassette
* Deeper look at cassette usage etc, channels and so

### Occulus
Memory introspector

### Rectify
Data, memory, sorting

## Planned modules

### Synapse
Networking

### Tome
Pak/resource file support
