#include "common.h"
#include "object.h"
#include "heap.h"
#include "sound.h"
#include "records.h"
#include "fieldstg.h"

/* The battles' enemies (fieldstg_battles.enemies; [0]: none), then the battles (fieldstg_start_battle).
 * Only fieldstg_80087DB0.c reads them, but they start the overlay's .data (DECISIONS "Overlay .data/.bss
 * per file"), so they are defined in its first file (or a data-only file linked first). */
RecordsEnemy fieldstg_battle_enemies[523] = {
    { 0, 0, 0, 0, 0 }, { 358, 12, 816, 9999, 16 }, { 124, 14, 912, 9999, 16 }, { 172, 9, 504, 9999, 22 },
    { 8, 11, 576, 9999, 23 }, { 207, 14, 684, 9999, 20 }, { 222, 12, 612, 9999, 27 }, { 77, 14, 684, 9999, 22 },
    { 119, 17, 792, 200, 14 }, { 11, 27, 768, 9999, 17 }, { 11, 27, 768, 9999, 17 }, { 400, 29, 1224, 9999, 14 },
    { 208, 29, 1224, 9999, 16 }, { 13, 29, 1088, 9999, 18 }, { 356, 32, 1776, 280, 16 }, { 70, 32, 1776, 9999, 16 },
    { 141, 34, 1872, 9999, 16 }, { 399, 33, 912, 9999, 16 }, { 302, 34, 1404, 9999, 16 }, { 437, 38, 3096, 40, 16 },
    { 337, 40, 2160, 9999, 16 }, { 436, 43, 2304, 960, 16 }, { 443, 45, 2400, 9999, 16 },
    { 138, 40, 1620, 9999, 18 }, { 402, 41, 2484, 9999, 16 }, { 237, 44, 1764, 9999, 16 },
    { 444, 46, 2448, 9999, 16 }, { 425, 41, 1656, 9999, 16 }, { 202, 43, 1728, 9999, 16 },
    { 238, 46, 1836, 9999, 17 }, { 447, 47, 2496, 9999, 16 }, { 270, 45, 1800, 9999, 17 },
    { 65, 46, 1836, 296, 17 }, { 360, 49, 1944, 9999, 18 }, { 23, 45, 1200, 9999, 17 }, { 427, 46, 1224, 9999, 17 },
    { 409, 49, 1296, 9999, 17 }, { 9, 45, 1200, 9999, 17 }, { 189, 47, 1248, 9999, 17 },
    { 104, 50, 1320, 1800, 17 }, { 244, 46, 1224, 9999, 19 }, { 135, 48, 1272, 9999, 18 },
    { 429, 50, 1320, 9999, 17 }, { 24, 47, 1872, 9999, 17 }, { 410, 49, 1296, 4, 17 }, { 115, 51, 1344, 9999, 17 },
    { 445, 55, 2880, 9999, 16 }, { 446, 57, 2976, 8, 16 }, { 108, 49, 1944, 9999, 18 }, { 193, 50, 1980, 9999, 18 },
    { 428, 52, 2052, 9999, 17 }, { 190, 50, 1980, 9999, 17 }, { 215, 52, 2052, 9999, 17 },
    { 231, 54, 2124, 9999, 16 }, { 431, 50, 1980, 9999, 17 }, { 434, 52, 1368, 9999, 17 },
    { 433, 54, 1416, 9999, 16 }, { 327, 52, 1368, 9999, 17 }, { 327, 52, 1368, 9999, 17 },
    { 247, 55, 1440, 9999, 16 }, { 312, 54, 1133, 9999, 17 }, { 228, 56, 1464, 9999, 16 },
    { 245, 58, 1512, 9999, 17 }, { 178, 99, 9999, 9999, 29 }, { 448, 53, 2784, 9999, 16 },
    { 432, 54, 1416, 9999, 16 }, { 236, 57, 1488, 9999, 17 }, { 438, 60, 3120, 9999, 16 },
    { 439, 57, 4464, 9999, 16 }, { 440, 57, 2976, 9999, 16 }, { 441, 57, 2976, 9999, 16 }, { 32, 1, 120, 9999, 16 },
    { 206, 1, 120, 9999, 16 }, { 197, 3, 168, 9999, 16 }, { 132, 4, 192, 9999, 16 }, { 217, 8, 312, 9999, 16 },
    { 4, 2, 144, 9999, 16 }, { 25, 4, 192, 9999, 16 }, { 51, 2, 144, 9999, 16 }, { 67, 3, 168, 9999, 16 },
    { 198, 3, 168, 9999, 16 }, { 166, 4, 192, 9999, 16 }, { 80, 4, 192, 9999, 16 }, { 277, 16, 504, 9999, 16 },
    { 172, 5, 240, 9999, 16 }, { 137, 13, 432, 9999, 16 }, { 222, 5, 240, 9999, 16 }, { 8, 6, 264, 9999, 16 },
    { 203, 6, 396, 9999, 16 }, { 7, 6, 264, 9999, 16 }, { 110, 8, 312, 9999, 16 }, { 49, 8, 312, 9999, 16 },
    { 37, 9, 336, 9999, 16 }, { 223, 31, 864, 9999, 16 }, { 50, 13, 432, 9999, 16 }, { 207, 10, 360, 9999, 16 },
    { 35, 18, 552, 9999, 16 }, { 208, 29, 816, 9999, 16 }, { 210, 20, 600, 120, 16 }, { 220, 27, 768, 9999, 16 },
    { 221, 40, 1080, 9999, 16 }, { 241, 24, 696, 9999, 16 }, { 119, 20, 600, 200, 16 }, { 10, 12, 408, 9999, 16 },
    { 212, 23, 672, 9999, 16 }, { 42, 20, 600, 64, 16 }, { 34, 20, 600, 9999, 16 }, { 136, 22, 972, 9999, 16 },
    { 108, 44, 1176, 9999, 16 }, { 273, 45, 1200, 9999, 16 }, { 28, 22, 648, 9999, 16 }, { 334, 22, 648, 9999, 16 },
    { 13, 25, 480, 9999, 16 }, { 435, 29, 816, 9999, 16 }, { 176, 23, 672, 150, 16 }, { 122, 23, 672, 336, 16 },
    { 143, 24, 696, 9999, 16 }, { 40, 24, 696, 9999, 16 }, { 11, 25, 720, 9999, 16 }, { 39, 26, 744, 9999, 16 },
    { 227, 47, 1248, 9999, 16 }, { 171, 47, 1248, 9999, 16 }, { 139, 27, 768, 270, 16 }, { 134, 27, 768, 9999, 16 },
    { 54, 46, 1224, 9999, 16 }, { 14, 29, 816, 9999, 16 }, { 76, 30, 840, 9999, 16 }, { 53, 29, 816, 9999, 16 },
    { 173, 31, 864, 9999, 16 }, { 175, 36, 984, 9999, 16 }, { 238, 43, 1152, 9999, 16 }, { 200, 34, 936, 9999, 16 },
    { 121, 38, 1032, 9999, 16 }, { 302, 35, 960, 9999, 16 }, { 364, 35, 960, 9999, 16 },
    { 365, 45, 1200, 9999, 16 }, { 226, 34, 936, 9999, 16 }, { 140, 36, 984, 9999, 16 }, { 138, 36, 984, 9999, 16 },
    { 24, 45, 1800, 9999, 16 }, { 224, 37, 1008, 9999, 16 }, { 225, 44, 1176, 9999, 16 }, { 61, 35, 960, 9999, 16 },
    { 165, 34, 936, 9999, 16 }, { 170, 36, 984, 96, 16 }, { 244, 39, 1056, 9999, 16 }, { 126, 36, 984, 9999, 16 },
    { 38, 41, 1104, 9999, 16 }, { 69, 41, 1104, 9999, 16 }, { 9, 41, 1104, 9999, 16 }, { 52, 41, 1104, 9999, 16 },
    { 23, 41, 1104, 9999, 16 }, { 94, 43, 1152, 9999, 16 }, { 41, 46, 1224, 9999, 16 }, { 270, 43, 1152, 9999, 16 },
    { 327, 49, 1296, 9999, 16 }, { 189, 43, 1152, 9999, 16 }, { 135, 43, 1152, 9999, 16 },
    { 237, 44, 1176, 9999, 16 }, { 229, 44, 1176, 9999, 16 }, { 204, 44, 1176, 9999, 16 },
    { 281, 43, 1152, 9999, 16 }, { 202, 43, 1152, 9999, 16 }, { 360, 44, 1176, 9999, 16 },
    { 60, 44, 1176, 9999, 16 }, { 231, 54, 1416, 9999, 16 }, { 215, 49, 1296, 9999, 16 },
    { 269, 45, 1200, 9999, 16 }, { 177, 45, 1200, 9999, 16 }, { 65, 44, 1176, 296, 16 },
    { 250, 45, 1200, 9999, 16 }, { 193, 45, 1200, 9999, 16 }, { 245, 55, 1440, 9999, 16 },
    { 272, 47, 1248, 9999, 16 }, { 190, 48, 1272, 9999, 16 }, { 228, 56, 1464, 9999, 16 },
    { 115, 47, 1248, 9999, 16 }, { 236, 55, 1440, 9999, 16 }, { 251, 56, 1464, 9999, 16 },
    { 312, 49, 1037, 9999, 16 }, { 247, 55, 1440, 9999, 16 }, { 382, 56, 2196, 9999, 16 }, { 77, 9, 336, 9999, 16 },
    { 395, 30, 840, 9999, 16 }, { 396, 8, 312, 9999, 16 }, { 397, 21, 624, 9999, 16 }, { 398, 22, 648, 9999, 16 },
    { 399, 33, 608, 9999, 16 }, { 400, 33, 912, 9999, 16 }, { 401, 19, 576, 9999, 16 }, { 402, 41, 1656, 9999, 16 },
    { 403, 21, 624, 9999, 16 }, { 404, 37, 1008, 9999, 16 }, { 405, 22, 648, 9999, 16 }, { 406, 36, 984, 9999, 16 },
    { 407, 37, 1008, 360, 16 }, { 408, 35, 960, 9999, 16 }, { 409, 47, 1248, 9999, 16 }, { 89, 42, 1128, 9999, 16 },
    { 410, 46, 1224, 4, 16 }, { 411, 48, 1272, 9999, 16 }, { 412, 46, 1224, 9999, 16 }, { 413, 31, 864, 9999, 16 },
    { 414, 39, 1056, 9999, 16 }, { 415, 43, 1152, 9999, 16 }, { 416, 43, 1152, 9999, 16 },
    { 417, 44, 1176, 9999, 16 }, { 418, 40, 1080, 9999, 16 }, { 419, 43, 1152, 9999, 16 },
    { 420, 45, 1200, 9999, 16 }, { 421, 32, 888, 9999, 16 }, { 422, 40, 1080, 9999, 16 },
    { 423, 48, 1272, 9999, 16 }, { 424, 36, 984, 9999, 16 }, { 425, 41, 1104, 9999, 16 },
    { 426, 47, 1872, 9999, 16 }, { 427, 44, 1176, 9999, 16 }, { 428, 48, 1272, 9999, 16 },
    { 429, 48, 1272, 9999, 16 }, { 430, 44, 1176, 9999, 16 }, { 104, 48, 1272, 1800, 16 },
    { 431, 47, 1872, 9999, 16 }, { 432, 54, 1416, 9999, 16 }, { 433, 54, 1416, 9999, 16 },
    { 434, 49, 1296, 9999, 16 }, { 14, 29, 816, 9999, 16 }, { 53, 29, 816, 9999, 16 }, { 435, 29, 816, 9999, 16 },
    { 76, 30, 840, 9999, 16 }, { 435, 29, 816, 9999, 16 }, { 76, 30, 840, 9999, 16 }, { 76, 30, 840, 9999, 16 },
    { 334, 32, 888, 9999, 22 }, { 139, 29, 816, 270, 17 }, { 134, 29, 816, 9999, 17 }, { 225, 44, 1176, 9999, 16 },
    { 229, 44, 1176, 9999, 16 }, { 41, 46, 1224, 9999, 16 }, { 245, 55, 1440, 9999, 16 },
    { 251, 56, 1464, 9999, 16 }, { 427, 48, 1272, 9999, 17 }, { 411, 50, 1320, 9999, 17 },
    { 411, 50, 1320, 9999, 17 }, { 412, 50, 1320, 9999, 17 }, { 231, 50, 1320, 9999, 15 },
    { 228, 51, 1344, 9999, 15 }, { 247, 52, 1368, 9999, 15 }, { 178, 53, 2784, 9999, 16 },
    { 231, 50, 1320, 9999, 15 }, { 228, 51, 1344, 9999, 15 }, { 51, 1, 100, 9999, 12 }, { 4, 4, 192, 9999, 21 },
    { 25, 4, 192, 9999, 16 }, { 198, 5, 240, 9999, 23 }, { 67, 40, 1080, 9999, 103 }, { 51, 40, 1080, 9999, 120 },
    { 137, 45, 1200, 9999, 44 }, { 32, 6, 264, 9999, 35 }, { 32, 7, 288, 9999, 38 }, { 166, 9, 336, 9999, 28 },
    { 80, 7, 288, 9999, 24 }, { 34, 8, 312, 9999, 8 }, { 197, 6, 264, 9999, 25 }, { 132, 7, 288, 9999, 24 },
    { 110, 7, 288, 9999, 15 }, { 206, 9, 336, 9999, 45 }, { 49, 40, 1080, 9999, 55 }, { 50, 40, 1080, 9999, 40 },
    { 7, 50, 1320, 9999, 80 }, { 4, 6, 264, 9999, 29 }, { 198, 7, 288, 9999, 27 }, { 77, 7, 288, 9999, 14 },
    { 51, 7, 288, 9999, 32 }, { 137, 7, 288, 9999, 11 }, { 396, 7, 288, 9999, 15 }, { 203, 10, 540, 9999, 22 },
    { 136, 10, 540, 9999, 9 }, { 28, 10, 360, 9999, 9 }, { 176, 11, 384, 150, 9 }, { 217, 10, 360, 9999, 18 },
    { 37, 10, 360, 9999, 17 }, { 10, 40, 1080, 9999, 42 }, { 50, 42, 1128, 9999, 42 }, { 35, 39, 1056, 9999, 31 },
    { 197, 17, 528, 9999, 50 }, { 132, 17, 528, 9999, 44 }, { 110, 18, 552, 9999, 28 }, { 32, 9, 336, 9999, 45 },
    { 166, 10, 360, 9999, 30 }, { 166, 10, 360, 9999, 30 }, { 206, 41, 1104, 9999, 147 }, { 170, 37, 1008, 96, 16 },
    { 223, 38, 1032, 9999, 19 }, { 364, 39, 1056, 9999, 18 }, { 210, 37, 1008, 120, 27 },
    { 14, 38, 1032, 9999, 20 }, { 407, 39, 1056, 360, 17 }, { 51, 36, 984, 9999, 109 }, { 395, 39, 1056, 9999, 20 },
    { 226, 40, 1080, 9999, 18 }, { 126, 40, 1080, 9999, 18 }, { 138, 40, 1080, 9999, 18 },
    { 140, 41, 1104, 9999, 18 }, { 405, 38, 1032, 9999, 25 }, { 408, 41, 1104, 9999, 18 },
    { 121, 41, 1104, 9999, 17 }, { 197, 41, 1104, 9999, 105 }, { 132, 41, 1104, 9999, 92 },
    { 110, 42, 1128, 9999, 58 }, { 206, 42, 1128, 9999, 150 }, { 51, 57, 1488, 9999, 165 },
    { 396, 58, 1512, 9999, 78 }, { 395, 60, 1560, 9999, 30 }, { 203, 42, 1692, 9999, 68 },
    { 136, 42, 1692, 9999, 28 }, { 8, 42, 1128, 9999, 68 }, { 166, 41, 1104, 9999, 92 },
    { 397, 43, 1152, 9999, 30 }, { 39, 42, 1128, 9999, 24 }, { 210, 34, 936, 120, 25 }, { 14, 35, 960, 9999, 19 },
    { 35, 35, 960, 9999, 28 }, { 241, 34, 936, 9999, 22 }, { 165, 36, 984, 9999, 17 }, { 37, 35, 960, 9999, 46 },
    { 223, 35, 960, 9999, 18 }, { 364, 36, 984, 9999, 16 }, { 122, 35, 960, 336, 23 }, { 143, 35, 960, 9999, 22 },
    { 61, 35, 960, 9999, 16 }, { 408, 35, 960, 9999, 16 }, { 121, 35, 960, 9999, 15 }, { 220, 36, 984, 9999, 20 },
    { 221, 36, 984, 9999, 15 }, { 126, 36, 984, 9999, 16 }, { 8, 34, 936, 9999, 57 }, { 140, 37, 1008, 9999, 16 },
    { 170, 44, 1176, 96, 19 }, { 223, 44, 1176, 9999, 22 }, { 404, 45, 1200, 9999, 19 }, { 4, 44, 1176, 9999, 131 },
    { 198, 44, 1176, 9999, 112 }, { 424, 45, 1200, 9999, 20 }, { 210, 43, 1152, 120, 31 },
    { 14, 44, 1176, 9999, 23 }, { 269, 45, 1200, 9999, 16 }, { 397, 45, 1200, 9999, 31 },
    { 171, 46, 1224, 9999, 16 }, { 126, 44, 1176, 9999, 19 }, { 138, 45, 1200, 9999, 20 },
    { 212, 45, 1200, 9999, 29 }, { 28, 44, 1176, 9999, 29 }, { 176, 44, 1176, 150, 28 },
    { 229, 46, 1224, 9999, 17 }, { 395, 44, 1176, 9999, 22 }, { 226, 45, 1200, 9999, 21 },
    { 60, 46, 1224, 9999, 17 }, { 224, 45, 1200, 9999, 19 }, { 225, 46, 1224, 9999, 17 },
    { 419, 45, 1200, 9999, 17 }, { 54, 46, 1224, 9999, 16 }, { 176, 52, 1368, 150, 33 },
    { 229, 53, 1392, 9999, 19 }, { 41, 53, 1392, 9999, 18 }, { 203, 52, 2052, 9999, 83 }, { 8, 52, 1368, 9999, 83 },
    { 409, 54, 1416, 9999, 18 }, { 206, 52, 1368, 9999, 182 }, { 365, 53, 1392, 9999, 19 },
    { 197, 52, 1368, 9999, 130 }, { 132, 52, 1368, 9999, 114 }, { 110, 53, 1392, 9999, 71 },
    { 224, 52, 1368, 9999, 22 }, { 225, 52, 1368, 9999, 19 }, { 281, 53, 1392, 9999, 19 },
    { 226, 52, 1368, 9999, 23 }, { 60, 52, 1368, 9999, 19 }, { 423, 53, 1392, 9999, 18 },
    { 122, 53, 1392, 336, 33 }, { 143, 53, 1392, 9999, 32 }, { 250, 54, 1416, 9999, 19 },
    { 405, 55, 1440, 9999, 36 }, { 425, 57, 1488, 9999, 22 }, { 428, 60, 1560, 9999, 20 },
    { 364, 53, 1392, 9999, 23 }, { 177, 53, 1392, 9999, 19 }, { 206, 53, 1392, 9999, 186 },
    { 365, 54, 1416, 9999, 19 }, { 32, 1, 120, 9999, 16 }, { 32, 1, 120, 9999, 16 }, { 32, 1, 120, 9999, 16 },
    { 32, 1, 120, 9999, 16 }, { 32, 1, 120, 9999, 16 }, { 32, 1, 120, 9999, 16 }, { 32, 1, 120, 9999, 16 },
    { 32, 1, 120, 9999, 16 }, { 32, 1, 120, 9999, 16 }, { 141, 90, 4560, 9999, 39 }, { 436, 95, 4800, 960, 33 },
    { 382, 99, 7488, 9999, 27 }, { 203, 14, 684, 9999, 28 }, { 51, 10, 360, 9999, 40 }, { 32, 6, 264, 9999, 35 },
    { 449, 17, 1056, 9999, 16 }, { 451, 26, 1488, 9999, 16 }, { 452, 28, 1584, 9999, 16 },
    { 453, 29, 1632, 9999, 16 }, { 454, 28, 1584, 9999, 16 }, { 456, 28, 1584, 9999, 16 },
    { 455, 28, 1584, 9999, 16 }, { 450, 19, 1152, 9999, 16 }, { 51, 5, 240, 9999, 27 }, { 395, 7, 288, 9999, 5 },
    { 281, 43, 1152, 9999, 16 }, { 202, 43, 1152, 9999, 16 }, { 204, 44, 1176, 9999, 16 },
    { 229, 44, 1176, 9999, 16 }, { 24, 45, 1800, 9999, 16 }, { 122, 23, 672, 336, 16 }, { 143, 24, 696, 9999, 16 },
    { 61, 35, 960, 9999, 16 }, { 408, 35, 960, 9999, 16 }, { 121, 38, 1032, 9999, 16 }, { 220, 27, 768, 9999, 16 },
    { 221, 40, 1080, 9999, 16 }, { 126, 36, 984, 9999, 16 }, { 8, 6, 264, 9999, 16 }, { 140, 36, 984, 9999, 16 },
    { 170, 36, 984, 96, 16 }, { 223, 31, 864, 9999, 16 }, { 404, 37, 1008, 9999, 16 }, { 4, 2, 144, 9999, 16 },
    { 198, 3, 168, 9999, 16 }, { 424, 36, 984, 9999, 16 }, { 210, 20, 600, 120, 16 }, { 14, 29, 816, 9999, 16 },
    { 269, 45, 1200, 9999, 16 }, { 397, 21, 624, 9999, 16 }, { 171, 47, 1248, 9999, 16 },
    { 126, 36, 984, 9999, 16 }, { 138, 36, 984, 9999, 16 }, { 212, 23, 672, 9999, 16 }, { 28, 22, 648, 9999, 16 },
    { 176, 23, 672, 150, 16 }, { 229, 44, 1176, 9999, 16 }, { 395, 30, 840, 9999, 16 }, { 226, 34, 936, 9999, 16 },
    { 60, 44, 1176, 9999, 16 }, { 224, 37, 1008, 9999, 16 }, { 225, 44, 1176, 9999, 16 },
    { 419, 43, 1152, 9999, 16 }, { 54, 46, 1224, 9999, 16 }, { 176, 23, 672, 150, 16 }, { 229, 44, 1176, 9999, 16 },
    { 41, 46, 1224, 9999, 16 }, { 203, 6, 396, 9999, 16 }, { 8, 6, 264, 9999, 16 }, { 409, 47, 1248, 9999, 16 },
    { 206, 1, 120, 9999, 16 }, { 365, 45, 1200, 9999, 16 }, { 197, 3, 168, 9999, 16 }, { 132, 4, 192, 9999, 16 },
    { 110, 8, 312, 9999, 16 }, { 224, 37, 1008, 9999, 16 }, { 225, 44, 1176, 9999, 16 },
    { 281, 43, 1152, 9999, 16 }, { 226, 34, 936, 9999, 16 }, { 60, 44, 1176, 9999, 16 },
    { 423, 48, 1272, 9999, 16 }, { 122, 23, 672, 336, 16 }, { 143, 24, 696, 9999, 16 }, { 250, 45, 1200, 9999, 16 },
    { 405, 22, 648, 9999, 16 }, { 425, 41, 1104, 9999, 16 }, { 428, 48, 1272, 9999, 16 },
    { 364, 35, 960, 9999, 16 }, { 177, 45, 1200, 9999, 16 }, { 206, 1, 120, 9999, 16 }, { 365, 45, 1200, 9999, 16 },
    { 449, 60, 3120, 9999, 47 }, { 451, 60, 3120, 9999, 34 }, { 450, 65, 3360, 9999, 47 },
    { 453, 60, 3120, 9999, 31 }, { 358, 70, 3600, 9999, 71 }, { 124, 75, 3840, 9999, 67 },
    { 237, 80, 4080, 9999, 28 }, { 425, 70, 2700, 9999, 26 }, { 437, 75, 5760, 40, 30 },
    { 337, 80, 4080, 9999, 30 }, { 193, 70, 2700, 9999, 24 }, { 432, 75, 2880, 9999, 22 },
    { 356, 80, 4080, 280, 37 }, { 215, 70, 2700, 9999, 22 }, { 70, 75, 3840, 9999, 35 }, { 24, 80, 6120, 9999, 27 },
    { 452, 60, 3120, 9999, 32 }, { 454, 70, 3600, 9999, 36 }, { 455, 70, 3600, 9999, 36 },
    { 456, 70, 1776, 9999, 16 }, { 141, 34, 1872, 9999, 16 }, { 337, 40, 2160, 9999, 16 },
    { 436, 44, 2352, 960, 16 }, { 443, 65, 3360, 9999, 22 }, { 444, 65, 3360, 9999, 22 },
    { 447, 70, 3600, 9999, 23 }, { 445, 70, 3600, 9999, 20 }, { 446, 75, 3840, 8, 21 }, { 448, 75, 3840, 9999, 22 },
    { 465, 28, 792, 9999, 16 }, { 436, 15, 960, 960, 7 }, { 442, 65, 5040, 9999, 16 }, { 466, 70, 3600, 9999, 16 },
    { 467, 70, 5400, 9999, 16 }, { 466, 70, 3600, 9999, 16 }, { 467, 70, 5400, 9999, 16 },
    { 457, 7, 288, 9999, 16 }, { 458, 12, 408, 9999, 16 }, { 459, 17, 528, 9999, 16 }, { 460, 27, 768, 9999, 16 },
    { 461, 37, 1008, 9999, 16 }, { 462, 42, 1128, 9999, 16 }, { 463, 47, 1248, 9999, 16 },
    { 464, 47, 1248, 9999, 16 },
};

FieldstgBattle fieldstg_battles[335] = {
    { { &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[1], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[2], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[3], &fieldstg_battle_enemies[4], &fieldstg_battle_enemies[5] }, 0, 2, { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[6], &fieldstg_battle_enemies[7], &fieldstg_battle_enemies[8] }, 0x7F, 2, { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[9], &fieldstg_battle_enemies[10], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[11], &fieldstg_battle_enemies[12], &fieldstg_battle_enemies[13] }, 0, 2, { 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[14], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[15], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[16], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[17], &fieldstg_battle_enemies[18], &fieldstg_battle_enemies[19] }, 0, 2, { 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[20], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[21], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[22], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[23], &fieldstg_battle_enemies[24], &fieldstg_battle_enemies[25] }, 0, 2, { 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[26], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[27], &fieldstg_battle_enemies[28], &fieldstg_battle_enemies[29] }, 0x7F, 2, { 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[30], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[31], &fieldstg_battle_enemies[32], &fieldstg_battle_enemies[33] }, 0, 2, { 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[34], &fieldstg_battle_enemies[35], &fieldstg_battle_enemies[36] }, 0, 2, { 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[37], &fieldstg_battle_enemies[38], &fieldstg_battle_enemies[39] }, 0, 2, { 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[40], &fieldstg_battle_enemies[41], &fieldstg_battle_enemies[42] }, 0, 2, { 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[43], &fieldstg_battle_enemies[44], &fieldstg_battle_enemies[45] }, 0, 2, { 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[46], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[47], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[48], &fieldstg_battle_enemies[49], &fieldstg_battle_enemies[50] }, 0, 2, { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[51], &fieldstg_battle_enemies[52], &fieldstg_battle_enemies[53] }, 0, 2, { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[54], &fieldstg_battle_enemies[55], &fieldstg_battle_enemies[56] }, 0, 2, { 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[57], &fieldstg_battle_enemies[58], &fieldstg_battle_enemies[59] }, 0, 2, { 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[60], &fieldstg_battle_enemies[61], &fieldstg_battle_enemies[62] }, 0, 2, { 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[63], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x7F, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[64], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[65], &fieldstg_battle_enemies[66], &fieldstg_battle_enemies[67] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[68], &fieldstg_battle_enemies[69], &fieldstg_battle_enemies[70] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1 } },
    { { &fieldstg_battle_enemies[71], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[72], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[73], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[74], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[75], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[76], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[77], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[78], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[79], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[80], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[81], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[82], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[83], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[84], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[85], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[86], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[87], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[88], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[89], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[90], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[91], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[92], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[93], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[94], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[95], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[96], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[97], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[98], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[99], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[100], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[101], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[102], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[103], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[104], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[105], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[106], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[107], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[108], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[109], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[110], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[111], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[112], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[113], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[114], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[115], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[116], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[117], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[118], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[119], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[120], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[121], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[122], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[123], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[124], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[125], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[126], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[127], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[128], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[129], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[130], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[131], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[132], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[133], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[134], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[135], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[136], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[137], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[138], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[139], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[140], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[141], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[142], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[143], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[144], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[145], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[146], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[147], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[148], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[149], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[150], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[151], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[152], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[153], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[154], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[155], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[156], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[157], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[158], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[159], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[160], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[161], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[162], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[163], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[164], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[165], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[166], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[167], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[168], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[169], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[170], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[171], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[172], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[173], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[174], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[175], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[176], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[177], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[178], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[179], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[180], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[181], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x7F, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[182], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[183], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[184], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[185], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[186], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[187], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[188], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[189], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[190], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[191], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[192], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[193], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[194], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[195], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[196], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[197], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[198], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[199], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[200], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[201], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[202], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[203], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x14, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[204], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x18, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[205], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x1C, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[206], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[207], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[208], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[209], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[210], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[211], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[212], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[213], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[214], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[215], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[216], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[217], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[218], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0xC, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[219], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[220], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[221], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[222], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[223], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[224], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 4, 1, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[225], &fieldstg_battle_enemies[226], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[227], &fieldstg_battle_enemies[228], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[229], &fieldstg_battle_enemies[230], &fieldstg_battle_enemies[231] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[232], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[233], &fieldstg_battle_enemies[234], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[235], &fieldstg_battle_enemies[236], &fieldstg_battle_enemies[237] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[238], &fieldstg_battle_enemies[239], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[240], &fieldstg_battle_enemies[241], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[242], &fieldstg_battle_enemies[243], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[244], &fieldstg_battle_enemies[245], &fieldstg_battle_enemies[246] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[247], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[248], &fieldstg_battle_enemies[249], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[250], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[251], &fieldstg_battle_enemies[252], &fieldstg_battle_enemies[253] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[254], &fieldstg_battle_enemies[255], &fieldstg_battle_enemies[256] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[257], &fieldstg_battle_enemies[258], &fieldstg_battle_enemies[259] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[260], &fieldstg_battle_enemies[261], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[262], &fieldstg_battle_enemies[263], &fieldstg_battle_enemies[264] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[265], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[266], &fieldstg_battle_enemies[267], &fieldstg_battle_enemies[268] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[269], &fieldstg_battle_enemies[270], &fieldstg_battle_enemies[271] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[272], &fieldstg_battle_enemies[273], &fieldstg_battle_enemies[274] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[275], &fieldstg_battle_enemies[276], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[277], &fieldstg_battle_enemies[278], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[279], &fieldstg_battle_enemies[280], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[281], &fieldstg_battle_enemies[282], &fieldstg_battle_enemies[283] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[284], &fieldstg_battle_enemies[285], &fieldstg_battle_enemies[286] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[287], &fieldstg_battle_enemies[288], &fieldstg_battle_enemies[289] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[290], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[291], &fieldstg_battle_enemies[292], &fieldstg_battle_enemies[293] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[294], &fieldstg_battle_enemies[295], &fieldstg_battle_enemies[296] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[297], &fieldstg_battle_enemies[298], &fieldstg_battle_enemies[299] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[300], &fieldstg_battle_enemies[301], &fieldstg_battle_enemies[302] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[303], &fieldstg_battle_enemies[304], &fieldstg_battle_enemies[305] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[306], &fieldstg_battle_enemies[307], &fieldstg_battle_enemies[308] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[309], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[310], &fieldstg_battle_enemies[311], &fieldstg_battle_enemies[312] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[313], &fieldstg_battle_enemies[314], &fieldstg_battle_enemies[315] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[316], &fieldstg_battle_enemies[317], &fieldstg_battle_enemies[318] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[319], &fieldstg_battle_enemies[320], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[321], &fieldstg_battle_enemies[322], &fieldstg_battle_enemies[323] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[324], &fieldstg_battle_enemies[325], &fieldstg_battle_enemies[326] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[327], &fieldstg_battle_enemies[328], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[329], &fieldstg_battle_enemies[330], &fieldstg_battle_enemies[331] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[332], &fieldstg_battle_enemies[333], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[334], &fieldstg_battle_enemies[335], &fieldstg_battle_enemies[336] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[337], &fieldstg_battle_enemies[338], &fieldstg_battle_enemies[339] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[340], &fieldstg_battle_enemies[341], &fieldstg_battle_enemies[342] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[343], &fieldstg_battle_enemies[344], &fieldstg_battle_enemies[345] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[346], &fieldstg_battle_enemies[347], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[348], &fieldstg_battle_enemies[349], &fieldstg_battle_enemies[350] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[351], &fieldstg_battle_enemies[352], &fieldstg_battle_enemies[353] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[354], &fieldstg_battle_enemies[355], &fieldstg_battle_enemies[356] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[357], &fieldstg_battle_enemies[358], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[359], &fieldstg_battle_enemies[360], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[361], &fieldstg_battle_enemies[362], &fieldstg_battle_enemies[363] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[364], &fieldstg_battle_enemies[365], &fieldstg_battle_enemies[366] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[367], &fieldstg_battle_enemies[368], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[369], &fieldstg_battle_enemies[370], &fieldstg_battle_enemies[371] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[372], &fieldstg_battle_enemies[373], &fieldstg_battle_enemies[374] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[375], &fieldstg_battle_enemies[376], &fieldstg_battle_enemies[377] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[378], &fieldstg_battle_enemies[379], &fieldstg_battle_enemies[380] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[381], &fieldstg_battle_enemies[382], &fieldstg_battle_enemies[383] }, 0, 3, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[384], &fieldstg_battle_enemies[385], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[386], &fieldstg_battle_enemies[387], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[388], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[389], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[390], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[391], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[392], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[393], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[394], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[395], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[396], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 4, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[397], &fieldstg_battle_enemies[398], &fieldstg_battle_enemies[399] }, 0, 4, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[400], &fieldstg_battle_enemies[401], &fieldstg_battle_enemies[402] }, 0, 2, { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[403], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[404], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[405], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[406], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[407], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[408], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[409], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[410], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[411], &fieldstg_battle_enemies[412], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[413], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[414], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[415], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[416], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[417], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 1, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[418], &fieldstg_battle_enemies[419], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[420], &fieldstg_battle_enemies[421], &fieldstg_battle_enemies[422] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[423], &fieldstg_battle_enemies[424], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[425], &fieldstg_battle_enemies[426], &fieldstg_battle_enemies[427] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[428], &fieldstg_battle_enemies[429], &fieldstg_battle_enemies[430] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[431], &fieldstg_battle_enemies[432], &fieldstg_battle_enemies[433] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[434], &fieldstg_battle_enemies[435], &fieldstg_battle_enemies[436] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[437], &fieldstg_battle_enemies[438], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[439], &fieldstg_battle_enemies[440], &fieldstg_battle_enemies[441] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[442], &fieldstg_battle_enemies[443], &fieldstg_battle_enemies[444] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[445], &fieldstg_battle_enemies[446], &fieldstg_battle_enemies[447] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[448], &fieldstg_battle_enemies[449], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[450], &fieldstg_battle_enemies[451], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[452], &fieldstg_battle_enemies[453], &fieldstg_battle_enemies[454] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[455], &fieldstg_battle_enemies[456], &fieldstg_battle_enemies[457] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[458], &fieldstg_battle_enemies[459], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[460], &fieldstg_battle_enemies[461], &fieldstg_battle_enemies[462] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[463], &fieldstg_battle_enemies[464], &fieldstg_battle_enemies[465] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[466], &fieldstg_battle_enemies[467], &fieldstg_battle_enemies[468] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[469], &fieldstg_battle_enemies[470], &fieldstg_battle_enemies[471] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[472], &fieldstg_battle_enemies[473], &fieldstg_battle_enemies[474] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[475], &fieldstg_battle_enemies[476], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[477], &fieldstg_battle_enemies[478], &fieldstg_battle_enemies[0] }, 0, 3, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 } },
    { { &fieldstg_battle_enemies[479], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[480], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[481], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[482], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[483], &fieldstg_battle_enemies[484], &fieldstg_battle_enemies[485] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[486], &fieldstg_battle_enemies[487], &fieldstg_battle_enemies[488] }, 0x7F, 2, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[489], &fieldstg_battle_enemies[490], &fieldstg_battle_enemies[491] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[492], &fieldstg_battle_enemies[493], &fieldstg_battle_enemies[494] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[495], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[496], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[497], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[498], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[499], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[500], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[501], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[502], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[503], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[504], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[505], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[506], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[507], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[508], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 3, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[509], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[510], &fieldstg_battle_enemies[511], &fieldstg_battle_enemies[512] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[513], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[514], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0, 2, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 } },
    { { &fieldstg_battle_enemies[515], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[516], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 8, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[517], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[518], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x10, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[519], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x18, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[520], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x18, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[521], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
    { { &fieldstg_battle_enemies[522], &fieldstg_battle_enemies[0], &fieldstg_battle_enemies[0] }, 0x20, 5, { 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 } },
};

extern s16 fieldstg_effects_voice; /* voice of the last looping sound */

/* The effects object (script object 813, created by fieldstg_effects_start): the event scripts' messages to it
 * play sounds, show or hide map sprites, shake the screen and open marked spots (fieldstg_effects_message). */
void fieldstg_effects_update(Object *obj) {
    switch (obj->state) {
    case OBJECT_STATE_DONE:
    case OBJECT_STATE_END:
        break;
    case OBJECT_STATE_INIT:
    case OBJECT_STATE_RUN:
    default:
        if (obj->step == 1) {
            fieldstg_partners_hide();
            obj->set_step(obj, 0);
        }
        break;
    }
}

void fieldstg_effects_message(Object *obj, s32 cmd) {
    Object *target;
    FieldstgSprite *entry;
    s32 n;

    if (obj == NULL) {
        return;
    }
    n = 0;
    if (cmd == 0x337) {
        obj->set_step(obj, 1);
    }
    switch (cmd) {
    case 0x34A:
        n = 1;
        /* fallthrough */
    case 0x339:
        n++;
        /* fallthrough */
    case 0x338:
        n++;
        target = heap_objects.find(0x16, -1, -1);
        target->set_step(target, n);
        break;
    }
    switch (cmd) {
    case 0x34D:
    case 0x34E:
    case 0x34F:
    case 0x350:
    case 0x351:
    case 0x352:
        for (entry = fieldstg_stage.sprites; entry->present != 0; entry++) {
            if (entry->type == cmd - 0x2E9) {
                entry->shown = 0;
            }
        }
        break;
    case 0x353:
    case 0x354:
    case 0x355:
    case 0x356:
    case 0x357:
    case 0x358:
        for (entry = fieldstg_stage.sprites; entry->present != 0; entry++) {
            if (entry->type == cmd - 0x2EF) {
                entry->shown = 1;
            }
        }
        break;
    }
    switch (cmd) {
    case 0x372:
        fieldstg_camera_set_shake(1);
        break;
    case 0x373:
        fieldstg_camera_set_shake(0);
        break;
    }
    if (cmd == 0x376) {
        fieldstg_spots_open_scripted();
    }
    switch (cmd) {
    case 0x365:
        sound_module.play(0xB80001);
        break;
    case 0x368:
        sound_module.play(0x80E8383C);
        break;
    case 0x369:
        sound_module.play(0x60040002);
        break;
    case 0x36A:
        sound_module.play(0xA40006);
        break;
    case 0x36B:
        sound_module.play(0x805458BD);
        break;
    case 0x36C:
        sound_module.play(0x800410BD);
        break;
    case 0x36D:
        sound_module.play(0x803C503C);
        break;
    case 0x36E:
        sound_module.play(0x01100000);
        break;
    case 0x36F:
        sound_module.play(0x01100002);
        break;
    case 0x374:
        sound_module.play(0x700001);
        break;
    case 0x375:
        sound_module.play(0x40015);
        break;
    case 0x377:
        sound_module.play(0x8004113E);
        break;
    case 0x378:
        sound_module.play(0x8110303C);
        break;
    case 0x379:
        sound_module.play(0x81103240);
        break;
    case 0x37A:
        sound_module.play(0x4001D);
        break;
    case 0x37C:
        sound_module.play(0x440001);
        break;
    case 0x37D:
        sound_module.play(0x340004);
        break;
    case 0x37E:
        sound_module.play(0x40013);
        break;
    case 0x37F:
        sound_module.play(0x800429BF);
        break;
    case 0x380:
        sound_module.play(0x800430BD);
        break;
    case 0x381:
        sound_module.play(0x80042DC7);
        break;
    case 0x383:
        sound_module.play(0x8004103C);
        break;
    }
    switch (cmd) {
    case 0x366:
        fieldstg_effects_voice = sound_module.play(0xA10C703C);
        break;
    case 0x370:
        fieldstg_effects_voice = sound_module.play(0xA0045EC9);
        break;
    case 0x382:
        fieldstg_effects_voice = sound_module.play(0xA054583C);
        break;
    case 0x384:
        fieldstg_effects_voice = sound_module.play(0xA0042FCB);
        break;
    }
    switch (cmd) {
    case 0x367:
        sound_module.key_off(0xA10C703C, fieldstg_effects_voice);
        break;
    case 0x371:
        sound_module.key_off(0xA0045EC9, fieldstg_effects_voice);
        break;
    case 0x385:
        sound_module.key_off(0xA0042FCB, fieldstg_effects_voice);
        break;
    case 0x386:
        sound_module.key_off(0xA054583C, fieldstg_effects_voice);
        break;
    }
}

OBJECT_V0(Object *) fieldstg_effects_start(void) {
    /* PC_PORT: FINDINGS 8: callers use the object (v0) */
    OBJECT_V0_TAIL(object_create(fieldstg_effects_update, sizeof(Object), 0, 0x32D))
}
