#include "reg52.h"
#include "Lcd12864.h"
#include "Key.h"
#define uchar unsigned char
#define uint unsigned int
sbit IRIN=P3^3;
uchar IrValue[6];
uchar Time;
/**********************************
 函数名         : IrInit()
* 函数功能		   : 初始化红外线接收
************************************/
void IrInit()
{
	IT0=1;//下降沿触发
	EX1=1;//打开中断0允许
	EA=1;	//打开总中断
	IRIN=1;//初始化端口
}
/************************************
伪随机数发生器
*************************************/
static unsigned long Seed = 1;
#define A 48271L
#define M 2147483647L
#define Q (M / A)
#define R (M % A)
double Random(void)
{
	long TmpSeed;
	TmpSeed=A*(Seed%Q)-R*(Seed/Q);
	if(TmpSeed>=0)
		Seed=TmpSeed;
	else
		Seed=TmpSeed+M;
	return (double)Seed/M;
}

/**************************************
为伪随机数发生器播种
***************************************/
void InitRandom(unsigned long InitVal)
{
	Seed=InitVal;
}

//延时子程序
void delay(unsigned int t)
{  
	unsigned int i,j;
	for(i=0;i<t;i++)
		for(j=0;j<10;j++);    
}
void delay1(uint i)
{
	while(i--);	
}
/*********************************
初始化MCU
**********************************/
void InitCpu(void)
{
	TMOD=0x0;
	TH0=0;
	TL0=0;
	TR0=1;
	ET0=1;
	EA=1;
}

#define N 30
struct Food
{
	uchar x;
	uchar y;
	uchar yes;
}food;//食物结构体
struct Snake
{
	uchar x[N];
	uchar y[N];
	uchar node;
	uchar direction;
	uchar life;
}snake;//蛇结构体

uchar Flag=0;
uchar Score=0;
uchar Speed=5;
uchar KeyBuffer=0;
uchar ASPEED=1;
#define FUNC 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define RIGHT 5
#define NFUNC 6
#define PASSSCORE 25//预定义过关成绩
void Timer0Int(void) interrupt 1
{
	switch(OSScanKey())
	{
		case 80:
			    KeyBuffer=NFUNC;break;
        case 90:
				KeyBuffer=FUNC;break;
		case 23:
				KeyBuffer=DOWN;
				break;
		case 43:
				KeyBuffer=UP;
				break;
		case 13:
				KeyBuffer=RIGHT;
				break;
	   	case 33:                                                   
				KeyBuffer=LEFT;
				break;
		default:
				break;
	}
     	switch(IrValue[2])
	{
		case 0x15:
			    KeyBuffer=NFUNC;break;
        case 0x09:
				KeyBuffer=FUNC;break;
		case 0x52:
				KeyBuffer=DOWN;
				break;
		case 0x18:
				KeyBuffer=UP;
				break;
		case 0x5a:
				KeyBuffer=RIGHT;
				break;
	   	case 0x08:                                            
				KeyBuffer=LEFT;
				break;
		default:
				break;
	}		
}

/******************************
画墙壁，初始化界面
*******************************/
void DrawBoard(void)
{
	uchar n;
	for(n=0;n<31;n++)
	{
		Lcd_Rectangle(3*n,0,3*n+2,2,1);
		Lcd_Rectangle(3*n,60,3*n+2,62,1);
        Lcd_HoriLine(3*n+1,1,1,1);
        Lcd_HoriLine(3*n+1,61,1,1);
	}
	for(n=0;n<21;n++)
	{
		Lcd_Rectangle(0,3*n,2,3*n+2,1);
		Lcd_Rectangle(90,3*n,92,3*n+2,1);
        Lcd_HoriLine(1,3*n+1,1,1);
        Lcd_HoriLine(91,3*n+1,1,1);		
	}
		for(n=1;n<11;n++)//路障y1
	{
		Lcd_Rectangle(30,3*n,32,3*n+2,1);
        Lcd_HoriLine(31,3*n+1,1,1);	
	}
		for(n=14;n<21;n++)//路障y2
	{
		Lcd_Rectangle(57,3*n,59,3*n+2,1);
        Lcd_HoriLine(58,3*n+1,1,1);	
	}
	    for(n=19;n<24;n++)//路障x
	{
		Lcd_Rectangle(3*n,42,3*n+2,44,1);
        Lcd_HoriLine(3*n+1,43,1,1);	
	}
	Lcd_HoriLine(93,30,35,1);
	Lcd_HoriLine(93,62,35,1);
}

/***************************
打印成绩 + 速度级别
****************************/
void PrintScore(void)
{
	uchar Str[3];;
	write_LCD_Str(6,0,"Mark");
	Str[0]=(Score/10)|0x30;//十位
	Str[1]=(Score%10)|0x30;//个位
	Str[2]=0;
	write_LCD_Str(7,1,Str);
}
void PrintSpeed(void)
{
	uchar Str[2];
            uchar str[2];
            uchar n;
    str[0]=0x4C;
	str[1]=0x76;
	str[2]=0;

    write_LCD_Str(6,3,str);
switch(Speed)
	{
     case 5:
			n=1;write_LCD_Str(6,2,"Easy");break;
     case 4:
			n=2;write_LCD_Str(6,2,"Easy");break;
     case 3:
			n=3;write_LCD_Str(6,2,"Hard");break;
     case 2:
			n=4;write_LCD_Str(6,2,"Hard");break;
     case 1:
			n=5;write_LCD_Str(6,2,"Fear");break;
     default:
			break;
            } 
	Str[0]=n|0x30;
	Str[1]=0;
	write_LCD_Str(7,3,Str);
}

/***********************************
游戏结束处理
************************************/
void GameOver(void)
{
	unsigned char n;
	Lcd_Rectangle(food.x,food.y,food.x+2,food.y+2,0);//消隐出食物
	for(n=1;n<snake.node;n++)
	{
		Lcd_Rectangle(snake.x[n],snake.y[n],snake.x[n]+2,snake.y[n]+2,0);//消隐食物，蛇头已到墙壁内，故不用消去		
	}
	if(snake.life==0)//如果蛇还活着
		write_LCD_Str(2,1,"WIN");
	else             //如果蛇死了
		write_LCD_Str(2,1,"LOSE");
	write_LCD_Str(1,2,"GAMEOVER");
}

/********************************
贪吃蛇算法部分
*********************************/
void GamePlay(void)
{
	uchar n;
	InitRandom(TL0);
	food.yes=1;    //1表示需要出现新食物，0表示已经存在食物尚未吃掉
	snake.life=0;  //表示蛇活着
	snake.direction=RIGHT;
	snake.x[0]=6;snake.y[0]=6;
	snake.x[1]=3;snake.y[1]=6;
	snake.node=2;
	PrintScore();
	PrintSpeed();
	while(1)
	{
		if(food.yes==1)
		{
			while(1)
			{
				food.x=Random()*85+3;
				food.y=Random()*55+3;//获得随机数
				while(food.x%3!=0)
					food.x++;        //如果生成在障碍物内，重新处理食物坐标
				while(food.y%3!=0)
					food.y++;
				while(food.x==30)
					food.x=food.x+3;
				while(food.x==57)
					food.x=food.x+3;
				while(food.y==19)
					food.x=food.y-3;

			    for(n=0;n<snake.node;n++)//判断产生的食物坐标是否和蛇身重合
				{
					if((food.x==snake.x[n])&&(food.y==snake.y[n]))
						break;
				}
				if(n==snake.node)
				{
					food.yes=0;
					break;            //产生有效的食物坐标
				}
			}
		}
	
		if(food.yes==0)
		{
			Lcd_Rectangle(food.x,food.y,food.x+2,food.y+2,1);
		}	
		for(n=snake.node-1;n>0;n--)
		{
			snake.x[n]=snake.x[n-1];  //蛇后一节等于前一节信息
			snake.y[n]=snake.y[n-1];
		}
		switch(snake.direction)    //方向处理
		{
			case DOWN: snake.y[0]+=3;break;
			case UP:   snake.y[0]-=3;break;
			case LEFT: snake.x[0]-=3;break;
			case RIGHT:snake.x[0]+=3;break;
			default:break;
		}
		for(n=3;n<snake.node;n++)//从第三节开始判断蛇头是否咬到自己
		{
			if(snake.x[n]==snake.x[0]&&snake.y[n]==snake.y[0])
			{
				snake.life=1;
				GameOver();
				break;
			}
		}
		if(snake.x[0]<3||snake.x[0]>=90||snake.y[0]<3||snake.y[0]>=60)//判蛇头是否撞到墙壁
		{
			snake.life=1;
			GameOver();
			break;
		}
	     for(n=1;n<11;n++)
		 {
		 if(snake.x[0]==30&&snake.y[0]==3*n)//判蛇头是否撞到障碍物y1
		{
			snake.life=1;
			GameOver();
			break;
		}
		}
		for(n=14;n<21;n++)
		 {
		 if(snake.x[0]==57&&snake.y[0]==3*n)//判蛇头是否撞到障碍物y2
		{
			snake.life=1;
			GameOver();
			break;
		}
		}	     
		for(n=19;n<24;n++)
		 {
		 if(snake.x[0]==3*n&&snake.y[0]==42)//判蛇头是否撞到障碍物x
		{
			snake.life=1;
			GameOver();
			break;
		}
		}
		if(snake.life==1)
			break;//蛇死，则跳出while(1)循环
		if(snake.x[0]==food.x&&snake.y[0]==food.y)//判蛇是否吃到食物
		{
			Lcd_Rectangle(food.x,food.y,food.x+2,food.y+2,0);//消隐食物
			snake.x[snake.node]=200;
			snake.y[snake.node]=200;//产生蛇新的节坐标先放在看不见的位置
			snake.node++;           //蛇节数加1
			food.yes=1;             //食物标志置1
			if(++Score>=PASSSCORE)
			{
				PrintScore();
				GameOver();
				break;
			}
			PrintScore();
		}
		for(n=0;n<snake.node;n++)
		{
			Lcd_Rectangle(snake.x[n],snake.y[n],snake.x[n]+2,snake.y[n]+2,1);
			Lcd_HoriLine(snake.x[n]+1,snake.y[n]+1,1,1);
		} //根据蛇的节数画出蛇
	 if(keypros()==1)
		{
			delay(Speed*(500-(5-Speed)*50));
		}
		else
		{
	    Speed=5-(Score/5); //根据得分改变游戏速度
		PrintSpeed();  //重新打印速度
		delay(Speed*(1000-(5-Speed)*120));//加快速度更改
		}
		Lcd_Rectangle(snake.x[snake.node-1],snake.y[snake.node-1],snake.x[snake.node-1]+2,snake.y[snake.node-1]+2,0);
		Lcd_HoriLine(snake.x[snake.node-1]+1,snake.y[snake.node-1]+1,1,0); 	//消隐蛇尾
		switch(KeyBuffer)
		{
			case NFUNC:
					ASPEED=0;
			        break;
			case FUNC:
					ASPEED=1;
				    break;
			case DOWN:
					KeyBuffer=0;
					if(snake.direction!=UP)
						snake.direction=DOWN;  //不可掉头
					break;
			case UP:
					KeyBuffer=0;
					if(snake.direction!=DOWN)
						snake.direction=UP;
					break;
			case RIGHT:
					KeyBuffer=0;
					if(snake.direction!=LEFT)
						snake.direction=RIGHT;
					break;
		   	case LEFT:
					KeyBuffer=0;
					if(snake.direction!=RIGHT)
						snake.direction=LEFT;
					break;
			default:
					break;
		}			
	}//结束while(1)
}
/***********************************
* 函数功能: 读取红外数值的中断函数
******************************************/
void ReadIr() interrupt 2
{
	uchar j,k;
	uint err;
	Time=0;					 
	delay1(700);	//7ms
	if(IRIN==0)		//确认是否真的接收到正确的信号
	{	 
		
		err=1000;				//1000*10us=10ms,超过说明接收到错误的信号
		/*当两个条件都为真是循环，如果有一个条件为假的时候跳出循环，免得程序出错的时
		侯，程序死在这里*/	
		while((IRIN==0)&&(err>0))	//等待前面9ms的低电平过去  		
		{			
			delay1(1);
			err--;
		} 
		if(IRIN==1)			//如果正确等到9ms低电平
		{
			err=500;
			while((IRIN==1)&&(err>0))		 //等待4.5ms的起始高电平过去
			{
				delay1(1);
				err--;
			}
			for(k=0;k<4;k++)		//共有4组数据
			{				
				for(j=0;j<8;j++)	//接收一组数据
				{

					err=60;		
					while((IRIN==0)&&(err>0))//等待信号前面的560us低电平过去
					{
						delay1(1);
						err--;
					}
					err=500;
					while((IRIN==1)&&(err>0))	 //计算高电平的时间长度。
					{
						delay1(10);	 //0.1ms
						Time++;
						err--;
						if(Time>30)
						{
							return;
						}
					}
					IrValue[k]>>=1;	 //k表示第几组数据
					if(Time>=8)			//如果高电平出现大于565us，那么是1
					{
						IrValue[k]|=0x80;
					}
					Time=0;		//用完时间要重新赋值							
				}
			}
		}
		if(IrValue[2]!=~IrValue[3])
		{
			return;
		}
	}
	
}


void Main()
{  
	InitCpu();//初始化CPU
	IrInit();  //初始化红外
	Lcd_Reset(); //初始化LCD屏
	write_LCD_Str(2,1,"GAMEBOY");
	delay(5000);
	delay(5000);
	write_LCD_command(0x01);   //清除显示，并且设定地址指针为00H
	Lcd_Clear(0);//清屏
	DrawBoard();//画界面
	GamePlay();//玩游戏
	GameOver();//游戏结束
	while(1);//要想重玩，只能重启，可继续完善该游戏
}
