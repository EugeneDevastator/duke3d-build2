# Port of duke3d onto Build2
Build 2 by Ken Silverman: https://www.advsys.net/ken/build.htm
## Plan:
- move all gameplay logic into .kc file, using evaldraw as main engine.
for now just boilerplate core into **maptest_duke.kc**
- update build engine to get duke data, like wall horisontal texture split, transparency, pal value etc.

## To see maps in engine you need:
- all .art files in folder with map
- palette.dat in same folder

## To run game
use command line `evaldraw maptest_duke.kc`
