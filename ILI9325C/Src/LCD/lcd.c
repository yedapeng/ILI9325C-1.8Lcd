
#include "lcd.h"
#include "stdlib.h"
#include "font.h" 
#include "stm32f4xx_hal.h" 
//#include "delay.h"	  
#include "cmsis_os.h"

//////////////////////////////////////////////////////////////////////////////////
//
//#define LCD_RST_L()     HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)
//#define LCD_RST_H()     HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)
//
//#define LCD_CS_L()     HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
//#define LCD_CS_H()     HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
//
//#define LCD_SCL_L()     HAL_GPIO_WritePin(LCD_SCL_GPIO_Port, LCD_SCL_Pin, GPIO_PIN_RESET)
//#define LCD_SCL_H()     HAL_GPIO_WritePin(LCD_SCL_GPIO_Port, LCD_SCL_Pin, GPIO_PIN_SET)
//
//#define LCD_SDA_L()     HAL_GPIO_WritePin(LCD_SDA_GPIO_Port, LCD_SDA_Pin, GPIO_PIN_RESET)
//#define LCD_SDA_H()     HAL_GPIO_WritePin(LCD_SDA_GPIO_Port, LCD_SDA_Pin, GPIO_PIN_SET)

#define LCD_RST_L()     LL_GPIO_ResetOutputPin(LCD_RST_GPIO_Port, LCD_RST_Pin)
#define LCD_RST_H()     LL_GPIO_SetOutputPin(LCD_RST_GPIO_Port, LCD_RST_Pin)

#define LCD_CS_L()      LL_GPIO_ResetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin)
#define LCD_CS_H()      LL_GPIO_SetOutputPin(LCD_CS_GPIO_Port, LCD_CS_Pin)

#define LCD_SCL_L()     LL_GPIO_ResetOutputPin(LCD_SCL_GPIO_Port, LCD_SCL_Pin)
#define LCD_SCL_H()     LL_GPIO_SetOutputPin(LCD_SCL_GPIO_Port, LCD_SCL_Pin)

#define LCD_SDA_L()     LL_GPIO_ResetOutputPin(LCD_SDA_GPIO_Port, LCD_SDA_Pin)
#define LCD_SDA_H()     LL_GPIO_SetOutputPin(LCD_SDA_GPIO_Port, LCD_SDA_Pin)


				 
//LCD的画笔颜色和背景色	   
u16 POINT_COLOR=0x0000;	//画笔颜色
u16 BACK_COLOR=0xFFFF;  //背景色 

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;
	
//送8位数据
void SPI_SendData(u8 data)
{  u8 n;
   for(n=0; n<8; n++)			
   {  
     if(data&0x80) 
      {
        LCD_SDA_H();
      }
      else          
      {
        LCD_SDA_L();
      }
      data<<= 1;
      LCD_SCL_L();
      LCD_SCL_H();			
   }
}		   
//写寄存器函数
//regval:寄存器值
void LCD_WR_REG(u8 regval)
{ 
	LCD_CS_L();
	LCD_SDA_L();//送指令
	LCD_SCL_L(); 
	LCD_SCL_H();

	SPI_SendData(regval);
  LCD_CS_H();
}
//写LCD数据
//data:要写入的值
void LCD_WR_DATA(u16 data)
{										    	   
	LCD_CS_L();
	LCD_SDA_H();
	LCD_SCL_L(); 
	LCD_SCL_H();
	SPI_SendData(data>>8);
	
	LCD_SDA_H();
	LCD_SCL_L(); 
	LCD_SCL_H();
	SPI_SendData(data&0xff);
	
	LCD_CS_H();
}					   
//写寄存器
//LCD_Reg:寄存器地址
//LCD_RegValue:要写入的数据
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}	      
//开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD_WR_REG(0x22);
}	 
//LCD写GRAM
//RGB_Code:颜色值
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code);
}

//当mdk -O1时间优化时需要设置
//延时i
void opt_delay(u8 i)
{
	while(i--);
}

//LCD开启显示
void LCD_DisplayOn(void)
{					   
	LCD_WriteReg(R7,0x0173); 			//开启显示
}	 
//LCD关闭显示
void LCD_DisplayOff(void)
{	   
	LCD_WriteReg(R7,0x0);//关闭显示 
}   
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	 
		if(lcddev.dir==1)Xpos=lcddev.width-1-Xpos;//横屏其实就是调转x,y坐标
		LCD_WriteReg(lcddev.setxcmd, Xpos+0x0A);//X不是从0坐标开始，是从0X0A坐标开始的
		LCD_WriteReg(lcddev.setycmd, Ypos);
} 		 
//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341等IC已经实际测试	   	   
void LCD_Scan_Dir(u8 dir)
{
	u16 regval=0;
	u8 dirreg=0;
	switch(dir)
	{
		case L2R_U2D://从左到右,从上到下
		 regval|=(1<<5)|(1<<4)|(0<<3); 
		 break;
		case L2R_D2U://从左到右,从下到上
		 regval|=(0<<5)|(1<<4)|(0<<3); 
		 break;
		case R2L_U2D://从右到左,从上到下
		 regval|=(1<<5)|(0<<4)|(0<<3);
		 break;
		case R2L_D2U://从右到左,从下到上
		 regval|=(0<<5)|(0<<4)|(0<<3); 
		 break;	 
		case U2D_L2R://从上到下,从左到右
		 regval|=(1<<5)|(1<<4)|(1<<3); 
		 break;
		case U2D_R2L://从上到下,从右到左
		 regval|=(1<<5)|(0<<4)|(1<<3); 
		 break;
		case D2U_L2R://从下到上,从左到右
		 regval|=(0<<5)|(1<<4)|(1<<3); 
		 break;
		case D2U_R2L://从下到上,从右到左
		 regval|=(0<<5)|(0<<4)|(1<<3); 
		 break;	 
	}
	dirreg=0X03;
	regval|=1<<12;  
	LCD_WriteReg(dirreg,regval);
}   
//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);		//设置光标位置 
	LCD_WriteRAM_Prepare();	//开始写入GRAM
	LCD_WR_DATA(POINT_COLOR); 
}
//快速画点
//x,y:坐标
//color:颜色
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color)
{	   
	if(lcddev.dir==1)x=lcddev.width-1-x;//横屏其实就是调转x,y坐标
	LCD_WriteReg(lcddev.setxcmd,x);
	LCD_WriteReg(lcddev.setycmd,y);
	 
	LCD_WR_REG(lcddev.wramcmd); 
	LCD_WR_DATA(color); 
}	 


//设置LCD显示方向（6804不支持横屏显示）
//dir:0,竖屏；1,横屏
void LCD_Display_Dir(u8 dir)
{
	if(lcddev.dir==0)//竖屏
	{
		lcddev.dir=0;//竖屏
		lcddev.width=220;
		lcddev.height=176;
		lcddev.wramcmd=R34;
		lcddev.setxcmd=R32;
		lcddev.setycmd=R33;  
	}else 
	{	  
		lcddev.dir=1;//横屏
		lcddev.width=176;
		lcddev.height=220;
		lcddev.wramcmd=R34;
		lcddev.setxcmd=R33;
		lcddev.setycmd=R32;  
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}
void IO_init(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);//使能PORTA B
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;				     //13 15  推挽输出  
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试! 
void LCD_Init(void)
{ 										  
//	GPIO_InitTypeDef GPIO_InitStructure;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);//使能PORTA B
//
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				      //PA0 推挽输出 RESET
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  //推挽输出
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_6 |GPIO_Pin_13 | GPIO_Pin_15;				     //PB1 6 14 15  推挽输出  
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
  //LCD复位 
	LCD_RST_H();
	osDelay(10);
	LCD_RST_L();
	osDelay(20);
	LCD_RST_H();
	osDelay(20);
	
	//************* Start Initial Sequence **********// 
	LCD_WriteReg(0x0001, 0x0100);  // set SS and SM bit 
	LCD_WriteReg(0x0002, 0x0700);  // set 1 line inversion 
	LCD_WriteReg(0x0003, 0x1030);  // set GRAM write direction and BGR=1. 
	LCD_WriteReg(0x0004, 0x0000);  // Resize register 
	LCD_WriteReg(0x0008, 0x0202);     // set the back porch and front porch 
	LCD_WriteReg(0x0009, 0x0000);  // set non-display area refresh cycle ISC[3:0] 
	LCD_WriteReg(0x000A, 0x0000);  // FMARK function 
	LCD_WriteReg(0x000C, 0x0000); // RGB interface setting 
	LCD_WriteReg(0x000D, 0x0000);  // Frame marker Position 
	LCD_WriteReg(0x000F, 0x0000); // RGB interface polarity 
	//*************Power On sequence ****************// 
	LCD_WriteReg(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB 
	LCD_WriteReg(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0] 
	LCD_WriteReg(0x0012, 0x0000); // VREG1OUT voltage 
	LCD_WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude 
	LCD_WriteReg(0x0007, 0x0001); 
	osDelay(200); // Dis-charge capacitor power voltage 
	LCD_WriteReg(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB 
	LCD_WriteReg(0x0011, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0] 
	osDelay(50);   // Delay 50ms 
	LCD_WriteReg(0x0012, 0x009D); // Internal reference voltage= Vci; 
	osDelay(50);   // Delay 50ms 
	LCD_WriteReg(0x0013, 0x1900); // Set VDV[4:0] for VCOM amplitude 
	LCD_WriteReg(0x0029, 0x002C); // Set VCM[5:0] for VCOMH 
	LCD_WriteReg(0x002B, 0x000D); // Set Frame Rate 
	osDelay(50); // Delay 50ms 
	LCD_WriteReg(0x0020, 0x0000); // GRAM horizontal Address 
	LCD_WriteReg(0x0021, 0x0000); // GRAM Vertical Address 
	// ----------- Adjust the Gamma   Curve ----------// 
	LCD_WriteReg(0x0030, 0x0007); 
	LCD_WriteReg(0x0031, 0x0303); 
	LCD_WriteReg(0x0032, 0x0003); 
	LCD_WriteReg(0x0035, 0x0206); 
	LCD_WriteReg(0x0036, 0x0008); 
	LCD_WriteReg(0x0037, 0x0406); 
	LCD_WriteReg(0x0038, 0x0304); 
	LCD_WriteReg(0x0039, 0x0007); 
	LCD_WriteReg(0x003C, 0x0602); 
	LCD_WriteReg(0x003D, 0x0008); 
	//------------------ Set GRAM area ---------------// 220*176
	LCD_WriteReg(0x0050, 0x000A);  // Horizontal GRAM Start Address 
	LCD_WriteReg(0x0051, 0x00E5);  // Horizontal GRAM End Address 
	LCD_WriteReg(0x0052, 0x0000);  // Vertical GRAM Start Address 
	LCD_WriteReg(0x0053, 0x00AF);  // Vertical GRAM Start Address 
	LCD_WriteReg(0x0060, 0xA700);  // Gate Scan Line 
	LCD_WriteReg(0x0061, 0x0001);  // NDL,VLE, REV 
	LCD_WriteReg(0x006A, 0x0000);  // set scrolling line 
	//-------------- Partial Display Control ---------// 
	LCD_WriteReg(0x0080, 0x0000); 
	LCD_WriteReg(0x0081, 0x0000); 
	LCD_WriteReg(0x0082, 0x0000); 
	LCD_WriteReg(0x0083, 0x0000); 
	LCD_WriteReg(0x0084, 0x0000); 
	LCD_WriteReg(0x0085, 0x0000); 
	//-------------- Panel Control -------------------// 
	LCD_WriteReg(0x0090, 0x0010); 
	LCD_WriteReg(0x0092, 0x0600); 
	LCD_WriteReg(0x0007, 0x0133);  // 262K color and display ON 
	
	LCD_Display_Dir(0);		 	//默认为竖屏
	//LCD_LED=1;					//点亮背光
	LCD_Clear(WHITE);
}  
//清屏函数
//color:要清屏的填充色
void LCD_Clear(u16 color)
{
	u32 index=0;      
	u32 totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 	//得到总点数
	LCD_SetCursor(0x00,0x0000);	//设置光标位置 
	LCD_WriteRAM_Prepare();     //开始写入GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD_WR_DATA(color);	   
	}
}  
//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{          
	u16 i,j;
	u16 xlen=0;
	xlen=ex-sx+1;	   
	for(i=sy;i<=ey;i++)
	{
	 	LCD_SetCursor(sx,i);      				//设置光标位置 
		LCD_WriteRAM_Prepare();     			//开始写入GRAM	  
		for(j=0;j<xlen;j++)LCD_WR_DATA(color);	//设置光标位置 	    
	}
}  
//在指定区域内填充指定颜色块			 
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)   
//color:要填充的颜色
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{  
	u16 height,width;
	u16 i,j;
	width=ex-sx+1; 		//得到填充的宽度
	height=ey-sy+1;		//高度
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//设置光标位置 
		LCD_WriteRAM_Prepare();     //开始写入GRAM
		for(j=0;j<width;j++)LCD_WR_DATA(color[i*height+j]);//写入数据 
	}	  
}  
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_DrawPoint(uRow,uCol);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}    
//画矩形	  
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		LCD_DrawPoint(x0+a,y0-b);             //5
 		LCD_DrawPoint(x0+b,y0-a);             //0           
		LCD_DrawPoint(x0+b,y0+a);             //4               
		LCD_DrawPoint(x0+a,y0+b);             //6 
		LCD_DrawPoint(x0-a,y0+b);             //1       
 		LCD_DrawPoint(x0-b,y0+a);             
		LCD_DrawPoint(x0-a,y0-b);             //2             
  		LCD_DrawPoint(x0-b,y0-a);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 									  
//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    u8 temp,t1,t;
	u16 y0=y;
	u16 colortemp=POINT_COLOR;      			     
	//设置窗口		   
	num=num-' ';//得到偏移后的值
	if(!mode) //非叠加方式
	{
	    for(t=0;t<size;t++)
	    {   
			if(size==12)temp=asc2_1206[num][t];  //调用1206字体
			else temp=asc2_1608[num][t];		 //调用1608字体 	                          
	        for(t1=0;t1<8;t1++)
			{			    
		        if(temp&0x80)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				LCD_DrawPoint(x,y);	
				temp<<=1;
				y++;
				if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }    
	}else//叠加方式
	{
	    for(t=0;t<size;t++)
	    {   
			if(size==12)temp=asc2_1206[num][t];  //调用1206字体
			else temp=asc2_1608[num][t];		 //调用1608字体 	                          
	        for(t1=0;t1<8;t1++)
			{			    
		        if(temp&0x80)LCD_DrawPoint(x,y); 
				temp<<=1;
				y++;
				if(x>=lcddev.height){POINT_COLOR=colortemp;return;}//超区域了
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//超区域了
					break;
				}
			}  	 
	    }     
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   
//m^n函数
//返回值:m^n次方.
u32 LCD_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//显示数字,高位为0,则不显示
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//color:颜色 
//num:数值(0~4294967295);	 
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
	}
} 
//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);	 
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)
{  
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/LCD_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
				else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
 				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
	}
} 
//显示字符串
//x,y:起点坐标
//width,height:区域大小  
//size:字体大小
//*p:字符串起始地址		  
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
	u8 x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}

//设置窗口
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height)
{  
  u8 hsareg,heareg,vsareg,veareg;
	u16 hsaval,heaval,vsaval,veaval; 
	u16 twidth,theight;
	twidth=sx+width-1;
	theight=sy+height-1;
	
	if(lcddev.dir==1)//横屏
	{
		//窗口值
		hsaval=sy;				
		heaval=theight;
		vsaval=lcddev.width-twidth-1;
		veaval=lcddev.width-sx-1;				
	}else
	{ 
		hsaval=sx;				
		heaval=twidth;
		vsaval=sy;
		veaval=theight;
	} 
	hsareg=0X50;heareg=0X51;//水平方向窗口寄存器
	vsareg=0X52;veareg=0X53;//垂直方向窗口寄存器	   							  
	//设置寄存器值
	LCD_WriteReg(hsareg,hsaval+0x0A);//起点是从0X0A开始的
	LCD_WriteReg(heareg,heaval+0x0A);
	LCD_WriteReg(vsareg,vsaval);
	LCD_WriteReg(veareg,veaval);		
	LCD_SetCursor(sx,sy);	//设置光标位置  
}






























