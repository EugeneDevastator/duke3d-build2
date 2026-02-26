//
// Created by omnis on 2/26/2026.
//

#ifndef RAYLIB_LUA_IMGUI_MAPFORM_DUKE_H
#define RAYLIB_LUA_IMGUI_MAPFORM_DUKE_H

#define SPRITE_BLOCKING         (1 << 0)   // 1
#define SPRITE_SEMI_TRANSPARENT (1 << 1)   // 2
#define SPRITE_FLIP_X           (1 << 2)   // 4
#define SPRITE_FLIP_Y           (1 << 3)   // 8
#define SPRITE_IS_WALL_PLANE     (1 << 4)   // 16
#define SPRITE_IS_FLOOR_PLANE    (1 << 5)   // 32
#define SPRITE_ONE_SIDED        (1 << 6)   // 64
#define SPRITE_TRUE_CENTERED    (1 << 7)   // 128
#define SPRITE_HITSCAN          (1 << 8)   // 256
#define SPRITE_TRANSPARENT      (1 << 9)   // 512
#define SPRITE_IGNORE_SHADE     (1 << 11)  // 2048
#define SPRITE_INVISIBLE        (1 << 15)  // 32768

#define WALL_BLOCKING           (1 << 0)   // 1
#define WALL_BOTTOM_SWAP        (1 << 1)   // 2
#define WALL_ALIGN_FLOOR        (1 << 2)   // 4 // default is align ceil.
#define WALL_FLIP_X             (1 << 3)   // 8
#define WALL_MASKED             (1 << 4)   // 16
#define WALL_SOLID_MASKED       (1 << 5)   // 32
#define WALL_HITSCAN            (1 << 6)   // 64
#define WALL_SEMI_TRANSPARENT   (1 << 7)   // 128
#define WALL_FLIP_Y             (1 << 8)   // 256
#define WALL_TRANSPARENT        (1 << 9)   // 512  // usualy always combines with semitransp.

#define SECTOR_PARALLAX         (1 << 0)   // 1
#define SECTOR_SLOPED           (1 << 1)   // 2
#define SECTOR_SWAP_XY          (1 << 2)   // 4
#define SECTOR_EXPAND_TEXTURE   (1 << 3)   // 8
#define SECTOR_FLIP_X           (1 << 4)   // 16
#define SECTOR_FLIP_Y           (1 << 5)   // 32
#define SECTOR_TEXWALL_ALIGN    (1 << 6)   // 64
#define SECTOR_MASKED           (1 << 7)   // 128
#define SECTOR_TRANSLUCENT      (1 << 8)   // 256
#define SECTOR_REVERSE_TRANS    (SECTOR_MASKED | SECTOR_TRANSLUCENT) // 384

#endif //RAYLIB_LUA_IMGUI_MAPFORM_DUKE_H