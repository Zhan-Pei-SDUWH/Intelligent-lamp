#include <reg52.h>
#include <intrins.h>          //����_nop_��ָ��
#define uchar unsigned char		// unsigned char����uchar����
#define uint unsigned int		  // unsigned int����uint����
	
sbit LED     = P1^0;					// ģʽָʾ�ƣ��������Զ�ģʽ��������ֶ�ģʽ
sbit Key1    = P1^1;					// ģʽ�л�����
sbit Key2    = P1^2; 					// ���ȼ��ٰ���     
sbit Key3    = P1^3;					// �������Ӱ���
sbit Lighten = P1^4; 	        // ̨�ƿ������ţ�Ĭ��Ϊ�ߵ�ƽ���͵�ƽʱ����С�ƣ�
sbit ADC_CLK = P2^0; 					// ADC��CLK����
sbit ADC_DAT = P2^1; 					// ADC��DI/DO���ţ���Ϊ��·����ͬʱ���������Խ��䲢����ͬһ���ڣ�
sbit Human   = P2^2;					// ���������ģ�飨��Ӧ����ʱ����ߵ�ƽ��
sbit ADC_CS  = P2^3; 					// ADC��CS����

uchar Count=0;								// ��������
uchar Light;									// ���ȱ�����0�����9��������һ��10���������ִ���һ�������ڵ�����ʱ����
uint  Time=0;								  // ��ʱ���������ڼ�ʱ���û��⵽����


void Delay(uint z)            //��ʱ������time��Ҫ��ʱ�ĺ�����
{
	uint x,y;
	for(x=0;x<z;x++)
		for(y=0;y<110;y++);       
}


void Timer()          // ��ʱ����ʼ������
{
	TMOD = 0x01;				// ʹ�ö�ʱ��0��������ʽ1	 
	TH0  = 252;					// ����ʱ��0��TH0װ��ֵ
	TL0  = 24;					// ����ʱ��0��TL0װ��ֵ	
	ET0  = 1;						// ��ʱ��0�ж�ʹ��
	EA   = 1;						// �����ж�
	TR0	 = 1;						// ������ʱ��0
}

void Pluse()                  //ADC��ʱ�����庯��                            
{
	_nop_();                    //����һ���������ڵ���ʱ
	ADC_CLK = 1;
	_nop_();
	ADC_CLK = 0;
}


uchar Data()          // ��ȡADCת�����
                      
{ 
	uchar i;
	uchar dat1=0;
	uchar dat2=0;
	
	ADC_CLK = 0;				// ��ƽ��ʼ��
	ADC_DAT = 1;
	_nop_();
	ADC_CS = 0;
	Pluse();					  // ��ʼ�ź� 
	ADC_DAT = 1;
	Pluse();					  // ͨ��ѡ��ĵ�һλ
	ADC_DAT = 0;      
	Pluse();					  // ͨ��ѡ��ĵڶ�λ
	ADC_DAT = 1;
	
	for(i=0;i<8;i++)		// ��һ�ζ�ȡ
	{
		dat1<<=1;
		Pluse();
		if(ADC_DAT)
			dat1=dat1|0x01;
		else
			dat1=dat1|0x00;
	}
	
	for(i=0;i<8;i++)		// �ڶ��ζ�ȡ
	{
		dat2>>= 1;
		if(ADC_DAT)
			dat2=dat2|0x80;
		else
			dat2=dat2|0x00;
		Pluse();
	}
	
	_nop_();						// �����˴δ���
	ADC_DAT = 1;
	ADC_CLK = 1;
	ADC_CS  = 1;   

	if(dat1==dat2)			// ���زɼ����
		return dat1;
	else
		return 0;
} 


void Manual()         // �ֶ�����
{
	if(Key2==0)					// ���ȼ��٣�����������ʱ,��Ӧ����Ϊ�͵�ƽ��
	{
		if(Light>0)			  // ֻ�е�ǰ���Ȳ�Ϊ���ʱ���ܼ�������
		{
			Light--;				
			Delay(500);		  // ��ʱ0.5�루�����ס������ÿ��0.5s����һ����
		}
	}
	
	
	if(Key3==0)					// ��������
	{
		if(Light<9)			  // ֻҪ��ǰ���Ȳ�Ϊ��߲�����������
		{
			Light++;				
			Delay(500);		  
		}
	}
}


void Auto(uchar num)                    // �Զ����� 
	                                      //����Ϊ��ѹֵ���Ϊ5V��������λ���Ϊ255����x/5=y/255��xΪ��ѹֵ��yΪ��Ӧ����λ��
{
	if(num<60)														// ����
		Light=9;
	else if((num>66)&&(num<82))						// �ڶ���
		Light=8;
	else if((num>88)&&(num<104))					// ������
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
	else if(num>242)										 // �
		Light=0;
}






// ������

void main()
 {
	uchar value;
	Timer(); 					   // ��ʱ����ʼ��
	LED=0;							 // �ϵ��Ĭ��Ϊ�Զ�ģʽ
	value=Data();		     // ��ȡAD�ɼ��������������ǿ�ȣ�
	Auto(value);			   

	 
		if(Key1==0)				 // �������1������ȥ
		{
			LED=~LED;				 // �л�LED��״̬
			if(LED==0)			 
			{
				value=Data();	 
				Auto(value);	 	
			
			}
			Delay(20);			 // ��ʱ����
			while(!Key1);		 // �ȴ������ͷ�
			Delay(20);			 
			
		
		if(LED==1)				 //ģʽ�ж�
		{
			Manual();			  
		}
		else							 
		{
			if(Time<60000)	 // ���Ϊ�Զ�ģʽ������60�����Ƿ�����
			{
				value=Data();	 // ��ȡAD�ɼ�����������й���Խ��������ĵ�ѹֵԽ�ߣ�
				Auto(value);	 // �����Զ�����	
				Delay(200);
			}
		}
		
		
		if(Human==1)       //����Ƿ�����
		{
			Time=0;					 // ��⵽���ˣ����60���ʱ����
		}
		if(Time>60000)						
		{
			Time=60000;			 // ��ʱ����60s��̶�����Ϊ�䷶ΧΪ0~65536��
			Light=0;			   // ���60���ⲻ�����ˣ����̨��Ϩ��
		}
	}
}




void Timer0(void) interrupt 1   //��ʱ�����ʱ�����жϺ���

{
	TH0  = 252;						       
	TL0  = 24;						       
	
	if(LED==0)
	{
		Time++;						         // ���ڼ����û��
	}
	
	Count++;							       // ÿ1���룬Count������1
	
	if(Count==10)				         // ���Count�ӵ�10��
	{
		Count=0;					       	 // ��Count���㣬������һ�ֵļ���
		if(Light!=0)				       // ���˵̨�Ʋ������(Ϩ��)
		{
			Lighten=0;						   // ���̨�Ƶ���
		}
	}
	if(Count==Light)		         // ���Count��������Lightһ����
	{
		if(Light!=9)				       // ���˵̨�Ʋ���������
		{
			Lighten=1;						   // ���̨��Ϩ��
			                         
		}
	}
}




