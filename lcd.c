/*
 * lcd.c
 *
 *  Created on: Sep 10, 2013
 *      Author: mark
 */

#include "msp430f6726.h"

#include "lcd.h"

unsigned char LCDM1m;
unsigned char LCDM2m;
unsigned char LCDM3m;
unsigned char LCDM4m;
unsigned char LCDM5m;
unsigned char LCDM6m;
unsigned char LCDM7m;
unsigned char LCDM8m;
unsigned char LCDM9m;
unsigned char LCDM10m;




void LCD_init(void)
{
//	LCDCCTL0 = LCDSON | LCDMX0 | LCDPRE_3 | LCDDIV_10 ;
	// pre_3 devide by 8
								// lcddiv_10 devide by 11
								// = 32768/(11*8) = 372
	LCDCCTL0 = LCDSON | LCDMX0 | LCDPRE_2 | LCDDIV_20 ;
	// pre_2 devide by 4
								// lcddiv_20 devide by 21
								// = 32768/(11*8) = 390

	//enable segs 0 thru 19
	LCDCPCTL0 = 0xffff;
	LCDCPCTL1 = 0x000f;

//	LCDCVCTL = LCDREXT | R03EXT | LCD2B;
//	LCDCVCTL = LCDREXT | LCD2B;

	LCDCCTL0 |=  LCDON ;

	LCD_clear(0);	// clear display

}

void LCD_clear(unsigned char stuff)
{

	LCDM1m  = stuff;
	LCDM2m  = stuff;
	LCDM3m  = stuff;
	LCDM4m  = stuff;
	LCDM5m  = stuff;
	LCDM6m  = stuff;
	LCDM7m  = stuff;
	LCDM8m  = stuff;
	LCDM9m  = stuff;
	LCDM10m = stuff;

}

void LCD_paint(void)
{

	LCDM1  = LCDM1m;
	LCDM2  = LCDM2m;
	LCDM3  = LCDM3m;
	LCDM4  = LCDM4m;
	LCDM5  = LCDM5m;
	LCDM6  = LCDM6m;
	LCDM7  = LCDM7m;
	LCDM8  = LCDM8m;
	LCDM9  = LCDM9m;
	LCDM10 = LCDM10m;

}

void LCD_nm(int set)
{
	LCDM7m |= nm_S07;
}
void LCD_ftlbs(int set)
{
	LCDM7m |= ftlbs_S07;
}
void LCD_psi(int set)
{
	LCDM8m |= psi_S08;
}
void LCD_bar(int set)
{
	LCDM8m |= bar_S08;
}
void LCD_kPa(int set)
{
	LCDM9m |= kpa_S09;
}




void LCD_batFrm(int set)
{
	if ( set)
		LCDM8m |=   bat1_S08;
	else
		LCDM8m &=  ~bat1_S08;
}
void LCD_batLow(int set)
{
	LCDM10m |=  bat4_S10;
}
void LCD_batMid(int set)
{
	LCDM10m |=  bat3_S10;
}
void LCD_batHi(int set)
{
	LCDM8m  |=  bat2_S08;
}



void LCD_dp1(int set)
{
	LCDM6m |= P1_S06;
}

void LCD_dp2(int set)
{
	LCDM7m |= P2_S07;
}

void LCD_dp3(int set)
{
	LCDM9m |= P3_S09;
}

void LCD_digit_1(int v)
{
	switch ( v )
	{
	case 0:
		LCDM5m  |= D1_A_S05 | D1_B_S05 | D1_C_S05;
		LCDM6m  |= D1_D_S06 | D1_E_S06 | D1_F_S06;
		break;
	case 1:
		LCDM5m  |=            D1_B_S05 | D1_C_S05           ;
		break;
	case 2:
		LCDM5m  |= D1_A_S05 | D1_B_S05 |            D1_G_S05;
		LCDM6m  |= D1_D_S06 | D1_E_S06           ;
		break;
	case 3:
		LCDM5m  |= D1_A_S05 | D1_B_S05 | D1_C_S05 | D1_G_S05;
		LCDM6m  |= D1_D_S06                      ;
		break;
	case 4:
		LCDM5m  |=            D1_B_S05 | D1_C_S05 | D1_G_S05;
		LCDM6m  |=                       D1_F_S06;
		break;
	case 5:
		LCDM5m  |= D1_A_S05            | D1_C_S05 | D1_G_S05;
		LCDM6m  |= D1_D_S06            | D1_F_S06;
		break;
	case 6:
		LCDM5m  |= D1_A_S05            | D1_C_S05 | D1_G_S05;
		LCDM6m  |= D1_D_S06 | D1_E_S06 | D1_F_S06;
		break;
	case 7:
		LCDM5m  |= D1_A_S05 | D1_B_S05 | D1_C_S05           ;
		break;
	case 8:
		LCDM5m  |= D1_A_S05 | D1_B_S05 | D1_C_S05 | D1_G_S05;
		LCDM6m  |= D1_D_S06 | D1_E_S06 | D1_F_S06;
		break;
	case 9:
		LCDM5m  |= D1_A_S05 | D1_B_S05 | D1_C_S05 | D1_G_S05;
		LCDM6m  |= D1_D_S06            | D1_F_S06;
		break;
	case -1:
	case '-':
		LCDM5m |= D1_G_S05;
		break;

	}


}
void LCD_digit_2(int v)
{
	switch ( v )
	{
	case 0:
		LCDM2m  |= D2_B_S02 | D2_C_S02;
		LCDM3m  |= D2_A_S03           ;
		LCDM4m  |= D2_E_S04 | D2_F_S04;
		LCDM7m  |= D2_D_S07;
		break;
	case 1:
		LCDM2m  |= D2_B_S02 | D2_C_S02;
		break;
	case 2:
		LCDM2m  |= D2_B_S02           ;
		LCDM3m  |= D2_A_S03 | D2_G_S03;
		LCDM4m  |= D2_E_S04           ;
		LCDM7m  |= D2_D_S07;
		break;
	case 3:
		LCDM2m  |= D2_B_S02 | D2_C_S02;
		LCDM3m  |= D2_A_S03 | D2_G_S03;
		LCDM7m  |= D2_D_S07;
		break;
	case 4:
		LCDM2m  |= D2_B_S02 | D2_C_S02;
		LCDM3m  |=            D2_G_S03;
		LCDM4m  |=            D2_F_S04;
		break;
	case 5:
		LCDM2m  |=            D2_C_S02;
		LCDM3m  |= D2_A_S03 | D2_G_S03;
		LCDM4m  |=            D2_F_S04;
		LCDM7m  |= D2_D_S07;
		break;
	case 6:
		LCDM2m  |=            D2_C_S02;
		LCDM3m  |= D2_A_S03 | D2_G_S03;
		LCDM4m  |= D2_E_S04 | D2_F_S04;
		LCDM7m  |= D2_D_S07;
		break;
	case 7:
		LCDM2m  |= D2_B_S02 | D2_C_S02;
		LCDM3m  |= D2_A_S03           ;
		break;
	case 8:
		LCDM2m  |= D2_B_S02 | D2_C_S02;
		LCDM3m  |= D2_A_S03 | D2_G_S03;
		LCDM4m  |= D2_E_S04 | D2_F_S04;
		LCDM7m  |= D2_D_S07;
		break;
	case 9:
		LCDM2m  |= D2_B_S02 | D2_C_S02;
		LCDM3m  |= D2_A_S03 | D2_G_S03;
		LCDM4m  |=            D2_F_S04;
		LCDM7m  |= D2_D_S07;
		break;
	case -1:
	case '-':
		LCDM3m  |= D2_G_S03;
		break;
	}
}
void LCD_digit_3(int v)
{
	switch ( v )
	{
	case 0:
		LCDM1m  |= D3_A_S01 | D3_B_S01 | D3_C_S01           ;
		LCDM2m  |= D3_E_S02 | D3_F_S02;
		LCDM9m  |= D3_D_S09;
		break;
	case 1:
		LCDM1m  |= D3_B_S01 | D3_C_S01                      ;
		break;
	case 2:
		LCDM1m  |= D3_A_S01 | D3_B_S01            |  D3_G_S01;
		LCDM2m  |= D3_E_S02           ;
		LCDM9m  |= D3_D_S09;
		break;
	case 3:
		LCDM1m  |= D3_A_S01 | D3_B_S01 | D3_C_S01 | D3_G_S01;
		LCDM9m  |= D3_D_S09;
		break;
	case 4:
		LCDM1m  |=            D3_B_S01 | D3_C_S01 | D3_G_S01;
		LCDM2m  |=            D3_F_S02;
		break;
	case 5:
		LCDM1m  |= D3_A_S01            | D3_C_S01 | D3_G_S01;
		LCDM2m  |=            D3_F_S02;
		LCDM9m  |= D3_D_S09;
		break;
	case 6:
		LCDM1m  |= D3_A_S01            | D3_C_S01 | D3_G_S01;
		LCDM2m  |= D3_E_S02 | D3_F_S02;
		LCDM9m  |= D3_D_S09;
		break;
	case 7:
		LCDM1m  |= D3_A_S01 | D3_B_S01 | D3_C_S01           ;
		break;
	case 8:
		LCDM1m  |= D3_A_S01 | D3_B_S01 | D3_C_S01 | D3_G_S01;
		LCDM2m  |= D3_E_S02 | D3_F_S02;
		LCDM9m  |= D3_D_S09;
		break;
	case 9:
		LCDM1m  |= D3_A_S01 | D3_B_S01 | D3_C_S01 | D3_G_S01;
		LCDM2m  |=            D3_F_S02;
		LCDM9m  |= D3_D_S09;
		break;
	case -1:
	case '-':
		LCDM1m  |=                                  D3_G_S01;
		break;
	}
}
void LCD_digit_4(int v)
{
	switch ( v )
	{
	case 0:
		LCDM3m  |= D4_E_S03 | D4_F_S03;
		LCDM4m  |= D4_A_S04           ;
		LCDM9m  |= D4_D_S09;
	    LCDM10m |= D4_B_S10 | D4_C_S10;
		break;
	case 1:
	    LCDM10m |= D4_B_S10 | D4_C_S10;
		break;
	case 2:
		LCDM3m  |= D4_E_S03           ;
		LCDM4m  |= D4_A_S04 | D4_G_S04;
		LCDM9m  |= D4_D_S09;
	    LCDM10m |= D4_B_S10           ;
		break;
	case 3:
		LCDM4m  |= D4_A_S04 | D4_G_S04;
		LCDM9m  |= D4_D_S09;
	    LCDM10m |= D4_B_S10 | D4_C_S10;
		break;
	case 4:
		LCDM3m  |=            D4_F_S03;
		LCDM4m  |=            D4_G_S04;
	    LCDM10m |= D4_B_S10 | D4_C_S10;
		break;
	case 5:
		LCDM3m  |=            D4_F_S03;
		LCDM4m  |= D4_A_S04 | D4_G_S04;
		LCDM9m  |= D4_D_S09;
	    LCDM10m |=            D4_C_S10;
	    break;
	case 6:
		LCDM3m  |= D4_E_S03 | D4_F_S03;
		LCDM4m  |= D4_A_S04 | D4_G_S04;
		LCDM9m  |= D4_D_S09;
	    LCDM10m |=            D4_C_S10;
		break;
	case 7:
		LCDM4m  |= D4_A_S04           ;
	    LCDM10m |= D4_B_S10 | D4_C_S10;
		break;
	case 8:
		LCDM3m  |= D4_E_S03 | D4_F_S03;
		LCDM4m  |= D4_A_S04 | D4_G_S04;
		LCDM9m  |= D4_D_S09;
	    LCDM10m |= D4_B_S10 | D4_C_S10;
		break;
	case 9:
		LCDM3m  |=            D4_F_S03;
		LCDM4m  |= D4_A_S04 | D4_G_S04;
		LCDM9m  |= D4_D_S09;
	    LCDM10m |= D4_B_S10 | D4_C_S10;
		break;
	case -1:
	case '-':
		LCDM4m  |=            D4_G_S04;
		break;
	}
}


void LCD_displayInt(int v)
{
	int d4;
	int d3;
	int d2;
	int d1;
	int minus = 0;

	if ( v < 0 )
	{
		v = -v;
		minus = 1;
	}

	d4 = v % 10;
	v /= 10;
	d3 = v % 10;
	v /= 10;
	d2 = v % 10;
	v /= 10;
	d1 = v % 10;

	LCD_digit_4(d4);
	LCD_digit_3(d3);
	LCD_digit_2(d2);
	if ( minus )
		LCD_digit_1('-');
	else
		LCD_digit_1(d1);

}

void convertToAsciiHex(unsigned char *buff,long fds)
{
	unsigned int i;
	unsigned char d[5];


	d[4] = 0x0f & fds;
	fds >>= 4;
	d[3] = 0x0f & fds;
	fds >>= 4;
	d[2] = 0x0f & fds;
	fds >>= 4;
	d[1] = 0x0f & fds;
	fds >>= 4;
	d[0] = 0x0f & fds;


	for ( i = 0 ; i < 5 ; i++)
	{
		switch ( d[i] )
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			buff[i] = 0x30+d[i];
			break;
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			buff[i] = 0x41+d[i]-10;
			break;

		}
	}
}

void convertToAsciiDec(unsigned char *buff,long v)
{
	unsigned int i;
	unsigned char d[6];
	int minus = 0;

	if ( v < 0 )
	{
		v = -v;
		minus = 1;
	}



	d[5] = v % 10;
	v /= 10;
	d[4] = v % 10;
	v /= 10;
	d[3] = v % 10;
	v /= 10;
	d[2] = v % 10;
	v /= 10;
	d[1] = v % 10;

	for ( i = 1 ; i < 6 ; i++)
	{
		buff[i] = 0x30+d[i];
	}

	if ( minus )
		buff[0] = '-';
	else
		buff[0] = ' ';
}





void LCD_convertToAsciiFloat3D(unsigned char *buff,float fv)
{
	unsigned char d[4];
	int minus = 0;


	fv = fv*1000.0;

	int v;

	v = (int)fv;

	if ( v < 0 )
	{
		v = -v;
		minus = 1;
	}


	d[3] = v % 10;
	v /= 10;
	d[2] = v % 10;
	v /= 10;
	d[1] = v % 10;
	v /= 10;
	d[0] = v % 10;

	if ( minus )
		buff[0] = '-';
	else
		buff[0] = ' ';

	buff[1] = 0x30 + d[0];
	buff[2] = '.';
	buff[3] = 0x30 + d[1];
	buff[4] = 0x30 + d[2];
	buff[5] = 0x30 + d[3];

}



void LCD_displayFloat3D(float fv)
{
	int d4;
	int d3;
	int d2;
	int d1;

	if ( fv > 9.999 )
	{
		LCD_displayFloat2D(fv);
		return;
	}

	if ( fv < 0.0 )
		fv = 0.0;


	fv = fv*1000.0;

	int v;

	v = (int)fv;

	d4 = v % 10;
	v /= 10;
	d3 = v % 10;
	v /= 10;
	d2 = v % 10;
	v /= 10;
	d1 = v % 10;

	LCD_digit_4(d4);
	LCD_digit_3(d3);
	LCD_digit_2(d2);
	LCD_digit_1(d1);

	LCD_dp1(1);
}

void LCD_displayFloat2D(float fv)
{
	int d4;
	int d3;
	int d2;
	int d1;

	if ( fv > 99.99 )
	{
		LCD_displayFloat1D(fv);
		return;
	}

	if ( fv < 0.0 )
		fv = 0.0;

	fv = fv*100.0;

	int v;

	v = (int)(fv+0.5);

	d4 = v % 10;
	v /= 10;
	d3 = v % 10;
	v /= 10;
	d2 = v % 10;
	v /= 10;
	d1 = v % 10;


	LCD_digit_4(d4);

	LCD_digit_3(d3);

	LCD_dp2(1);

	LCD_digit_2(d2);

	if ( d1 != 0 )
		LCD_digit_1(d1);
}

void LCD_displayFloat1D(float fv)
{
	int d4;
	int d3;
	int d2;
	int d1;

	if ( fv > 999.9 )
	{
		LCD_displayFloat0D(fv);
		return;
	}

	if ( fv < 0.0 )
		fv = 0.0;

	fv = fv*10.0;

	int v;

	v = (int)fv;

	d4 = v % 10;
	v /= 10;
	d3 = v % 10;
	v /= 10;
	d2 = v % 10;
	v /= 10;
	d1 = v % 10;


	LCD_digit_4(d4);

	LCD_dp3(1);

	LCD_digit_3(d3);

	if (( d2 != 0) || ( d1 != 0 ))
		LCD_digit_2(d2);

	if ( d1 != 0 )
		LCD_digit_1(d1);
}

void LCD_displayFloat0D(float fv)
{
	int d4;
	int d3;
	int d2;
	int d1;

	if ( fv < 0.0 )
		fv = 0.0;

	fv = fv*1.0;

	int v;

	v = (int)fv;

	d4 = v % 10;
	v /= 10;
	d3 = v % 10;
	v /= 10;
	d2 = v % 10;
	v /= 10;
	d1 = v % 10;


	LCD_digit_4(d4);

	if ( (d3 != 0) || ( d2 != 0) || ( d1 != 0 ))
	LCD_digit_3(d3);

	if (( d2 != 0) || ( d1 != 0 ))
		LCD_digit_2(d2);

	if ( d1 != 0 )
		LCD_digit_1(d1);
}

void LCD_displayX10(void)
{
	// digit 1 dash
	LCDM5m |= D1_G_S05;

	// digit 2 dash
	LCDM3m  |= D2_G_S03;

	// digit 3 one
	LCDM1m  |= D3_B_S01 | D3_C_S01;

	// digit 4 zero
	LCDM3m  |= D4_E_S03 | D4_F_S03;
	LCDM4m  |= D4_A_S04           ;
	LCDM9m  |= D4_D_S09;
    LCDM10m |= D4_B_S10 | D4_C_S10;

}


void LCD_calibration(void)
{
	LCD_clear(DISPLAY_ALL_OFF);

	// digit 2 'C'
	LCDM3m  |= D2_A_S03;
	LCDM4m  |= D2_E_S04 | D2_F_S04;
	LCDM7m  |= D2_D_S07;

	// digit 3 'A'
	LCDM1m  |= D3_A_S01 | D3_B_S01 | D3_C_S01 | D3_G_S01;
	LCDM2m  |= D3_E_S02 | D3_F_S02;

	// digit 4 'L'

	LCDM3m  |= D4_E_S03 | D4_F_S03;
	LCDM9m  |= D4_D_S09;

}


void LCD_save(void)
{
//	LCD_clear(DISPLAY_ALL_OFF);

	// digit 1 'S'
	LCDM5m  |= D1_A_S05 |            D1_C_S05 | D1_G_S05;
	LCDM6m  |= D1_D_S06 |            D1_F_S06;

	// digit 2 'A'
	LCDM2m  |= D2_B_S02 | D2_C_S02;
	LCDM3m  |= D2_A_S03 | D2_G_S03;
	LCDM4m  |= D2_E_S04 | D2_F_S04;

	// digit 3 'v'
	LCDM1m  |=                       D3_C_S01           ;
	LCDM2m  |= D3_E_S02           ;
	LCDM9m  |= D3_D_S09;

	// digit 4 'E'
	LCDM3m  |= D4_E_S03 | D4_F_S03;
	LCDM4m  |= D4_A_S04 | D4_G_S04;
	LCDM9m  |= D4_D_S09;

}

void LCD_error(void)
{
	// digit 1 'E'
	LCDM5m  |= D1_A_S05                       | D1_G_S05;
	LCDM6m  |= D1_D_S06 | D1_E_S06 | D1_F_S06;

	// digit 2 'r'
	LCDM3m  |=            D2_G_S03;
	LCDM4m  |= D2_E_S04           ;

	// digit 3 'r'
	LCDM1m  |=                                  D3_G_S01;
	LCDM2m  |= D3_E_S02           ;

}

void LCD_dashes(void)
{
	LCD_clear(DISPLAY_ALL_OFF);

	// digit 1 '-'
	LCDM5m  |= D1_G_S05;

	// digit 2 '-'
	LCDM3m  |= D2_G_S03;

	// digit 3 '-'
	LCDM1m  |= D3_G_S01;

	// digit 4 '-'
	LCDM4m  |= D4_G_S04;

}

