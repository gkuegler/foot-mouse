/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2023 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <stdint.h>

// THe 'FUTURE_FUTURE_KEY_MASK' mask is used so the interface can take ascii characters as well.
#define HID_USAGE_MASK 0xF000

#ifdef __cplusplus
extern "C"{
#endif
// http://en.wikipedia.org/wiki/Keyboard_layout

// Explanation why 16 bits and the purpose of upper 8 bits of these defines
// https://forum.pjrc.com/threads/71874?p=318669&viewfull=1#post318669

#define MOUSE_LEFT   0x01
#define MOUSE_RIGHT  0x02
#define MOUSE_MIDDLE 0x04

#define MODIFIERKEY_CTRL        ( 0x01 | 0xE000 )
#define MODIFIERKEY_SHIFT       ( 0x02 | 0xE000 )
#define MODIFIERKEY_ALT         ( 0x04 | 0xE000 )
#define MODIFIERKEY_GUI         ( 0x08 | 0xE000 )
#define MODIFIERKEY_LEFT_CTRL   ( 0x01 | 0xE000 )
#define MODIFIERKEY_LEFT_SHIFT  ( 0x02 | 0xE000 )
#define MODIFIERKEY_LEFT_ALT    ( 0x04 | 0xE000 )
#define MODIFIERKEY_LEFT_GUI    ( 0x08 | 0xE000 )
#define MODIFIERKEY_RIGHT_CTRL  ( 0x10 | 0xE000 )
#define MODIFIERKEY_RIGHT_SHIFT ( 0x20 | 0xE000 )
#define MODIFIERKEY_RIGHT_ALT   ( 0x40 | 0xE000 )
#define MODIFIERKEY_RIGHT_GUI   ( 0x80 | 0xE000 )

// #define KEY_SYSTEM_POWER_DOWN   ( 0x81 | 0xE200 )
// #define KEY_SYSTEM_SLEEP        ( 0x82 | 0xE200 )
// #define KEY_SYSTEM_WAKE_UP      ( 0x83 | 0xE200 )

// #define KEY_MEDIA_PLAY          ( 0xB0 | 0xE400 )
// #define KEY_MEDIA_PAUSE         ( 0xB1 | 0xE400 )
// #define KEY_MEDIA_RECORD        ( 0xB2 | 0xE400 )
// #define KEY_MEDIA_FAST_FORWARD  ( 0xB3 | 0xE400 )
// #define KEY_MEDIA_REWIND        ( 0xB4 | 0xE400 )
// #define KEY_MEDIA_NEXT_TRACK    ( 0xB5 | 0xE400 )
// #define KEY_MEDIA_PREV_TRACK    ( 0xB6 | 0xE400 )
// #define KEY_MEDIA_STOP          ( 0xB7 | 0xE400 )
// #define KEY_MEDIA_EJECT         ( 0xB8 | 0xE400 )
// #define KEY_MEDIA_RANDOM_PLAY   ( 0xB9 | 0xE400 )
// #define KEY_MEDIA_PLAY_PAUSE    ( 0xCD | 0xE400 )
// #define KEY_MEDIA_PLAY_SKIP     ( 0xCE | 0xE400 )
// #define KEY_MEDIA_MUTE          ( 0xE2 | 0xE400 )
// #define KEY_MEDIA_VOLUME_INC    ( 0xE9 | 0xE400 )
// #define KEY_MEDIA_VOLUME_DEC    ( 0xEA | 0xE400 )

#define KEY_A                   (   4  | HID_USAGE_MASK )
#define KEY_B                   (   5  | HID_USAGE_MASK )
#define KEY_C                   (   6  | HID_USAGE_MASK )
#define KEY_D                   (   7  | HID_USAGE_MASK )
#define KEY_E                   (   8  | HID_USAGE_MASK )
#define KEY_F                   (   9  | HID_USAGE_MASK )
#define KEY_G                   (  10  | HID_USAGE_MASK )
#define KEY_H                   (  11  | HID_USAGE_MASK )
#define KEY_I                   (  12  | HID_USAGE_MASK )
#define KEY_J                   (  13  | HID_USAGE_MASK )
#define KEY_K                   (  14  | HID_USAGE_MASK )
#define KEY_L                   (  15  | HID_USAGE_MASK )
#define KEY_M                   (  16  | HID_USAGE_MASK )
#define KEY_N                   (  17  | HID_USAGE_MASK )
#define KEY_O                   (  18  | HID_USAGE_MASK )
#define KEY_P                   (  19  | HID_USAGE_MASK )
#define KEY_Q                   (  20  | HID_USAGE_MASK )
#define KEY_R                   (  21  | HID_USAGE_MASK )
#define KEY_S                   (  22  | HID_USAGE_MASK )
#define KEY_T                   (  23  | HID_USAGE_MASK )
#define KEY_U                   (  24  | HID_USAGE_MASK )
#define KEY_V                   (  25  | HID_USAGE_MASK )
#define KEY_W                   (  26  | HID_USAGE_MASK )
#define KEY_X                   (  27  | HID_USAGE_MASK )
#define KEY_Y                   (  28  | HID_USAGE_MASK )
#define KEY_Z                   (  29  | HID_USAGE_MASK )
#define KEY_1                   (  30  | HID_USAGE_MASK )
#define KEY_2                   (  31  | HID_USAGE_MASK )
#define KEY_3                   (  32  | HID_USAGE_MASK )
#define KEY_4                   (  33  | HID_USAGE_MASK )
#define KEY_5                   (  34  | HID_USAGE_MASK )
#define KEY_6                   (  35  | HID_USAGE_MASK )
#define KEY_7                   (  36  | HID_USAGE_MASK )
#define KEY_8                   (  37  | HID_USAGE_MASK )
#define KEY_9                   (  38  | HID_USAGE_MASK )
#define KEY_0                   (  39  | HID_USAGE_MASK )
#define KEY_ENTER               (  40  | HID_USAGE_MASK )
#define KEY_ESC                 (  41  | HID_USAGE_MASK )
#define KEY_BACKSPACE           (  42  | HID_USAGE_MASK )
#define KEY_TAB                 (  43  | HID_USAGE_MASK )
#define KEY_SPACE               (  44  | HID_USAGE_MASK )
#define KEY_MINUS               (  45  | HID_USAGE_MASK )
#define KEY_EQUAL               (  46  | HID_USAGE_MASK )
#define KEY_LEFT_BRACE          (  47  | HID_USAGE_MASK )
#define KEY_RIGHT_BRACE         (  48  | HID_USAGE_MASK )
#define KEY_BACKSLASH           (  49  | HID_USAGE_MASK )
#define KEY_NON_US_NUM          (  50  | HID_USAGE_MASK )
#define KEY_SEMICOLON           (  51  | HID_USAGE_MASK )
#define KEY_QUOTE               (  52  | HID_USAGE_MASK )
#define KEY_TILDE               (  53  | HID_USAGE_MASK )
#define KEY_COMMA               (  54  | HID_USAGE_MASK )
#define KEY_PERIOD              (  55  | HID_USAGE_MASK )
#define KEY_SLASH               (  56  | HID_USAGE_MASK )
#define KEY_CAPS_LOCK           (  57  | HID_USAGE_MASK )
#define KEY_F1                  (  58  | HID_USAGE_MASK )
#define KEY_F2                  (  59  | HID_USAGE_MASK )
#define KEY_F3                  (  60  | HID_USAGE_MASK )
#define KEY_F4                  (  61  | HID_USAGE_MASK )
#define KEY_F5                  (  62  | HID_USAGE_MASK )
#define KEY_F6                  (  63  | HID_USAGE_MASK )
#define KEY_F7                  (  64  | HID_USAGE_MASK )
#define KEY_F8                  (  65  | HID_USAGE_MASK )
#define KEY_F9                  (  66  | HID_USAGE_MASK )
#define KEY_F10                 (  67  | HID_USAGE_MASK )
#define KEY_F11                 (  68  | HID_USAGE_MASK )
#define KEY_F12                 (  69  | HID_USAGE_MASK )
#define KEY_PRINTSCREEN         (  70  | HID_USAGE_MASK )
#define KEY_SCROLL_LOCK         (  71  | HID_USAGE_MASK )
#define KEY_PAUSE               (  72  | HID_USAGE_MASK )
#define KEY_INSERT              (  73  | HID_USAGE_MASK )
#define KEY_HOME                (  74  | HID_USAGE_MASK )
#define KEY_PAGE_UP             (  75  | HID_USAGE_MASK )
#define KEY_DELETE              (  76  | HID_USAGE_MASK )
#define KEY_END                 (  77  | HID_USAGE_MASK )
#define KEY_PAGE_DOWN           (  78  | HID_USAGE_MASK )
#define KEY_RIGHT               (  79  | HID_USAGE_MASK )
#define KEY_LEFT                (  80  | HID_USAGE_MASK )
#define KEY_DOWN                (  81  | HID_USAGE_MASK )
#define KEY_UP                  (  82  | HID_USAGE_MASK )
#define KEY_NUM_LOCK            (  83  | HID_USAGE_MASK )
#define KEYPAD_SLASH            (  84  | HID_USAGE_MASK )
#define KEYPAD_ASTERIX          (  85  | HID_USAGE_MASK )
#define KEYPAD_MINUS            (  86  | HID_USAGE_MASK )
#define KEYPAD_PLUS             (  87  | HID_USAGE_MASK )
#define KEYPAD_ENTER            (  88  | HID_USAGE_MASK )
#define KEYPAD_1                (  89  | HID_USAGE_MASK )
#define KEYPAD_2                (  90  | HID_USAGE_MASK )
#define KEYPAD_3                (  91  | HID_USAGE_MASK )
#define KEYPAD_4                (  92  | HID_USAGE_MASK )
#define KEYPAD_5                (  93  | HID_USAGE_MASK )
#define KEYPAD_6                (  94  | HID_USAGE_MASK )
#define KEYPAD_7                (  95  | HID_USAGE_MASK )
#define KEYPAD_8                (  96  | HID_USAGE_MASK )
#define KEYPAD_9                (  97  | HID_USAGE_MASK )
#define KEYPAD_0                (  98  | HID_USAGE_MASK )
#define KEYPAD_PERIOD           (  99  | HID_USAGE_MASK )
#define KEY_NON_US_BS           ( 100  | HID_USAGE_MASK )
#define KEY_MENU               	( 101  | HID_USAGE_MASK )
#define KEY_F13                 ( 104  | HID_USAGE_MASK )
#define KEY_F14                 ( 105  | HID_USAGE_MASK )
#define KEY_F15                 ( 106  | HID_USAGE_MASK )
#define KEY_F16                 ( 107  | HID_USAGE_MASK )
#define KEY_F17                 ( 108  | HID_USAGE_MASK )
#define KEY_F18                 ( 109  | HID_USAGE_MASK )
#define KEY_F19                 ( 110  | HID_USAGE_MASK )
#define KEY_F20                 ( 111  | HID_USAGE_MASK )
#define KEY_F21                 ( 112  | HID_USAGE_MASK )
#define KEY_F22                 ( 113  | HID_USAGE_MASK )
#define KEY_F23                 ( 114  | HID_USAGE_MASK )
#define KEY_F24                 ( 115  | HID_USAGE_MASK )

// for compatibility with Leonardo's slightly different names
#define KEY_UP_ARROW	KEY_UP
#define KEY_DOWN_ARROW	KEY_DOWN
#define KEY_LEFT_ARROW	KEY_LEFT
#define KEY_RIGHT_ARROW	KEY_RIGHT
#define KEY_RETURN	KEY_ENTER
#define KEY_LEFT_CTRL	MODIFIERKEY_LEFT_CTRL
#define KEY_LEFT_SHIFT	MODIFIERKEY_LEFT_SHIFT
#define KEY_LEFT_ALT	MODIFIERKEY_LEFT_ALT
#define KEY_LEFT_GUI	MODIFIERKEY_LEFT_GUI
#define KEY_RIGHT_CTRL	MODIFIERKEY_RIGHT_CTRL
#define KEY_RIGHT_SHIFT	MODIFIERKEY_RIGHT_SHIFT
#define KEY_RIGHT_ALT	MODIFIERKEY_RIGHT_ALT
#define KEY_RIGHT_GUI	MODIFIERKEY_RIGHT_GUI


// Everything below this line is not intended for use in "normal" programs.
// These private symbols populate lookup tables, which are used to translate
// ascii and UTF8 unicode into keystroke sequences.

#define LAYOUT_US_ENGLISH
#ifdef LAYOUT_US_ENGLISH

#define SHIFT_MASK		0x40
#define KEYCODE_TYPE		uint8_t
#define KEYCODE_MASK		0x007F

#define ASCII_20	KEY_SPACE				// 32  
#define ASCII_21	KEY_1 + SHIFT_MASK			// 33 !
#define ASCII_22	KEY_QUOTE + SHIFT_MASK			// 34 "
#define ASCII_23	KEY_3 + SHIFT_MASK			// 35 #
#define ASCII_24	KEY_4 + SHIFT_MASK			// 36 $
#define ASCII_25	KEY_5 + SHIFT_MASK			// 37 %
#define ASCII_26	KEY_7 + SHIFT_MASK			// 38 &
#define ASCII_27	KEY_QUOTE				// 39 '
#define ASCII_28	KEY_9 + SHIFT_MASK			// 40 (
#define ASCII_29	KEY_0 + SHIFT_MASK			// 41 )
#define ASCII_2A	KEY_8 + SHIFT_MASK			// 42 *
#define ASCII_2B	KEY_EQUAL + SHIFT_MASK			// 43 +
#define ASCII_2C	KEY_COMMA				// 44 ,
#define ASCII_2D	KEY_MINUS				// 45 -
#define ASCII_2E	KEY_PERIOD				// 46 .
#define ASCII_2F	KEY_SLASH				// 47 /
#define ASCII_30	KEY_0					// 48 0
#define ASCII_31	KEY_1					// 49 1
#define ASCII_32	KEY_2					// 50 2
#define ASCII_33	KEY_3					// 51 3
#define ASCII_34	KEY_4					// 52 4
#define ASCII_35	KEY_5					// 53 5
#define ASCII_36	KEY_6					// 54 6
#define ASCII_37	KEY_7					// 55 7
#define ASCII_38	KEY_8					// 55 8
#define ASCII_39	KEY_9					// 57 9
#define ASCII_3A	KEY_SEMICOLON + SHIFT_MASK		// 58 :
#define ASCII_3B	KEY_SEMICOLON				// 59 ;
#define ASCII_3C	KEY_COMMA + SHIFT_MASK			// 60 <
#define ASCII_3D	KEY_EQUAL				// 61 =
#define ASCII_3E	KEY_PERIOD + SHIFT_MASK			// 62 >
#define ASCII_3F	KEY_SLASH + SHIFT_MASK			// 63 ?
#define ASCII_40	KEY_2 + SHIFT_MASK			// 64 @
#define ASCII_41	KEY_A + SHIFT_MASK			// 65 A
#define ASCII_42	KEY_B + SHIFT_MASK			// 66 B
#define ASCII_43	KEY_C + SHIFT_MASK			// 67 C
#define ASCII_44	KEY_D + SHIFT_MASK			// 68 D
#define ASCII_45	KEY_E + SHIFT_MASK			// 69 E
#define ASCII_46	KEY_F + SHIFT_MASK			// 70 F
#define ASCII_47	KEY_G + SHIFT_MASK			// 71 G
#define ASCII_48	KEY_H + SHIFT_MASK			// 72 H
#define ASCII_49	KEY_I + SHIFT_MASK			// 73 I
#define ASCII_4A	KEY_J + SHIFT_MASK			// 74 J
#define ASCII_4B	KEY_K + SHIFT_MASK			// 75 K
#define ASCII_4C	KEY_L + SHIFT_MASK			// 76 L
#define ASCII_4D	KEY_M + SHIFT_MASK			// 77 M
#define ASCII_4E	KEY_N + SHIFT_MASK			// 78 N
#define ASCII_4F	KEY_O + SHIFT_MASK			// 79 O
#define ASCII_50	KEY_P + SHIFT_MASK			// 80 P
#define ASCII_51	KEY_Q + SHIFT_MASK			// 81 Q
#define ASCII_52	KEY_R + SHIFT_MASK			// 82 R
#define ASCII_53	KEY_S + SHIFT_MASK			// 83 S
#define ASCII_54	KEY_T + SHIFT_MASK			// 84 T
#define ASCII_55	KEY_U + SHIFT_MASK			// 85 U
#define ASCII_56	KEY_V + SHIFT_MASK			// 86 V
#define ASCII_57	KEY_W + SHIFT_MASK			// 87 W
#define ASCII_58	KEY_X + SHIFT_MASK			// 88 X
#define ASCII_59	KEY_Y + SHIFT_MASK			// 89 Y
#define ASCII_5A	KEY_Z + SHIFT_MASK			// 90 Z
#define ASCII_5B	KEY_LEFT_BRACE				// 91 [
#define ASCII_5C	KEY_BACKSLASH				// 92
#define ASCII_5D	KEY_RIGHT_BRACE				// 93 ]
#define ASCII_5E	KEY_6 + SHIFT_MASK			// 94 ^
#define ASCII_5F	KEY_MINUS + SHIFT_MASK			// 95 _
#define ASCII_60	KEY_TILDE				// 96 `
#define ASCII_61	KEY_A					// 97 a
#define ASCII_62	KEY_B					// 98 b
#define ASCII_63	KEY_C					// 99 c
#define ASCII_64	KEY_D					// 100 d
#define ASCII_65	KEY_E					// 101 e
#define ASCII_66	KEY_F					// 102 f
#define ASCII_67	KEY_G					// 103 g
#define ASCII_68	KEY_H					// 104 h
#define ASCII_69	KEY_I					// 105 i
#define ASCII_6A	KEY_J					// 106 j
#define ASCII_6B	KEY_K					// 107 k
#define ASCII_6C	KEY_L					// 108 l
#define ASCII_6D	KEY_M					// 109 m
#define ASCII_6E	KEY_N					// 110 n
#define ASCII_6F	KEY_O					// 111 o
#define ASCII_70	KEY_P					// 112 p
#define ASCII_71	KEY_Q					// 113 q
#define ASCII_72	KEY_R					// 114 r
#define ASCII_73	KEY_S					// 115 s
#define ASCII_74	KEY_T					// 116 t
#define ASCII_75	KEY_U					// 117 u
#define ASCII_76	KEY_V					// 118 v
#define ASCII_77	KEY_W					// 119 w
#define ASCII_78	KEY_X					// 120 x
#define ASCII_79	KEY_Y					// 121 y
#define ASCII_7A	KEY_Z					// 122 z
#define ASCII_7B	KEY_LEFT_BRACE + SHIFT_MASK		// 123 {
#define ASCII_7C	KEY_BACKSLASH + SHIFT_MASK		// 124 |
#define ASCII_7D	KEY_RIGHT_BRACE + SHIFT_MASK		// 125 }
#define ASCII_7E	KEY_TILDE + SHIFT_MASK			// 126 ~
#define ASCII_7F	KEY_BACKSPACE				// 127

#endif // LAYOUT_US_ENGLISH

// extern const KEYCODE_TYPE keycodes_ascii[];

#ifdef __cplusplus
} // extern "C"
#endif
