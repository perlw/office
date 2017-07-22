TODO
===
## Game
The actual game
* Logging
  - Log to file aswell as log to console
  - Separate files for different systems
* Lua bridge
  - Allow systems to register own interface?
* Sound
  - Ability to load and play sounds, won't need to define
* Occulus
  - Break out log parsing in own app
  - Move compile flag into a runtime check instead?
* Resolutions
  - Load higher res font when possible?
* Input
  - Remove need for "action" command, bake into gossip? Probably?
    Does it even need a separate command? Let be for now and investigate.

### System management
For handling global systems, startup, shutdown, etc
* Message passing
* How to register a system? Necessary?
* Targetted start/stop
* Start/stop all
* Pausing?
* Profiling

## Bedrock
Tests?

### Picasso
Graphics, pipeline
* Probable need for reworking of picasso window and gl management

### Archivist module
File management
* Opening/reading
* PAK-files? or should this be part of tome?

### Sound/Music
Boombox/Cassette
* Deeper look at cassette usage etc, channels and so *HIGH PRIO*

### Occulus
Memory introspector

### Rectify
Data, memory, sorting

### Tome
Asset management

## Planned modules

### Synapse
Networking
