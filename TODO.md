TODO
===
## Game - Plans
* Complete graphics, sound, settings
* Rebuild init of graphics to handle dynamic changes
* Titlescreen
  - Prototype in rexpaint
* Options
  - Prototype in rexpaint
* Initial world editing
  - Needs basic UI capabilities
* Ability to explore created world
  - Networking needs to be in early, preferably at this stage

## Game - Tech
* Settings, graphics and sound etc
  - Ability to choose monitor and sound device
  - Reload graphics on change
  - Resizable window
* Logging
  - Log to file aswell as log to console
  - Separate files for different systems
* Sound
  - Ability to load and play sounds, won't need to define
  - Hook into tome
* World file/data format
  - Header with all used ids
  - Map data should be 0-based references to mentioned ids
* Test harness
* Code documentation and generator
* Remote accesss/debugging of internals (telnet? http?) *Depends on synapse*
  - Send/watch messages
  - See internal states
* Rendering and message queues, prioritizations, etc

## Bedrock

### Picasso
Graphics, pipeline
* Probable need for reworking of picasso window and gl management
* Look over callback usage in regards to multiple windows

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
