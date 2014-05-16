#include "display_7seg.h"
#include <string.h>
#include <ctype.h>


#define DIGIT_CODES_LEN 16
//Таблица соответсвия цифр и их представлений на индикаторе.
static const value7seg_t __flash digit_codes[DIGIT_CODES_LEN] = {
    DPY7_CODE_0, DPY7_CODE_1, DPY7_CODE_2, DPY7_CODE_3,
    DPY7_CODE_4, DPY7_CODE_5, DPY7_CODE_6, DPY7_CODE_7,
    DPY7_CODE_8, DPY7_CODE_9, DPY7_CODE_A, DPY7_CODE_B,
    DPY7_CODE_C, DPY7_CODE_D, DPY7_CODE_E, DPY7_CODE_F
};

#define CHARS_CODES_LEN 26
//Таблица символов больших.
static const value7seg_t __flash chars_codes_upper[CHARS_CODES_LEN] = {
    DPY7_CODE_A, DPY7_CODE_B, DPY7_CODE_C, DPY7_CODE_D,
    DPY7_CODE_E, DPY7_CODE_F, DPY7_CODE_G, DPY7_CODE_H,
    DPY7_CODE_I, DPY7_CODE_J, DPY7_CODE_K, DPY7_CODE_L,
    DPY7_CODE_M, DPY7_CODE_N, DPY7_CODE_O, DPY7_CODE_P,
    DPY7_CODE_Q, DPY7_CODE_R, DPY7_CODE_S, DPY7_CODE_T,
    DPY7_CODE_U, DPY7_CODE_V, DPY7_CODE_W, DPY7_CODE_X,
    DPY7_CODE_Y, DPY7_CODE_Z
};
//Таблица символов маленьких.
static const value7seg_t __flash chars_codes_lower[CHARS_CODES_LEN] = {
    DPY7_CODE_a, DPY7_CODE_b, DPY7_CODE_c, DPY7_CODE_d,
    DPY7_CODE_e, DPY7_CODE_f, DPY7_CODE_g, DPY7_CODE_h,
    DPY7_CODE_i, DPY7_CODE_j, DPY7_CODE_k, DPY7_CODE_l,
    DPY7_CODE_m, DPY7_CODE_n, DPY7_CODE_o, DPY7_CODE_p,
    DPY7_CODE_q, DPY7_CODE_r, DPY7_CODE_s, DPY7_CODE_t,
    DPY7_CODE_u, DPY7_CODE_v, DPY7_CODE_w, DPY7_CODE_x,
    DPY7_CODE_y, DPY7_CODE_z
};

value7seg_t display_7seg_digit_code(uint8_t digit)
{
    if(digit >= DIGIT_CODES_LEN) digit = digit % DIGIT_CODES_LEN;
    return digit_codes[digit];
}

value7seg_t display_7seg_char_code(char c)
{
    if(isdigit(c)) return digit_codes[c - '0'];
    if(isalpha(c)){
        if(isupper(c)) return chars_codes_upper[c - 'A'];
        return chars_codes_lower[c - 'a'];
    }
    //space - is default char
    //if(isspace(c)) return DPY7_CODE_NONE;
    if(c == '.' || c == ',') return DPY7_CODE_DOT;
    if(c == '-') return DPY7_CODE_MINUS;
    if(c == '_') return DPY7_CODE_UNDERLINE;
    if(c == '*') return DPY7_CODE_CELSIUM;
    
    return DPY7_CODE_NONE;
}

void display_7seg_convert(value7seg_t* values, const char* str)
{
    value7seg_t* vals = values;
    value7seg_t v = DPY7_CODE_NONE;
    while(*str){
        v = display_7seg_char_code(*str ++);
        if(v == DPY7_CODE_DOT && vals > values) *(vals - 1) |= DPY7_CODE_DOT;
        else *vals ++ = v;
    }
}

