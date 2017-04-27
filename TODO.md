TODO
===
## Game
The actual game
* Clean up main.c
* Make asciilayer support colors, multiple fonts, etc
  * Multiple kinds of colormanagement
    * Grayscale pixel (rgb = same) is multiplied with tile color
    * Not grayscale pixel (rgb != same) is shown verbatim
    * Alpha channel < 1 is discard
* Bedrock modules should not be interdependant
  * Potentially pull occulus out of bedrock?

## Bedrock
Tests?

### Picasso
Graphics, pipeline

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
