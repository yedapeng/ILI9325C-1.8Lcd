
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


				 
//LCD�Ļ�����ɫ�ͱ���ɫ	   
u16 POINT_COLOR=0x0000;	//������ɫ
u16 BACK_COLOR=0xFFFF;  //����ɫ 

//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;
	
//��8λ����
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
//д�Ĵ�������
//regval:�Ĵ���ֵ
void LCD_WR_REG(u8 regval)
{ 
	LCD_CS_L();
	LCD_SDA_L();//��ָ��
	LCD_SCL_L(); 
	LCD_SCL_H();

	SPI_SendData(regval);
  LCD_CS_H();
}
//дLCD����
//data:Ҫд���ֵ
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
//д�Ĵ���
//LCD_Reg:�Ĵ�����ַ
//LCD_RegValue:Ҫд�������
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}	      
//��ʼдGRAM
void LCD_WriteRAM_Prepare(void)
{
 	LCD_WR_REG(0x22);
}	 
//LCDдGRAM
//RGB_Code:��ɫֵ
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code);
}

//��mdk -O1ʱ���Ż�ʱ��Ҫ����
//��ʱi
void opt_delay(u8 i)
{
	while(i--);
}

//LCD������ʾ
void LCD_DisplayOn(void)
{					   
	LCD_WriteReg(R7,0x0173); 			//������ʾ
}	 
//LCD�ر���ʾ
void LCD_DisplayOff(void)
{	   
	LCD_WriteReg(R7,0x0);//�ر���ʾ 
}   
//���ù��λ��
//Xpos:������
//Ypos:������
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	 
		if(lcddev.dir==1)Xpos=lcddev.width-1-Xpos;//������ʵ���ǵ�תx,y����
		LCD_WriteReg(lcddev.setxcmd, Xpos+0x0A);//X���Ǵ�0���꿪ʼ���Ǵ�0X0A���꿪ʼ��
		LCD_WriteReg(lcddev.setycmd, Ypos);
} 		 
//����LCD���Զ�ɨ�跽��
//ע��:�����������ܻ��ܵ��˺������õ�Ӱ��(������9341/6804����������),
//����,һ������ΪL2R_U2D����,�������Ϊ����ɨ�跽ʽ,���ܵ�����ʾ������.
//dir:0~7,����8������(���嶨���lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341��IC�Ѿ�ʵ�ʲ���	   	   
void LCD_Scan_Dir(u8 dir)
{
	u16 regval=0;
	u8 dirreg=0;
	switch(dir)
	{
		case L2R_U2D://������,���ϵ���
		 regval|=(1<<5)|(1<<4)|(0<<3); 
		 break;
		case L2R_D2U://������,���µ���
		 regval|=(0<<5)|(1<<4)|(0<<3); 
		 break;
		case R2L_U2D://���ҵ���,���ϵ���
		 regval|=(1<<5)|(0<<4)|(0<<3);
		 break;
		case R2L_D2U://���ҵ���,���µ���
		 regval|=(0<<5)|(0<<4)|(0<<3); 
		 break;	 
		case U2D_L2R://���ϵ���,������
		 regval|=(1<<5)|(1<<4)|(1<<3); 
		 break;
		case U2D_R2L://���ϵ���,���ҵ���
		 regval|=(1<<5)|(0<<4)|(1<<3); 
		 break;
		case D2U_L2R://���µ���,������
		 regval|=(0<<5)|(1<<4)|(1<<3); 
		 break;
		case D2U_R2L://���µ���,���ҵ���
		 regval|=(0<<5)|(0<<4)|(1<<3); 
		 break;	 
	}
	dirreg=0X03;
	regval|=1<<12;  
	LCD_WriteReg(dirreg,regval);
}   
//����
//x,y:����
//POINT_COLOR:�˵����ɫ
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);		//���ù��λ�� 
	LCD_WriteRAM_Prepare();	//��ʼд��GRAM
	LCD_WR_DATA(POINT_COLOR); 
}
//���ٻ���
//x,y:����
//color:��ɫ
void LCD_Fast_DrawPoint(u16 x,u16 y,u16 color)
{	   
	if(lcddev.dir==1)x=lcddev.width-1-x;//������ʵ���ǵ�תx,y����
	LCD_WriteReg(lcddev.setxcmd,x);
	LCD_WriteReg(lcddev.setycmd,y);
	 
	LCD_WR_REG(lcddev.wramcmd); 
	LCD_WR_DATA(color); 
}	 


//����LCD��ʾ����6804��֧�ֺ�����ʾ��
//dir:0,������1,����
void LCD_Display_Dir(u8 dir)
{
	if(lcddev.dir==0)//����
	{
		lcddev.dir=0;//����
		lcddev.width=220;
		lcddev.height=176;
		lcddev.wramcmd=R34;
		lcddev.setxcmd=R32;
		lcddev.setycmd=R33;  
	}else 
	{	  
		lcddev.dir=1;//����
		lcddev.width=176;
		lcddev.height=220;
		lcddev.wramcmd=R34;
		lcddev.setxcmd=R33;
		lcddev.setycmd=R32;  
	} 
	LCD_Scan_Dir(DFT_SCAN_DIR);	//Ĭ��ɨ�跽��
}
void IO_init(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTA B
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;				     //13 15  �������  
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
//��ʼ��lcd
//�ó�ʼ���������Գ�ʼ������ILI93XXҺ��,�������������ǻ���ILI9320��!!!
//�������ͺŵ�����оƬ��û�в���! 
void LCD_Init(void)
{ 										  
//	GPIO_InitTypeDef GPIO_InitStructure;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTA B
//
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				      //PA0 ������� RESET
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		  //�������
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_6 |GPIO_Pin_13 | GPIO_Pin_15;				     //PB1 6 14 15  �������  
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
  //LCD��λ 
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
	
	LCD_Display_Dir(0);		 	//Ĭ��Ϊ����
	//LCD_LED=1;					//��������
	LCD_Clear(WHITE);
}  
//��������
//color:Ҫ���������ɫ
void LCD_Clear(u16 color)
{
	u32 index=0;      
	u32 totalpoint=lcddev.width;
	totalpoint*=lcddev.height; 	//�õ��ܵ���
	LCD_SetCursor(0x00,0x0000);	//���ù��λ�� 
	LCD_WriteRAM_Prepare();     //��ʼд��GRAM	 	  
	for(index=0;index<totalpoint;index++)
	{
		LCD_WR_DATA(color);	   
	}
}  
//��ָ����������䵥����ɫ
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{          
	u16 i,j;
	u16 xlen=0;
	xlen=ex-sx+1;	   
	for(i=sy;i<=ey;i++)
	{
	 	LCD_SetCursor(sx,i);      				//���ù��λ�� 
		LCD_WriteRAM_Prepare();     			//��ʼд��GRAM	  
		for(j=0;j<xlen;j++)LCD_WR_DATA(color);	//���ù��λ�� 	    
	}
}  
//��ָ�����������ָ����ɫ��			 
//(sx,sy),(ex,ey):�����ζԽ�����,�����СΪ:(ex-sx+1)*(ey-sy+1)   
//color:Ҫ������ɫ
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{  
	u16 height,width;
	u16 i,j;
	width=ex-sx+1; 		//�õ����Ŀ��
	height=ey-sy+1;		//�߶�
 	for(i=0;i<height;i++)
	{
 		LCD_SetCursor(sx,sy+i);   	//���ù��λ�� 
		LCD_WriteRAM_Prepare();     //��ʼд��GRAM
		for(j=0;j<width;j++)LCD_WR_DATA(color[i*height+j]);//д������ 
	}	  
}  
//����
//x1,y1:�������
//x2,y2:�յ�����  
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		LCD_DrawPoint(uRow,uCol);//���� 
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
//������	  
//(x1,y1),(x2,y2):���εĶԽ�����
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}
//��ָ��λ�û�һ��ָ����С��Բ
//(x,y):���ĵ�
//r    :�뾶
void Draw_Circle(u16 x0,u16 y0,u8 r)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //�ж��¸���λ�õı�־
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
		//ʹ��Bresenham�㷨��Բ     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 									  
//��ָ��λ����ʾһ���ַ�
//x,y:��ʼ����
//num:Ҫ��ʾ���ַ�:" "--->"~"
//size:�����С 12/16
//mode:���ӷ�ʽ(1)���Ƿǵ��ӷ�ʽ(0)
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{  							  
    u8 temp,t1,t;
	u16 y0=y;
	u16 colortemp=POINT_COLOR;      			     
	//���ô���		   
	num=num-' ';//�õ�ƫ�ƺ��ֵ
	if(!mode) //�ǵ��ӷ�ʽ
	{
	    for(t=0;t<size;t++)
	    {   
			if(size==12)temp=asc2_1206[num][t];  //����1206����
			else temp=asc2_1608[num][t];		 //����1608���� 	                          
	        for(t1=0;t1<8;t1++)
			{			    
		        if(temp&0x80)POINT_COLOR=colortemp;
				else POINT_COLOR=BACK_COLOR;
				LCD_DrawPoint(x,y);	
				temp<<=1;
				y++;
				if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}  	 
	    }    
	}else//���ӷ�ʽ
	{
	    for(t=0;t<size;t++)
	    {   
			if(size==12)temp=asc2_1206[num][t];  //����1206����
			else temp=asc2_1608[num][t];		 //����1608���� 	                          
	        for(t1=0;t1<8;t1++)
			{			    
		        if(temp&0x80)LCD_DrawPoint(x,y); 
				temp<<=1;
				y++;
				if(x>=lcddev.height){POINT_COLOR=colortemp;return;}//��������
				if((y-y0)==size)
				{
					y=y0;
					x++;
					if(x>=lcddev.width){POINT_COLOR=colortemp;return;}//��������
					break;
				}
			}  	 
	    }     
	}
	POINT_COLOR=colortemp;	    	   	 	  
}   
//m^n����
//����ֵ:m^n�η�.
u32 LCD_Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}			 
//��ʾ����,��λΪ0,����ʾ
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//color:��ɫ 
//num:��ֵ(0~4294967295);	 
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
//��ʾ����,��λΪ0,������ʾ
//x,y:�������
//num:��ֵ(0~999999999);	 
//len:����(��Ҫ��ʾ��λ��)
//size:�����С
//mode:
//[7]:0,�����;1,���0.
//[6:1]:����
//[0]:0,�ǵ�����ʾ;1,������ʾ.
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
//��ʾ�ַ���
//x,y:�������
//width,height:�����С  
//size:�����С
//*p:�ַ�����ʼ��ַ		  
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{         
	u8 x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//�˳�
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }  
}

//���ô���
void LCD_Set_Window(u16 sx,u16 sy,u16 width,u16 height)
{  
  u8 hsareg,heareg,vsareg,veareg;
	u16 hsaval,heaval,vsaval,veaval; 
	u16 twidth,theight;
	twidth=sx+width-1;
	theight=sy+height-1;
	
	if(lcddev.dir==1)//����
	{
		//����ֵ
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
	hsareg=0X50;heareg=0X51;//ˮƽ���򴰿ڼĴ���
	vsareg=0X52;veareg=0X53;//��ֱ���򴰿ڼĴ���	   							  
	//���üĴ���ֵ
	LCD_WriteReg(hsareg,hsaval+0x0A);//����Ǵ�0X0A��ʼ��
	LCD_WriteReg(heareg,heaval+0x0A);
	LCD_WriteReg(vsareg,vsaval);
	LCD_WriteReg(veareg,veaval);		
	LCD_SetCursor(sx,sy);	//���ù��λ��  
}






























