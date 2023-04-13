/*
 * lcd.h
 *
 *  Created on: Sep 10, 2013
 *      Author: mark
 */

#ifndef LCD_H_
#define LCD_H_

void LCD_init(void);
void LCD_clear(unsigned char stuff);
void LCD_paint(void);

#define DISPLAY_ALL_ON 0xff
#define DISPLAY_ALL_OFF 0x00

void LCD_nm(int set);
void LCD_ftlbs(int set);
void LCD_psi(int set);
void LCD_bar(int set);
void LCD_kPa(int set);

void LCD_batFrm(int set);
void LCD_batLow(int set);
void LCD_batMid(int set);
void LCD_batHi(int set);
void LCD_batLevel(int level);

void LCD_dp1(int set);
void LCD_dp2(int set);
void LCD_dp3(int set);

void LCD_digit_1(int v);
void LCD_digit_2(int v);
void LCD_digit_3(int v);
void LCD_digit_4(int v);

void LCD_displayInt(int v);
void convertToAsciiHex(unsigned char *buff,long fds);
void convertToAsciiDec(unsigned char *buff,long v);
void LCD_convertToAsciiFloat3D(unsigned char *buff,float fv);
void LCD_displayFloat3D(float fv);
void LCD_displayFloat2D(float fv);
void LCD_displayFloat1D(float fv);
void LCD_displayFloat0D(float fv);

void LCD_displayX10(void);
void LCD_calibration(void);
void LCD_save(void);
void LCD_error(void);

void LCD_dashes(void);

#define D1_A_S05  0x10 // 0b00000001
#define D1_B_S05  0x01 // 0b00010000
#define D1_C_S05  0x02 // 0b00100000
#define D1_D_S06  0x20 // 0b00000010
#define D1_E_S06  0x02 // 0b00100000
#define D1_F_S06  0x01 // 0b00010000
#define D1_G_S05  0x20 // 0b00000010

#define D2_A_S03  0x01 // 0b00010000
#define D2_B_S02  0x10 // 0b00000001
#define D2_C_S02  0x20 // 0b00000010
#define D2_D_S07  0x20 // 0b00000010
#define D2_E_S04  0x02 // 0b00000010
#define D2_F_S04  0x01 // 0b00000001
#define D2_G_S03  0x02 // 0b00100000

#define D3_A_S01  0x10 // 0b00000001
#define D3_B_S01  0x01 // 0b00010000
#define D3_C_S01  0x02 // 0b00100000
#define D3_D_S09  0x20 // 0b00000010
#define D3_E_S02  0x02 // 0b00100000
#define D3_F_S02  0x01 // 0b00010000
#define D3_G_S01  0x20 // 0b00000010

#define D4_A_S04  0x10 // 0b00010000
#define D4_B_S10  0x10 // 0b00000001
#define D4_C_S10  0x20 // 0b00000010
#define D4_D_S09  0x01 // 0b00010000
#define D4_E_S03  0x20 // 0b00000010
#define D4_F_S03  0x10 // 0b00000001
#define D4_G_S04  0x20 // 0b00100000

#define bat4_S10  0x02 // 0b00100000
#define bat3_S10  0x01 // 0b00010000
#define bat2_S08  0x01 // 0b00000001
#define bat1_S08  0x2// 0b00000010

#define kpa_S09   0x02 // 0b00100000
#define P3_S09    0x10 // 0b00000001
#define psi_S08   0x20 // 0b00100000
#define bar_S08   0x10 // 0b00010000
#define P2_S07    0x10 // 0b00000001

#define nm_S07    0x02 // 0b00100000
#define ftlbs_S07 0x01 // 0b00010000
#define P1_S06    0x10 // 0b00000001




#endif /* LCD_H_ */
