#include <reg52.h>
#include <intrins.h>          //包含_nop_等指令
#define uchar unsigned char		// unsigned char可用uchar代替
#define uint unsigned int		  // unsigned int可用uint代替
	
sbit LED     = P1^0;					// 模式指示灯，亮代表自动模式，灭代表手动模式
sbit Key1    = P1^1;					// 模式切换按键
sbit Key2    = P1^2; 					// 亮度减少按键     
sbit Key3    = P1^3;					// 亮度增加按键
sbit Lighten = P1^4; 	        // 台灯控制引脚（默认为高电平，低电平时点亮小灯）
sbit ADC_CLK = P2^0; 					// ADC的CLK引脚
sbit ADC_DAT = P2^1; 					// ADC的DI/DO引脚（因为两路不会同时工作，所以将其并联至同一串口）
sbit Human   = P2^2;					// 人体红外检测模块（感应到人时输出高电平）
sbit ADC_CS  = P2^3; 					// ADC的CS引脚

uchar Count=0;								// 计数变量
uchar Light;									// 亮度变量，0是最暗，9是最亮，一共10档（该数字代表一个周期内点亮的时长）
uint  Time=0;								  // 计时变量，用于计时多久没检测到有人


void Delay(uint z)            //延时函数，time是要延时的毫秒数
{
	uint x,y;
	for(x=0;x<z;x++)
		for(y=0;y<110;y++);       
}


void Timer()          // 定时器初始化函数
{
	TMOD = 0x01;				// 使用定时器0，工作方式1	 
	TH0  = 252;					// 给定时器0的TH0装初值
	TL0  = 24;					// 给定时器0的TL0装初值	
	ET0  = 1;						// 定时器0中断使能
	EA   = 1;						// 打开总中断
	TR0	 = 1;						// 启动定时器0
}

void Pluse()                  //ADC的时钟脉冲函数                            
{
	_nop_();                    //产生一个机器周期的延时
	ADC_CLK = 1;
	_nop_();
	ADC_CLK = 0;
}


uchar Data()          // 获取ADC转换结果
                      
{ 
	uchar i;
	uchar dat1=0;
	uchar dat2=0;
	
	ADC_CLK = 0;				// 电平初始化
	ADC_DAT = 1;
	_nop_();
	ADC_CS = 0;
	Pluse();					  // 起始信号 
	ADC_DAT = 1;
	Pluse();					  // 通道选择的第一位
	ADC_DAT = 0;      
	Pluse();					  // 通道选择的第二位
	ADC_DAT = 1;
	
	for(i=0;i<8;i++)		// 第一次读取
	{
		dat1<<=1;
		Pluse();
		if(ADC_DAT)
			dat1=dat1|0x01;
		else
			dat1=dat1|0x00;
	}
	
	for(i=0;i<8;i++)		// 第二次读取
	{
		dat2>>= 1;
		if(ADC_DAT)
			dat2=dat2|0x80;
		else
			dat2=dat2|0x00;
		Pluse();
	}
	
	_nop_();						// 结束此次传输
	ADC_DAT = 1;
	ADC_CLK = 1;
	ADC_CS  = 1;   

	if(dat1==dat2)			// 返回采集结果
		return dat1;
	else
		return 0;
} 


void Manual()         // 手动控制
{
	if(Key2==0)					// 亮度减少（当按键按下时,对应引脚为低电平）
	{
		if(Light>0)			  // 只有当前亮度不为最低时才能减少亮度
		{
			Light--;				
			Delay(500);		  // 延时0.5秒（如果按住不动则每过0.5s降低一档）
		}
	}
	
	
	if(Key3==0)					// 亮度增加
	{
		if(Light<9)			  // 只要当前亮度不为最高才能增加亮度
		{
			Light++;				
			Delay(500);		  
		}
	}
}


void Auto(uchar num)                    // 自动控制 
	                                      //（因为电压值最大为5V，二进制位最大为255，则x/5=y/255，x为电压值，y为对应数字位）
{
	if(num<60)														// 最亮
		Light=9;
	else if((num>66)&&(num<82))						// 第二亮
		Light=8;
	else if((num>88)&&(num<104))					// 第三亮
		Light=7;
	else if((num>110)&&(num<126))
		Light=6;
	else if((num>132)&&(num<148))
		Light=5;
	else if((num>154)&&(num<170))
		Light=4;
	else if((num>176)&&(num<192))
		Light=3;
	else if((num>198)&&(num<214))
		Light=2;
	else if((num>220)&&(num<236))
		Light=1;
	else if(num>242)										 // 最暗
		Light=0;
}






// 主函数

void main()
 {
	uchar value;
	Timer(); 					   // 定时器初始化
	LED=0;							 // 上电后默认为自动模式
	value=Data();		     // 获取AD采集结果（环境光照强度）
	Auto(value);			   

	 
		if(Key1==0)				 // 如果按键1被按下去
		{
			LED=~LED;				 // 切换LED灯状态
			if(LED==0)			 
			{
				value=Data();	 
				Auto(value);	 	
			
			}
			Delay(20);			 // 延时消抖
			while(!Key1);		 // 等待按键释放
			Delay(20);			 
			
		
		if(LED==1)				 //模式判断
		{
			Manual();			  
		}
		else							 
		{
			if(Time<60000)	 // 如果为自动模式，则检测60秒内是否有人
			{
				value=Data();	 // 获取AD采集结果（环境中光线越亮所输入的电压值越高）
				Auto(value);	 // 进行自动控制	
				Delay(200);
			}
		}
		
		
		if(Human==1)       //检测是否有人
		{
			Time=0;					 // 检测到有人，则把60秒计时清零
		}
		if(Time>60000)						
		{
			Time=60000;			 // 计时大于60s后固定（因为其范围为0~65536）
			Light=0;			   // 如果60秒检测不到有人，则把台灯熄灭
		}
	}
}




void Timer0(void) interrupt 1   //定时器溢出时触发中断函数

{
	TH0  = 252;						       
	TL0  = 24;						       
	
	if(LED==0)
	{
		Time++;						         // 用于检测多久没人
	}
	
	Count++;							       // 每1毫秒，Count变量加1
	
	if(Count==10)				         // 如果Count加到10了
	{
		Count=0;					       	 // 则将Count清零，进入新一轮的计数
		if(Light!=0)				       // 如果说台灯不是最暗的(熄灭)
		{
			Lighten=0;						   // 则把台灯点亮
		}
	}
	if(Count==Light)		         // 如果Count计数到和Light一样了
	{
		if(Light!=9)				       // 如果说台灯不是最亮的
		{
			Lighten=1;						   // 则把台灯熄灭
			                         
		}
	}
}




