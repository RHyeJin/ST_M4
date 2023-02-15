#ifndef __UART2_H__ //정의가안됐으면 이렇게 정의해라 (헤더파일의 중복을 회피하기위해 반드시 넣어야한다) //프로젝트에 헤더파일 추가할필요없음 이미 c파일 헤더에 적었기때문에
#define __UART2_H__    

void UART2_init();
void Serial2_Event();
void Serial2_Send(unsigned char t);
void Serial2_Send_String(char* s);
int putchar(int ch);            //printf() 사용시 필요
void print_2d1(double number);

#endif 