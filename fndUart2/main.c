#include "stm32f4xx.h"
#include "uart2.h"
#include <stdio.h>
#include <stdlib.h>
#include "fnd.h"
extern volatile unsigned char rx2Flag;
extern char rx2Data[50];

//void FND_init();
//void display_digit(int positon,int number);
//void display_fnd( int N );
//static void Delay(const uint32_t Count);
// delay 함수

static void Delay(const uint32_t Count)
{
  __IO uint32_t index = 0; 
  for(index = (16800 * Count); index != 0; index--);
}
//
//// 7-세그먼트 폰트를 배열로 지정한다.
//unsigned char Font[18] = {0x3F, 0X06, 0X5B, 0X4F, 
//                                         0X66, 0X6D, 0X7C, 0X07,
//                                         0X7F, 0X67, 0X77, 0X7C, 
//                                         0X39, 0X5E, 0X79, 0X71, 
//                                         0X08, 0X80};
//
//
//void display_fnd( int N )  // Segment 함수 선언
//{
//  int  i ;
//  unsigned char arrayNum[4];
//  int Buff ;
//
//  arrayNum[0] = N /1000;  // 세그먼트에서 사용하는 천의 자리를 추출
//  Buff = N % 1000 ;
//  arrayNum[1] = Buff / 100 ; // 세그먼트에서 사용하는 백의자리 추출
//  Buff = Buff % 100;
//  arrayNum[2] = Buff /10 ;     // 세그먼트에서 사용하는 십의 자리 추출
//  arrayNum[3] =  Buff % 10 ;    // 세그먼트에서 사용하는 일의 자리 추출    
//
//  for( i=0;i<4;i++)
//      display_digit(i,arrayNum[i]);
//}
//void display_digit(int positon,int number)
//{
//   GPIO_Write(GPIOC,(GPIO_ReadInputData(GPIOC) | (0x0f00)));  //LED all off 
//   GPIO_Write(GPIOC,(GPIO_ReadInputData(GPIOC) &  ~( 0x0100 << positon)));
//   GPIO_Write(GPIOC,(GPIO_ReadInputData(GPIOC) &  0xff00 ) | Font[number]);
//   Delay(1);
//}

int main()
{

  int count = 999;
  
  FND_init();
  UART2_init();
  printf("Start main()\n\r");
  
  while(1)
  {
    if(rx2Flag)
    {
      printf("%s\r\n",rx2Data);
      count = atoi(rx2Data);
      rx2Flag = 0;
    }
   display_fnd(count);
   Delay(10);
    count--;
    
    if(count = 10000){count = 0;}
    else if(count = 1000){count = -1000;}
    Delay(1000);
  }
}

