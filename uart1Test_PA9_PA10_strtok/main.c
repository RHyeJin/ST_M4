/******************************************************************************
  포트 연결:
    1)  ARM Cortex-M4 모듈의 포트A의 9~10번핀을(PA9~PA10) 2핀 케이블로
        UART모듈의 RXD, TXD 핀에 연결한다.
******************************************************************************/
// stm32f4xx의 각 레지스터들을 정의한 헤더파일
#include "stm32f4xx.h"// 기본적으로 포함되어야함

#include <stdio.h>
#include <string.h>
#include <stdlib.h>//atoi함수

#define CMD_SIZE 50//한번에 받을 데이터 양
#define ARR_CNT 5  //포인트배열의 인덱스개수


volatile unsigned char rx1Flag = 0;//인터럽트서비스 루틴과 메인함수간에 공유하는 변수 전역변수로 꼭 설정해야함 volatile를 써라 운영체제가 없기때문에 
char rx1Data[50];//유아트번호로 구분 두세개 쓸수도 있으니
//함수원형들 컴파일러가 위에서 부터 동작해서 헤더파일에 써줘야함 
void Serial_Event(); //루프를 돌면서 백슬레시엔까지 데이터를 처리하기위한 
void Serial_Send(unsigned char t);//한바이트를 전송하는 함수
void Serial_Send_String(char* s);// 문자열을 전송하는 함수 널문자나올때까지 전송하는 식으로 호출
int putchar(int ch);//프린트에프쓰기위해 넣은 함수
void print_2d1(double number);// 프린트에프 하나쓰면 20K메모리가 쓰임 딜레이 문제 디버깅할때만 쓰고 지워야함 실제로 문자를 출력하고 싶으면 함수를 만들어써라
//정수두자리 소수한자리
int main()
{
  GPIO_InitTypeDef   GPIO_InitStructure; //GPIO초기값설정
  USART_InitTypeDef USART_InitStructure;//유아트를 초기화하기위한 구조체
  NVIC_InitTypeDef   NVIC_InitStructure; //중첩가능한 
//초기화안하면 쓰레기값

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //모든 장비에 클럭제공
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;                   //LED쓸거니깐
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                 //0이냐 1이냐
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;         //노이즈생길수
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;            //내부풀업풀다운 꼭 설정
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;        
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;// 부가기능쓰려함
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //위에하고 똑같이 되어있어서 초기화할 필요없음
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//기본상태가 하이여서 풀업풀다운
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure); //실제 부가기능을 뭘쓸지는 밑에서 할당할거
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);     //USART1_TX  <- 유아트입장 GPIO를 거쳐야해서 다시초기화하는과정
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);    //USART1_RX
  
//인터럽트 컨트롤러 NVIC 와 각각주변장치마다 다 인터럽트를 할당해줘야함 유아트인터럽트를 마스크낫마스크 막을건지 말건지 
  //우선순위가 높은 인터럽트를 먼저부를때 선점이라고함 그래서 인터럽트중첩가능
  //인터럽트 enable 및 Priority 설정.
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //우선순위 선점허용할지말지 그룹나누는것 NVIC_PriorityGroup_2 : 2비트 선점 2비트 서브 로 쓰겠다
  //같은그룹이면 선점안함. 서브는 동시에 인터럽트가 발생했을때 서브프라이어티 우선순위가 더높은(값이 0 ) 실행 10 01 10 01  
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; //이미 인터럽트서비스루틴은 다정의되어있음 STM32F4XX.H 헤더에 정의되어있음
  //STM3214XX_IT.C 코어에서 발생하는 핸들러 정의 내가 막을수없는 인터럽트 2개 : 배터리다되면 서버에 알리고 다저장하고 꺼야하니깐,  
  //STARTUP_STM3214_41XXX.S 인터럽트 백터 테이블 DCD 0번지에 4번 지 8번지// 모든 인터럽트 주소 정의되어있음
  // DCD     USART1_IRQHandler                 ; USART1   함수이름은 모두 포인터 첫번째주소가됨 호출시 NVIC가 이주소로 점프 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; // 프리
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01; // 서브 동시에 발생했을때 우선순위 높을애를 쓰겟다.
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 
  NVIC_Init(&NVIC_InitStructure);//

  USART_InitStructure.USART_BaudRate = 115200; //구조체 멤버 변수 사용자 정의대로 정해주는거 //1초에 115200비트 전송
  USART_InitStructure.USART_WordLength = USART_WordLength_8b; //하나의 프레임 5-9비트사이의 값을 전송할수 잇는데 이해하기좋기위해 바이트8 8비트로 보냄
  USART_InitStructure.USART_StopBits = USART_StopBits_1; // 스탑비트로 속도조절 1
  USART_InitStructure.USART_Parity = USART_Parity_No; //페리트 비트는 스탑비트 앞에있음, 페리트 비트를 추가해서 1의개수 짝수혹은 홀수로 만들어줌  한비트가 깨져서 오류가낫는지 체크하는 방식
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;// PC와 모뎀 
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //두개의 보드가 통신하는데 한방향으로 하겟다 핀이부족하거나 필요에따라설정// 전이동통신방식: 둘다 사용하겟다상태
  USART_Init(USART1, &USART_InitStructure);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // USART1 Interrupt enable //USART_ITConfig개별인터럽트 RX소스를 인터럽트하겠다
  USART_Cmd(USART1, ENABLE);//?

  Serial_Send_String("Start Main()\n\r");
  while(1)//168MG
  {
    if(rx1Flag)  // '\r' 까지 입력되면
      Serial_Event();
    //플래그를 나눠서 사용
    //RPOS 이용
  }
}

void Serial_Event() //셋된후
{
  int i=0;
  int num = 0;
  char * pToken;
  char * pArray[ARR_CNT]={0};//캐릭터형 주소저장
  char recvBuf[CMD_SIZE]={0};   //rx에 들어온 데이터를 복사  수신중에 새값이 들어오면 안되니깐    
  strcpy(recvBuf,rx1Data); 

  rx1Flag = 0; // 다시 Rflag 를 0으로 놓는다. 방금처리한 데이터 끝내려구
  Serial_Send_String(recvBuf);  //방금받은 명령어를 그대로 보내줌
  Serial_Send_String("\n\r");//s가 200번지 "S"가들어가있음 널문자아님 //내가설정을 \n(줄바꿈)으로 해놓음 \r 캐리지리턴
  printf("rx : %s\r\n",recvBuf); //디버깅용
  printf("test : %f\r\n",12.3); //디버깅용
  printf("\n\r");//디버깅용
  print_2d1(12.3);//디버깅용
  pToken = strtok(recvBuf,"[@]"); //문자열 @로 나눔(strrok함수)  널문자로 교체해버림 그 주소값 리턴 이위치외에 다시쓸수없음?

  while(pToken != NULL) //포인터변수 0x01이 널이냐
  {
    pArray[i] =  pToken; //0인덱스에 0x01주소값넣겟다
    if(++i >= ARR_CNT)  //
      break;
    pToken = strtok(NULL,"[@]");//계속분리할때는 널이와야함 //PArayy[0] : 0x01 "RHJ_ARM" 첫번째 문자 널이기때문에 아무것도 리턴안됨?
  }
       
  if(!strcmp(pArray[1],"LED")) //널문자나올때까지문자열
  { //동일하면 0리턴
    if(pArray[3] != NULL)              
      num = atoi(pArray[3]); //0이라는 문자열을 숫자로 바꿈 2바이트 (널문자포함)
    else
      num = 0;
    if(!strcmp(pArray[2],"ON"))
    {
     GPIO_SetBits(GPIOC, 0x01 << num); //키자?
    }
    else 
    {
      GPIO_ResetBits(GPIOC, 0x01 << num); 
    }
  }  
}

void Serial_Send(unsigned char t)
{
  USART_SendData(USART1, t); //최하위비트 8비트를 보내야함?
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); //1일될때가 와일문돌겟다 버퍼에 다찰때까지 잠시기다리겟다 RESET 0 SET1 빠져나옴
}

void Serial_Send_String(char* s)
{
  while( *s != '\0') //201번지 데이타 ----->널이면 나감
  {
   Serial_Send((unsigned char)(*s)); //다시받음 형변환
   s++;   //s = s + 1;
  }
}

int putchar(int ch)
{
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1,ch);
	return ch;
}

void USART1_IRQHandler(void)
{
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//인터럽트발생했는지 확인 8비트가 들ㅇ오면 인터럽트발생 우리가 그렇개 설정함 엠프티됏는지 확인
  {
	static int i=0; //처음한번만 초기화 빠져나가도 값유지해야해서
	rx1Data[i] = USART_ReceiveData(USART1); //한비트 읽어옴 0번에 S저장
	if(rx1Data[i] == '\r')   //원하는 명령어를 다읽었을때 
	{
		rx1Data[i] = '\0'; //문자열 완성시킴 50개까지의 문자
		rx1Flag = 1; //하나의 명령어가 수신완료됐다고 표시
		i = 0;//다시 0부터시작하도록
	}
	else
	{
		i++;
	}
  }
}

void print_2d1(double number) //numer:<-12.34		        /* floating-point number xx.x */
{ 
        unsigned int i, j;

	j = (int)(number*10. + 0.5); //반올림하기위한 123.9 INT형변환 123
	i = j / 100;	//i=1 				// 10^1
	if(i == 0) Serial_Send(' '); // 없앰
	else       Serial_Send(i + '0'); //정수를 아스키문자로 만들기위해 출력

	j = j % 100;	//두번째 자리 계산 i=2				// 10^0
	i = j / 10;
	Serial_Send(i + '0'); //2찍고
	Serial_Send('.'); //소수점찍기

	i = j % 10;	//3찍고				// 10^-1
	Serial_Send(i + '0');
      //소수 둘째자리 100나누고 0.005곱하기 
}