 Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog] and this project attempts to adhere to [Semantic Versioning].

Changes will be documented under Added, Changed, Deprecated, Removed, Fixed or Security headers.

## Known bugs:

## Unreleased
### Pending
- Confirm before exiting?

## [v1.1.13]: 2026-08-08
### Fixed
- Fixes changing background on tileset selector.
- Displays "displaced" tiles on editor and tileset selector.

## [v1.1.12]: 2026-07-30
### Added
- Lctrl + click to duplicate selected item.

## [v1.1.11]: 2025-06-01
### Added
- Right/left to change pages in thing selector.

## [v1.1.10]: 2025-06-01
### Fixed
- Left alt (to snap to grid) works with different zoom levels now.

## [v1.1.9]: 2025-05-17
### Changed
- Allow looping on thing properties.
- Adapt to changes in libraries.

## [v1.1.8]: 2024-08-27
### Fixed
- Property indexes are now correct when browsing them.
### Changed
- Properties now appear in the order they are declared.

## [v1.1.7]: 2024-08-19
### Added
- Added left/right for file browser as alternatives to pageup/down.
- Window name now includes filename

## [v1.1.6]: 2024-03-06
### Fixed
- Allows negative numbers to be entered on numeric properties.

## [v1.1.5]: 2024-03-06
### Added
- Changes to adapt to dfw.

## [v1.1.4]: 2024-02-27
### Added
- Changed help to reflect that "i" can be used to insert new stuff.
### Changed
- Inverted sprites now show as such.
- Changes max zoom level.
### Fixes
- Zoom keys and definitions were inverted.

## [v1.1.3]: 2024-02-08
### Changed
- Adapts to dfw library changes.
- Build system updated: will not be able to specify an invalid executable type

## [v1.1.2]: 2022-07-29
### Added
- AppImage build mode
- Log files reside now in the user home .tile_editor directory

## [v1.1.1]: 2022-07-29
### Added
- Log compatibility

## [v1.1.0]: 2022-07-28
### Added
- Adds default layers to config file

## [v1.0.1]: 2021-11-21
### Added
- Makes secondary config file paths relative to the primary file path (id est, no need to run from the config file directory)
- Automatically creates the log directory in the user dir
- Added run-from-anywhere support.
- Added command line help

### Fixed
- Help may not display because not enough memory is available, so it is now split in sections.

## [v1.0.0]: Dec 2020
### Added
- Ported to DFW.

