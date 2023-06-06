#include "reg52.h"
#include "intrins.h"
#include "LCD12864.h"
#define uint unsigned int 
#define uchar unsigned char
void delay3(unsigned int t)
{  
	unsigned int i,j;
	for(i=0;i<t;i++)
		for(j=0;j<10;j++);    
}
/*************
测试LCD是否处于忙状态
如果忙则返回0x80，否则返回0
**************/
uchar Read_busy(void)
{
    uchar Busy;
    LCD_data=0xff;   //读取端口状态先写1
    RS=0;
    RW=1;
    E=1;
    _nop_(); 
    Busy=LCD_data&0x80;
    E=0;
    return Busy;
}

/*********************************
向LCD写入字节数据
**********************************/
void write_LCD_data(uchar Data)
{  
	while(Read_busy());
	RS=1;
	RW=0;
	E=0;
	_nop_(); 
	LCD_data=Data;
	E=1;
	_nop_();
	_nop_();
    _nop_();
	E=0;
}

/***********************************
从LCD中读出数据
************************************/
uchar read_LCD_data(void)
{
	uchar Temp;
	while(Read_busy());
 	LCD_data=0xff;
 	RS=1;
	RW=1;
	E=1;
	_nop_();
   	Temp=LCD_data;
   	E=0;
   	return Temp;
}

/*************************************
想LCD中写入指令代码
**************************************/
void write_LCD_command(uchar CmdCode)
{  
	while(Read_busy());
   	RS=0;
   	RW=0;
   	E=0;
   	_nop_();  
	_nop_();
   	LCD_data=CmdCode;
   	_nop_(); 
	_nop_();
   	E=1;
   	_nop_();  
	_nop_();
    _nop_();
   	E=0;
}

/*************************************
向LCD指定起始位置写入一个字符串
*************************************/
void write_LCD_Str(uchar x,uchar y,uchar *Str)
{
	if((y>3)||(x>7))
		return;//如果指定位置不在显示区域内，则不做任何写入直接返回
	EA=0;
	switch(y)
	{
		case 0:
				write_LCD_command(0x80+x);
				break;
		case 1:
				write_LCD_command(0x90+x);
				break;				
		case 2:
				write_LCD_command(0x88+x);
				break;
		case 3:
				write_LCD_command(0x98+x);
				break;
	}
	while(*Str>0)
	{  
		write_LCD_data(*Str);
  		Str++;     
	}
	EA=1;
}

/**************************************
为加速逻辑运算而设置的掩码表，这是以牺牲空间而换取时间的办法
***************************************/
code uint MaskTab[]={0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};
/***************************************
向LCD指定坐标写入一个像素，0代表白（无显示），1代表黑（有显示）
****************************************/
 void Lcd_PutPixel(uchar x,uchar y,uchar Color)
{
	uchar z,w;
	uint Temp;
	if(x>=128||y>=64)
		return;        //如果指定位置不在显示区域内，则不做任何写入直接返回
	Color=Color%2;
	w=15-x%16;         //确定对这个字的第多少位进行操作
	x=x/16;            //确定为一行上的第几字

	if(y<32)           //如果为上页
		z=0x80;
	else               //如果为下页
		z=0x88;

	y=y%32;
	EA=0;
	write_LCD_command(0x36);          //扩充指令 绘图显示
	write_LCD_command(y+0x80);        //行地址
	write_LCD_command(x+z);           //列地址 
	Temp=read_LCD_data();             //先空读一次
	Temp=(uint)read_LCD_data()<<8;  //再读出高8位
	Temp|=(uint)read_LCD_data();    //再读出低8位
	EA=1;
	if(Color==1) //如果写入颜色为1
		Temp|=MaskTab[w]; //在此处查表实现加速
	else         //如果写入颜色为0
		Temp&=~MaskTab[w];//在此处查表实现加速
	EA=0;
	write_LCD_command(y+0x80);        //行地址
	write_LCD_command(x+z);           //列地址
    write_LCD_data(Temp>>8);          //先写入高8位
    write_LCD_data(Temp&0x00ff);      //再写入低8位
	write_LCD_command(0x30);          //基本指令集
	EA=1;
}



/***************************************
向LCD指定位置画一条长度为Length的指定颜色的水平线
****************************************/
void Lcd_HoriLine(uchar x,uchar y,uchar Length,uchar Color)
{
	uchar i;
	if(Length==0)
		return;
	for(i=0;i<Length;i++)
	{
		Lcd_PutPixel(x+i,y,Color);
	}
}

/***************************************
向LCD指定位置画一条长度为Length的指定颜色的垂直线
****************************************/
void Lcd_VertLine(uchar x,uchar y,uchar Length,uchar Color)
{
	uchar i;
	if(Length==0)
		return;
	for(i=0;i<Length;i++)
	{
		Lcd_PutPixel(x,y+i,Color);
	}
}


/*******************************************
向LCD指定左上角坐标和右下角坐标画一个指定颜色的矩形
********************************************/
void Lcd_Rectangle(uchar x0,uchar y0,uchar x1,uchar y1,uchar Color)
{
	uchar Temp;
	if(x0>x1)
	{
		Temp=x0;
		x0=x1;
		x1=Temp;
	}	
	if(y0>y1)
	{
		Temp=y0;
		y0=y1;
		y1=Temp;
	}
	Lcd_VertLine(x0,y0,y1-y0+1,Color);
	Lcd_VertLine(x1,y0,y1-y0+1,Color);
	Lcd_HoriLine(x0,y0,x1-x0+1,Color);
	Lcd_HoriLine(x0,y1,x1-x0+1,Color);	
}

/*******************************************
动态效果，画面取反
********************************************/
void Lcd_MRectangle(void) 
{
	uchar n,m;
for(m=0;m<90;m++)
	{
		for(n=0;n<21;n++)
	{
		Lcd_Rectangle(m,3*n,m+2,3*n+2,1);
	}
  }
}



/*****************************************
清除Lcd全屏，如果清除模式Mode为0，则为全屏清除为颜色0（无任何显示）
否则为全屏清除为颜色1(全屏填充显示)
******************************************/
void Lcd_Clear(uchar Mode)
{
	uchar x,y,ii;
	uchar Temp;
	if(Mode%2==0)
		Temp=0x00;
	else
		Temp=0xff;
	write_LCD_command(0x36);
	for(ii=0;ii<9;ii+=8)   
		for(y=0;y<0x20;y++)     
			for(x=0;x<8;x++)
			{ 	
				EA=0;
				write_LCD_command(y+0x80);        //行地址
				write_LCD_command(x+0x80+ii);     //列地址     
				write_LCD_data(Temp);             //写数据 D15－D8 
				write_LCD_data(Temp);             //写数据 D7－D0 
				EA=1;
			}
	write_LCD_command(0x30);
}

/****************************************
LCD初始化
*****************************************/
void Lcd_Reset()
{  
	
	write_LCD_command(0x30);       //选择基本指令集
	write_LCD_command(0x0c);       //开显示(无游标、不反白)
	write_LCD_command(0x01);       //清除显示，并且设定地址指针为00H
	write_LCD_command(0x06);       //指定在资料的读取及写入时，设定游标的移动方向及指定显示的移位
}
