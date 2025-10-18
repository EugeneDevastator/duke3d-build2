# Port of duke3d onto Build2
Build 2 by Ken Silverman: https://www.advsys.net/ken/build.htm

## To see maps in engine you'll need:
- all .art files in folder with map
- palette.dat in same folder
use grpunpacker from utils

## To run game
use command line `evaldraw maptest_duke.kc`

## Plan:
- move all gameplay logic into .kc file, using evaldraw as main engine.
for now just boilerplate core into **maptest_duke.kc**
- [x] update build engine to get duke data, pal value etc.
- [ ] store masked tile data and transparency info for the future
