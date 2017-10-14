TODO
===
## Game - Plans
* Titlescreen
  - Prototype in rexpaint
* Options
  - Prototype in rexpaint
* Initial world editing
  - Needs basic UI capabilities
* Ability to explore created world
  - Networking needs to be in early, preferably at this stage

## Game - Tech
* Move windowing into own system, remove game system for now
  - Merge game, input, into windowing system?
* Kill Tome, atleast this incarnation of it?
* Ability to choose monitor and sound device
* Logging
  - Log to file aswell as log to console
  - Separate files for different systems
* Sound
  - Ability to load and play sounds, won't need to define
  - Hook into tome
* Resolutions
  - Load higher res font when possible?
* World edit
  - Load/save world
    - Needs more ui widgets, at minimum some kind of lua front for dialogs etc? modals?
    - Text input widget
    - File listing widget
      - Needs list widget
* Lua
  * Bridge
  * Tiles, properties, tags, flags, special actions
  * UI
    - Abstract window management
    - Widgets
* World file/data format
  - Header with all used ids
  - Map data should be 0-based references to mentioned ids
* Test harness
* Code documentation and generator
* Remote accesss/debugging of internals (telnet? http?) *Depends on synapse*
  - Send/watch messages
  - See internal states

## Bedrock

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
* Break out log parsing in own app
* Move compile flag into a runtime check instead?

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
