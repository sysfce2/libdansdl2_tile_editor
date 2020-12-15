# Tile editor

A new version of the tile editor. Less hacky and much more structured and planned than the previous one, taking more advantage of the tools at hand.

There are some red flags, like the monstruous "editor" controller (could be separated into smaller pieces) or how creating a new layer will not instantly load its toolset until the editor state is invoked again, but it works in a much more streamlined way I would also say that the config format is also better and more intutive and than the separation of layers (in which each layer can be of its own type) makes for a more generic tool.

## TODO

- poly layers could have winding
- poly layers could allow concave / convex / all properties
- add poly
- select poly
- move poly
- delete poly
- sort polys on save
- change properties of poly
- edit poly layer properties (set, id, alpha), send a ptr to the layer and be done.

## Building

You will need a compiler that supports the c++17 standard and a cmake that is at least 3.5.1.

First grab these development libraries:

You will also need the following development libraries:

- SDL2
- SDL2_ttf
- SDL2_mixer
- SDL2_image
- GL

Next, build and install these repos first (from github.com/themarlboroman). Don't worry, they can all be made and installed with cmake:

- log
- libdansdl2
- tools
- ldtools
- dfw

Then go the usual cmake way.

## How to use

./editor.out -c #configfile# [-f #mapfile#] [-w #WxH#]

Where:
	- c : configuration file (see below, configfile in "input files").
	- f : optional map file
	- w : optional window size expressed as width x height, as in 800x600.

## Input files
### configfile

Contains:

- Templates for map properties
- Tilesets
- Object sets
	- Linked properties can ONLY be integers :D.
- Polygon sets
	- Linked properties can ONLY be integers :D. W and H cannot be linked.
- Session data

Can be reused along many map files thast share the same building blocks.

### mapfile

Contains

- Map metadata
- Map attributes (from map property templates!)
- Tiles
- Things (in the grand Doom tradition, also known as "objects").
- Polygons

## What are...

### Layers

A layer is a homogeneous group of map entities (tiles, objects or polygons). The number and ordering of layers is open.

### Templates for map properties

TODO

### Tilesets

TODO

### Object sets

TODO

### Polygon sets

TODO

## Output file format

TODO

## History

- summer - winter 2020: Refactor.
	- Complete refactor, backwards compatibility breaking.
- 7-8-2017: Changes.
	- Added the possibility to change the size of logic objects... This actually has consequences in many places (loaders, exporters, logic editor...).
- 3-8-2017: Changes.
	- Added alpha to each layer so shadow layers can be done.
	- Added alpha and separator space to layer properties controller.
	- Added the possibility to change cell size, which previously was impossible without fiddling with the map files.
	- Also added this value to importers and exporters.
	- Changed a bit the structure of each layer to further presentational changes (such as background colours) can be changed easily.
	- Hacked the life away to support png alpha.
- 23-7-2017: Changes.
	- Added dnot export-import.
	- Fixed bug in dnot_parser, of course, only in the frozen branch here.
- 22-7-2017: Many more changes.
	- Added dnot export. Still not 100% functional.
	- Changed the way how properties work in objects, using a simple key->value sequence instead of the numerical index thing.
- 21-7-2017: Many changes...
	- Added grid and file properties dialog.
	- Added also a little message in the bottom, to know what you are doing.
	- Fixed bug in tile selection shortcut.
	- Added background to tile selection list, to make it more usable.
	- Removed old bracket input to change tiles.
- 19-7-2017: Added keyboard tile selection shortcuts.
- As of July 2017 the dependencies have been added and the project has been patched to work with the bundled dependencies.-
- As of November 5th, 2015, the tool has been patched with a compatibility layer to support the many changes made to the original libdansdl2. Should the tool be updated, the compatibility layer will be removed. The tool now needs the SDL2_ttf library to link with (even though it's not used).

## Credits

Bebas Neue font by Dharma Type.
Monofonto and Unispace by Raymond Larabie
Example tileset 1 by Paulina Riva
Example tileset 2 by mfburn
