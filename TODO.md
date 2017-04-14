TODO
===
## Game
The actual game
* Clean up main.c
* Bedrock modules should not be interdependant
  * Remove input<->action management from bedrock/picasso
  * Remove dependencies to gossip, fine if apps use it but bedrock should not
  * Potentially pull occulus out of bedrock?

## Bedrock

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
