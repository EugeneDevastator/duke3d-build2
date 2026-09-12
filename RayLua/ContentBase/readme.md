resolving of galleries is done on filename basis
individual_tiles:
0001_params_anyname.png

atlases:
A_0001_14x17_spaceingx_spacingy_name.png
14x17 is number of tiles in atlas. 

only png is supported

everything will be repacked in runtime atlases so tile dimensions can be NPOT.
sprites and tiles containing empyt(alpha=0) pixels will be packed separately from solid ones.

grouping and hints are in gal.defs


SND is part of gallery has similar defs
