/**
 * @file display_7seg_codes.h
 * Коды символов для 7-сегментных индикаторов.
 */

#ifndef DISPLAY_7SEG_CODES_H
#define	DISPLAY_7SEG_CODES_H

#include <stdint.h>

//Тип символа, отобрадаемого на индикаторе.
typedef uint8_t value7seg_t;

#define DPY7_CODE_ALL    0b11111111
#define DPY7_CODE_NONE   0b00000000
#define DPY7_CODE_WS     0b00000000
#define DPY7_CODE_SPACE  0b00000000

#define DPY7_CODE_MINUS  0b00000010

#define DPY7_CODE_UNDERLINE  0b00010000

#define DPY7_CODE_CELSIUM 0b11000110

#define DPY7_CODE_ASTERISK 0b11000110

#define DPY7_CODE_APOSTROPHE 0b00000100

#define DPY7_CODE_0      0b11111100
#define DPY7_CODE_1      0b01100000
#define DPY7_CODE_2      0b11011010
#define DPY7_CODE_3      0b11110010
#define DPY7_CODE_4      0b01100110
#define DPY7_CODE_5      0b10110110
#define DPY7_CODE_6      0b10111110
#define DPY7_CODE_7      0b11100000
#define DPY7_CODE_8      0b11111110
#define DPY7_CODE_9      0b11110110
#define DPY7_CODE_DOT    0b00000001

//Upper
#define DPY7_CODE_A      0b11101110
#define DPY7_CODE_B      0b11111110
#define DPY7_CODE_C      0b10011100
#define DPY7_CODE_D      0b01111010
#define DPY7_CODE_E      0b10011110
#define DPY7_CODE_F      0b10001110
#define DPY7_CODE_G      0b10111110
#define DPY7_CODE_H      0b01101110
#define DPY7_CODE_I      0b00001100
#define DPY7_CODE_J      0b01110000
#define DPY7_CODE_K      0b01011110
#define DPY7_CODE_L      0b00011100
#define DPY7_CODE_M      0b11101100
#define DPY7_CODE_N      0b11101100
#define DPY7_CODE_O      0b11111100
#define DPY7_CODE_o      0b00111010
#define DPY7_CODE_P      0b11001110
#define DPY7_CODE_Q      0b11100110
#define DPY7_CODE_R      0b10001100//0b00001010
#define DPY7_CODE_S      0b10110110
#define DPY7_CODE_T      0b10001100
#define DPY7_CODE_U      0b01111100
#define DPY7_CODE_V      0b01111100
#define DPY7_CODE_W      0b01111100
#define DPY7_CODE_X      0b01101110
#define DPY7_CODE_Y      0b01110110
#define DPY7_CODE_Z      0b11011010

//Lower
#define DPY7_CODE_a      0b11101110
#define DPY7_CODE_b      0b00111110
#define DPY7_CODE_c      0b00011010
#define DPY7_CODE_d      0b01111010
#define DPY7_CODE_e      0b10011110
#define DPY7_CODE_f      0b10001110
#define DPY7_CODE_g      0b10111110
#define DPY7_CODE_h      0b00101110
#define DPY7_CODE_i      0b00001000
#define DPY7_CODE_j      0b01110000
#define DPY7_CODE_k      0b01011110
#define DPY7_CODE_l      0b00011100
#define DPY7_CODE_m      0b11101100
#define DPY7_CODE_n      0b00101010
#define DPY7_CODE_o      0b00111010
#define DPY7_CODE_p      0b11001110
#define DPY7_CODE_q      0b11100110
#define DPY7_CODE_r      0b00001010
#define DPY7_CODE_s      0b10110110
#define DPY7_CODE_t      0b00011110
#define DPY7_CODE_u      0b00111000
#define DPY7_CODE_v      0b01111100
#define DPY7_CODE_w      0b01111100
#define DPY7_CODE_x      0b01101110
#define DPY7_CODE_y      0b01110110
#define DPY7_CODE_z      0b11011010

#endif	/* DISPLAY_7SEG_CODES_H */

