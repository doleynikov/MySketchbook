/*
 * PCD8544 - Interface with Philips PCD8544 (or compatible) LCDs.
 *
 * Copyright (c) 2010 Carlos Rodrigues <cefrodrigues@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include <avr/pgmspace.h>


// The 7-bit ASCII character set...
const PROGMEM unsigned char charset[][5] = {
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //00    0
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //01    1
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //02    2
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //03    3
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //04    4
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //05    5
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //06    6
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //07    7
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //08    8
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //09    9
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //0A    10
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //0B    11
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //0C    12
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //0D    13
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //0E    14
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //0F    15
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //10    16
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //11    17
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //12    18
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //13    19
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //14    20
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //15    21
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //16    22
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //17    23
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //18    24
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //19    25
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //1A    26
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //1B    27
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //1C    28
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //1D    29
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //1E    30
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //1F    31
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //20    32
{  0x00, 0x00, 0x5F, 0x00, 0x00 },   //21    33
{  0x00, 0x07, 0x00, 0x07, 0x00 },   //22    34
{  0x14, 0x7F, 0x14, 0x7F, 0x14 },   //23    35
{  0x24, 0x2A, 0x6B, 0x2A, 0x12 },   //24    36
{  0x23, 0x13, 0x08, 0x64, 0x62 },   //25    37
{  0x36, 0x49, 0x56, 0x20, 0x50 },   //26    38
{  0x00, 0x00, 0x07, 0x00, 0x00 },   //27    39
{  0x00, 0x1C, 0x22, 0x41, 0x00 },   //28    40
{  0x00, 0x41, 0x22, 0x1C, 0x00 },   //29    41
{  0x0A, 0x04, 0x0E, 0x04, 0x0A },   //2A    42
{  0x08, 0x08, 0x3E, 0x08, 0x08 },   //2B    43
{  0x00, 0x40, 0x30, 0x00, 0x00 },   //2C    44
{  0x08, 0x08, 0x08, 0x08, 0x08 },   //2D    45
{  0x00, 0x60, 0x60, 0x00, 0x00 },   //2E    46
{  0x40, 0x30, 0x08, 0x06, 0x01 },   //2F    47
{  0x3E, 0x51, 0x49, 0x45, 0x3E },   //30    48
{  0x00, 0x42, 0x7F, 0x40, 0x00 },   //31    49
{  0x72, 0x49, 0x49, 0x49, 0x46 },   //32    50
{  0x21, 0x41, 0x49, 0x4D, 0x33 },   //33    51
{  0x18, 0x14, 0x12, 0x7F, 0x10 },   //34    52
{  0x27, 0x45, 0x45, 0x45, 0x39 },   //35    53
{  0x3E, 0x49, 0x49, 0x49, 0x32 },   //36    54
{  0x41, 0x21, 0x11, 0x09, 0x07 },   //37    55
{  0x36, 0x49, 0x49, 0x49, 0x36 },   //38    56
{  0x26, 0x49, 0x49, 0x49, 0x3E },   //39    57
{  0x00, 0x00, 0x14, 0x00, 0x00 },   //3A    58
{  0x00, 0x40, 0x34, 0x00, 0x00 },   //3B    59
{  0x08, 0x14, 0x22, 0x41, 0x00 },   //3C    60
{  0x14, 0x14, 0x14, 0x14, 0x14 },   //3D    61
{  0x00, 0x41, 0x22, 0x14, 0x08 },   //3E    62
{  0x02, 0x01, 0x59, 0x09, 0x06 },   //3F    63
{  0x3E, 0x41, 0x5D, 0x49, 0x26 },   //40    64
{  0x7C, 0x12, 0x11, 0x12, 0x7C },   //41    65
{  0x7F, 0x49, 0x49, 0x49, 0x36 },   //42    66
{  0x3E, 0x41, 0x41, 0x41, 0x22 },   //43    67
{  0x7F, 0x41, 0x41, 0x41, 0x3E },   //44    68
{  0x7F, 0x49, 0x49, 0x49, 0x41 },   //45    69
{  0x7F, 0x09, 0x09, 0x01, 0x03 },   //46    70
{  0x3E, 0x41, 0x41, 0x51, 0x73 },   //47    71
{  0x7F, 0x08, 0x08, 0x08, 0x7F },   //48    72
{  0x00, 0x41, 0x7F, 0x41, 0x00 },   //49    73
{  0x20, 0x40, 0x41, 0x3F, 0x01 },   //4A    74
{  0x7F, 0x08, 0x14, 0x22, 0x41 },   //4B    75
{  0x7F, 0x40, 0x40, 0x40, 0x60 },   //4C    76
{  0x7F, 0x02, 0x1C, 0x02, 0x7F },   //4D    77
{  0x7F, 0x04, 0x08, 0x10, 0x7F },   //4E    78
{  0x3E, 0x41, 0x41, 0x41, 0x3E },   //4F    79
{  0x7F, 0x09, 0x09, 0x09, 0x06 },   //50    80
{  0x3E, 0x41, 0x51, 0x21, 0x5E },   //51    81
{  0x7F, 0x09, 0x19, 0x29, 0x46 },   //52    82
{  0x26, 0x49, 0x49, 0x49, 0x32 },   //53    83
{  0x03, 0x01, 0x7F, 0x01, 0x03 },   //54    84
{  0x3F, 0x40, 0x40, 0x40, 0x3F },   //55    85
{  0x1F, 0x20, 0x40, 0x20, 0x1F },   //56    86
{  0x3F, 0x40, 0x38, 0x40, 0x3F },   //57    87
{  0x63, 0x14, 0x08, 0x14, 0x63 },   //58    88
{  0x03, 0x04, 0x78, 0x04, 0x03 },   //59    89
{  0x63, 0x51, 0x49, 0x45, 0x63 },   //5A    90
{  0x00, 0x7F, 0x41, 0x41, 0x00 },   //5B    91
{  0x01, 0x06, 0x08, 0x30, 0x40 },   //5C    92
{  0x00, 0x41, 0x41, 0x7F, 0x00 },   //5D    93
{  0x04, 0x02, 0x01, 0x02, 0x04 },   //5E    94
{  0x40, 0x40, 0x40, 0x40, 0x40 },   //5F    95
{  0x00, 0x01, 0x02, 0x04, 0x00 },   //60    96
{  0x20, 0x54, 0x54, 0x78, 0x40 },   //61    97
{  0x7F, 0x28, 0x44, 0x44, 0x38 },   //62    98
{  0x38, 0x44, 0x44, 0x44, 0x28 },   //63    99
{  0x38, 0x44, 0x44, 0x28, 0x7F },   //64    100
{  0x38, 0x54, 0x54, 0x54, 0x18 },   //65    101
{  0x00, 0x08, 0x7E, 0x09, 0x02 },   //66    102
{  0x18, 0xA4, 0xA4, 0x9C, 0x78 },   //67    103
{  0x7F, 0x08, 0x04, 0x04, 0x78 },   //68    104
{  0x00, 0x44, 0x7D, 0x40, 0x00 },   //69    105
{  0x20, 0x40, 0x40, 0x3D, 0x00 },   //6A    106
{  0x7F, 0x10, 0x28, 0x44, 0x00 },   //6B    107
{  0x00, 0x41, 0x7F, 0x40, 0x00 },   //6C    108
{  0x7C, 0x04, 0x78, 0x04, 0x78 },   //6D    109
{  0x7C, 0x08, 0x04, 0x04, 0x78 },   //6E    110
{  0x38, 0x44, 0x44, 0x44, 0x38 },   //6F    111
{  0xFC, 0x18, 0x24, 0x24, 0x18 },   //70    112
{  0x18, 0x24, 0x24, 0x18, 0xFC },   //71    113
{  0x7C, 0x08, 0x04, 0x04, 0x08 },   //72    114
{  0x48, 0x54, 0x54, 0x54, 0x24 },   //73    115
{  0x04, 0x04, 0x3F, 0x44, 0x24 },   //74    116
{  0x3C, 0x40, 0x40, 0x20, 0x7C },   //75    117
{  0x1C, 0x20, 0x40, 0x20, 0x1C },   //76    118
{  0x3C, 0x40, 0x30, 0x40, 0x3C },   //77    119
{  0x44, 0x28, 0x10, 0x28, 0x44 },   //78    120
{  0x4C, 0x90, 0x90, 0x90, 0x7C },   //79    121
{  0x44, 0x64, 0x54, 0x4C, 0x44 },   //7A    122
{  0x00, 0x08, 0x36, 0x41, 0x00 },   //7B    123
{  0x00, 0x00, 0x77, 0x00, 0x00 },   //7C    124
{  0x00, 0x41, 0x36, 0x08, 0x00 },   //7D    125
{  0x08, 0x04, 0x08, 0x10, 0x08 },   //7E    126
{  0x3C, 0x26, 0x23, 0x26, 0x3C },   //7F    127
{  0x7F, 0x11, 0x09, 0x48, 0x30 },   //80    128
{  0x7C, 0x04, 0x05, 0x04, 0x00 },   //81    129
{  0x00, 0x40, 0x30, 0x00, 0x00 },   //82    130
{  0x00, 0x78, 0x0A, 0x09, 0x00 },   //83    131
{  0x40, 0x30, 0x00, 0x40, 0x30 },   //84    132
{  0x40, 0x00, 0x40, 0x00, 0x40 },   //85    133
{  0x00, 0x02, 0x7F, 0x02, 0x00 },   //86    134
{  0x00, 0x22, 0x7F, 0x22, 0x00 },   //87    135
{  0x3E, 0x55, 0x55, 0x41, 0x22 },   //88    136
{  0x13, 0x08, 0x64, 0x02, 0x60 },   //89    137
{  0x7C, 0x02, 0x7F, 0x48, 0x30 },   //8A    138
{  0x00, 0x08, 0x14, 0x00, 0x00 },   //8B    139
{  0x7F, 0x08, 0x7F, 0x48, 0x30 },   //8C    140
{  0x7E, 0x18, 0x19, 0x24, 0x42 },   //8D    141
{  0x7F, 0x11, 0x09, 0x08, 0x70 },   //8E    142
{  0x7F, 0x40, 0xC0, 0x40, 0x7F },   //8F    143
{  0x7F, 0x12, 0x8A, 0x70, 0x00 },   //90    144
{  0x00, 0x00, 0x06, 0x01, 0x00 },   //91    145
{  0x00, 0x04, 0x03, 0x00, 0x00 },   //92    146
{  0x06, 0x01, 0x00, 0x06, 0x01 },   //93    147
{  0x04, 0x03, 0x00, 0x04, 0x03 },   //94    148
{  0x00, 0x0C, 0x0C, 0x00, 0x00 },   //95    149
{  0x00, 0x08, 0x08, 0x08, 0x00 },   //96    150
{  0x08, 0x08, 0x08, 0x08, 0x08 },   //97    151
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //98    152
{  0x78, 0x11, 0x27, 0x11, 0x78 },   //99    153
{  0x70, 0x08, 0x7C, 0x50, 0x20 },   //9A    154
{  0x00, 0x00, 0x14, 0x08, 0x00 },   //9B    155
{  0x7C, 0x10, 0x7C, 0x50, 0x20 },   //9C    156
{  0x7C, 0x10, 0x12, 0x29, 0x44 },   //9D    157
{  0x7F, 0x12, 0x0A, 0x70, 0x00 },   //9E    158
{  0x7C, 0x40, 0xC0, 0x40, 0x7C },   //9F    159
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //A0    160
{  0x23, 0x44, 0x39, 0x04, 0x03 },   //A1    161
{  0x24, 0x49, 0x32, 0x09, 0x04 },   //A2    162
{  0x20, 0x41, 0x3F, 0x01, 0x00 },   //A3    163
{  0x5D, 0x22, 0x22, 0x22, 0x5D },   //A4    164
{  0x7E, 0x02, 0x02, 0x02, 0x01 },   //A5    165
{  0x00, 0x00, 0x77, 0x00, 0x00 },   //A6    166
{  0x00, 0x4A, 0x55, 0x29, 0x00 },   //A7    167
{  0x7C, 0x55, 0x54, 0x45, 0x00 },   //A8    168
{  0x3E, 0x7F, 0x63, 0x77, 0x3E },   //A9    169
{  0x3E, 0x49, 0x49, 0x41, 0x22 },   //AA    170
{  0x08, 0x14, 0x00, 0x08, 0x14 },   //AB    171
{  0x08, 0x08, 0x08, 0x08, 0x18 },   //AC    172
{  0x00, 0x00, 0x00, 0x00, 0x00 },   //AD    173
{  0x3E, 0x5F, 0x4B, 0x55, 0x3E },   //AE    174
{  0x00, 0x45, 0x7C, 0x45, 0x00 },   //AF    175
{  0x00, 0x04, 0x0A, 0x04, 0x00 },   //B0    176
{  0x44, 0x44, 0x5F, 0x44, 0x44 },   //B1    177
{  0x00, 0x41, 0x7F, 0x41, 0x00 },   //B2    178
{  0x00, 0x44, 0x7D, 0x40, 0x00 },   //B3    179
{  0x7C, 0x04, 0x04, 0x02, 0x00 },   //B4    180
{  0x7C, 0x10, 0x10, 0x3C, 0x40 },   //B5    181
{  0x06, 0x0F, 0x7F, 0x01, 0x7F },   //B6    182
{  0x00, 0x00, 0x08, 0x00, 0x00 },   //B7    183
{  0x38, 0x55, 0x54, 0x55, 0x18 },   //B8    184
{  0x7C, 0x10, 0x20, 0x7C, 0x01 },   //B9    185
{  0x38, 0x54, 0x54, 0x44, 0x28 },   //BA    186
{  0x14, 0x08, 0x00, 0x14, 0x08 },   //BB    187
{  0x20, 0x40, 0x3D, 0x00, 0x00 },   //BC    188
{  0x66, 0x49, 0x49, 0x49, 0x33 },   //BD    189
{  0x08, 0x54, 0x54, 0x54, 0x20 },   //BE    190
{  0x00, 0x45, 0x7C, 0x41, 0x00 },   //BF    191
{  0x7C, 0x12, 0x11, 0x12, 0x7C },   //C0    192
{  0x7F, 0x49, 0x49, 0x49, 0x30 },   //C1    193
{  0x7F, 0x49, 0x49, 0x49, 0x36 },   //C2    194
{  0x7F, 0x01, 0x01, 0x01, 0x01 },   //C3    195
{  0xC0, 0x7E, 0x41, 0x7F, 0xC0 },   //C4    196
{  0x7F, 0x49, 0x49, 0x49, 0x41 },   //C5    197
{  0x77, 0x08, 0x7F, 0x08, 0x77 },   //C6    198
{  0x00, 0x49, 0x49, 0x49, 0x36 },   //C7    199
{  0x7F, 0x10, 0x08, 0x04, 0x7F },   //C8    200
{  0x7F, 0x10, 0x09, 0x04, 0x7F },   //C9    201
{  0x7F, 0x08, 0x14, 0x22, 0x41 },   //CA    202
{  0x7C, 0x02, 0x01, 0x01, 0x7F },   //CB    203
{  0x7F, 0x02, 0x0C, 0x02, 0x7F },   //CC    204
{  0x7F, 0x08, 0x08, 0x08, 0x7F },   //CD    205
{  0x3E, 0x41, 0x41, 0x41, 0x3E },   //CE    206
{  0x7F, 0x01, 0x01, 0x01, 0x7F },   //CF    207
{  0x7F, 0x09, 0x09, 0x09, 0x06 },   //D0    208
{  0x3E, 0x41, 0x41, 0x41, 0x22 },   //D1    209
{  0x01, 0x01, 0x7F, 0x01, 0x01 },   //D2    210
{  0x07, 0x48, 0x48, 0x48, 0x3F },   //D3    211
{  0x0E, 0x11, 0x7F, 0x11, 0x0E },   //D4    212
{  0x63, 0x14, 0x08, 0x14, 0x63 },   //D5    213
{  0x7F, 0x40, 0x40, 0x7F, 0xC0 },   //D6    214
{  0x07, 0x08, 0x08, 0x08, 0x7F },   //D7    215
{  0x7F, 0x40, 0x7F, 0x40, 0x7F },   //D8    216
{  0x7F, 0x40, 0x7F, 0x40, 0xFF },   //D9    217
{  0x01, 0x7F, 0x48, 0x48, 0x30 },   //DA    218
{  0x7F, 0x48, 0x30, 0x00, 0x7F },   //DB    219
{  0x7F, 0x48, 0x48, 0x48, 0x30 },   //DC    220
{  0x22, 0x41, 0x49, 0x49, 0x3E },   //DD    221
{  0x7F, 0x08, 0x3E, 0x41, 0x3E },   //DE    222
{  0x46, 0x29, 0x19, 0x09, 0x7F },   //DF    223
{  0x20, 0x54, 0x54, 0x54, 0x78 },   //E0    224
{  0x7C, 0x54, 0x54, 0x54, 0x20 },   //E1    225
{  0x7C, 0x54, 0x54, 0x54, 0x28 },   //E2    226
{  0x7C, 0x04, 0x04, 0x04, 0x00 },   //E3    227
{  0xC0, 0x78, 0x44, 0x7C, 0xC0 },   //E4    228
{  0x38, 0x54, 0x54, 0x54, 0x18 },   //E5    229
{  0x6C, 0x10, 0x7C, 0x10, 0x6C },   //E6    230
{  0x00, 0x54, 0x54, 0x54, 0x28 },   //E7    231
{  0x7C, 0x20, 0x10, 0x08, 0x7C },   //E8    232
{  0x7C, 0x20, 0x12, 0x08, 0x7C },   //E9    233
{  0x7C, 0x10, 0x10, 0x28, 0x44 },   //EA    234
{  0x70, 0x08, 0x04, 0x04, 0x7C },   //EB    235
{  0x7C, 0x08, 0x10, 0x08, 0x7C },   //EC    236
{  0x7C, 0x10, 0x10, 0x10, 0x7C },   //ED    237
{  0x38, 0x44, 0x44, 0x44, 0x38 },   //EE    238
{  0x7C, 0x04, 0x04, 0x04, 0x7C },   //EF    239
{  0x7C, 0x14, 0x14, 0x14, 0x08 },   //F0    240
{  0x38, 0x44, 0x44, 0x44, 0x28 },   //F1    241
{  0x04, 0x04, 0x7C, 0x04, 0x04 },   //F2    242
{  0x0C, 0x50, 0x50, 0x50, 0x3C },   //F3    243
{  0x38, 0x44, 0xFC, 0x44, 0x38 },   //F4    244
{  0x44, 0x28, 0x10, 0x28, 0x44 },   //F5    245
{  0x7C, 0x40, 0x40, 0x7C, 0xC0 },   //F6    246
{  0x0C, 0x10, 0x10, 0x10, 0x7C },   //F7    247
{  0x7C, 0x40, 0x7C, 0x40, 0x7C },   //F8    248
{  0x7C, 0x40, 0x7C, 0x40, 0xFC },   //F9    249
{  0x04, 0x7C, 0x50, 0x50, 0x20 },   //FA    250
{  0x7C, 0x50, 0x20, 0x00, 0x7C },   //FB    251
{  0x7C, 0x50, 0x50, 0x20, 0x00 },   //FC    252
{  0x28, 0x44, 0x54, 0x54, 0x38 },   //FD    253
{  0x7C, 0x10, 0x38, 0x44, 0x38 },   //FE    254
{  0x48, 0x34, 0x14, 0x14, 0x7C }    //FF    255
};

