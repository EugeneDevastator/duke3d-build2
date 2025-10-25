# Compiling
1. youll need luajit in external\luajit folder
2. for windows compile using `msvcbuild.bat static`

Should be run from here: `.\RayLua\External\LuaJIT\src\msvcbuild.bat`

# New map format iteration
this doc is work in progress. I would like to stick to original tag system as much as possible and introduce as less new fields as possible.
## Tags
**Hitags** - use to set shared signal like usual, for ex button with hitag will trigger all activators with same hitag.
**Tags** - are sort of class-id, and assigned based on following logic:
1. Can it overlap or add to other behaviors? - use sprite+tag
2. can it be scoped to walls only? - use wall+tag (hmm, maybe sprite facing wall to pick that wall for behavior overlap?)
3. rest of the cases - sector+tag
So for example water:
in original duke it was tagged per sector.
However in new basis we would tag sprite because there can be more stuff in the water.

Seems like sprite as behavior taggin is main thing for everything.

## New fields - behavior
### classId - 2 bytes.
in future we can merge all behaviors from different games, by this class id.
### RX,TX - 2 bytes each
blood system to emit receive.
### SW polymorphic tags: tag1-tag15
2 bytes each.
can be used for whatever - delays, randomization, etc.
### sw polymorphic bools: bool1-bool32
32 bits total
### additional polymorphic choice: opt1-opt15 
4 bits each (16 options, hopefully enough, like transmitted command, fadng curve, etc.
## new fields - common and map state
### Layer (2bytes)
will be used for chunking purposes or editor selection isolation, for ex. mark entire second floor like layer 2 and then toggle show only layer 2 in editor.
### Renderflags (2bytes)
I want to split flags into maybe just 2 fields, one for visualisation, like, is it flipped, is it wall sprite, etc.
### CoreFlags (2bytes)
those go for physics and other stuff like, blocking, hit receiving, physics layer (last 4 bytes)
### Color (4 bytes rgba)
add Alpha to Ken's field
### Pal and fogpal (1 and 1 byte)
we can combine pal lookup with fog.

# Ai behaviors.
main plan is to make behavior trees, and expose construction of them in lua.
