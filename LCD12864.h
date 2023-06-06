#ifndef __lcd12864_H__
#define __lcd12864_H__
sbit RS=P2^6; //并行的指令/数据选择信号, H数据, L命令
sbit RW=P2^5; //并行读写选择信号, H读, L写
sbit E=P2^7; //并行使能端, H有效, L无效
sbit PSB=P3^2; //并/串接口选择, H并,L串
sbit RET=P3^4; //复位, L有效
#define  LCD_data P0
unsigned char Read_busy(void);
void write_LCD_data(unsigned char);
unsigned char read_LCD_data(void);
void write_LCD_command(unsigned char);
void Lcd_PutPixel(unsigned char,unsigned char,unsigned char);
unsigned char Lcd_ReadPixel(unsigned char,unsigned char);
void Lcd_HoriLine(unsigned char,unsigned char,unsigned char Length,unsigned char Color);
void Lcd_VertLine(unsigned char x,unsigned char y,unsigned char Length,unsigned char Color);
void Lcd_Line(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,unsigned char Color);
void Lcd_Rectangle(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char Color);
void Lcd_Circle(unsigned char x,unsigned char y,unsigned char r,unsigned char Color);
void Lcd_Clear(unsigned char);
void write_LCD_Str(unsigned char,unsigned char,unsigned char *);
void Lcd_Reset(void);
void Lcd_MRectangle(void);

#endif
