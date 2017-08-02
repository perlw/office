TODO
===
## Game
The actual game
* Ability to choose monitor and sound device
* Scene-wrapper-system for scenes instead of raw systems
* Logging
  - Log to file aswell as log to console
  - Separate files for different systems
* Lua bridge
* Sound
  - Ability to load and play sounds, won't need to define
* Occulus
  - Break out log parsing in own app
  - Move compile flag into a runtime check instead?
* Resolutions
  - Load higher res font when possible?
* World edit
  - Load/save world
    - Needs more ui widgets, at minimum some kind of lua front for dialogs etc? modals?
    - Text input widget
    - File listing widget
      - Needs list widget
* Lua UI
* Lua defined tiles, properties, tags, flags, special actions

## Bedrock
Tests?

### Picasso
Graphics, pipeline
* Probable need for reworking of picasso window and gl management

### Archivist module
File management
* Opening/reading/writing
* PAK-files

### Sound/Music
Boombox/Cassette
* Deeper look at cassette usage etc, channels and so *HIGH PRIO*

### Occulus
Memory introspector

### Rectify
Data, memory, sorting

### Tome
Asset management

### Kronos
Systems management
* Profiling

## Planned modules

### Synapse
Networking
