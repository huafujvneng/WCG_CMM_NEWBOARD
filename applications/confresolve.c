/**************************************************************************
Copyright (C), 2012, XJ ELECTRIC Co., LTD.
文件名  ：  confresolve.c
作者    ：
项目名称：   
功能    ：  配置文件解析处理
创建日期：   
备注    ：
修改记录：
**************************************************************************/
char filebuf[10],filebuf1[10];
#include "confresolve.h"
#include "Applications/tcp_debug.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 

void filetoconf(uint8_t *confbuf)
{
	uint8_t tempdata;
	//strsep();
	
char str[11] = "abcdefgclmn";
char *p = str;
char *key_point;
	char *poutput;

	
uint8_t i;
key_point = strtok(p,"c");
	//key_point = strtok_r(p,);
	
	if(p)
	{
		for(i = 0; i< sizeof(p); i++)
		{
			filebuf[i] = *p++;
		}
	}
	p = strtok(NULL, "c");
	if(p)
	{
		for(i = 0; i< sizeof(p); i++)
		{
			filebuf1[i] = *p++;
		}
		//strncpy(filebuf1,p,sizeof(p));
		
	}	
	
	
	/*while(p)
	{
		while(key_point = strtok(p,"cd"))
		{
			if(*key_point == 1)
			continue;
			else
				break;
		}
		for(i = 0; i< 10; i++)
		{
			filebuf[i] = *p++;
		}
		
		
	}*/
	

}

int maintest(void)
{
  int j,in = 0;
  char buffer[100] = "Fred male 25,John male 62,Anna female 16";
  char *p[20];
  char *buf = buffer;
  char *outer_ptr = NULL;
  char *inner_ptr = NULL;
  while ((p[in] = strtok_r(buf, ",", &outer_ptr)) != NULL)
  {
    buf = p[in];
    while ((p[in] = strtok_r(buf, " ", &inner_ptr)) != NULL)
    {
      in++;
      buf = NULL;
    }
    buf = NULL;
  }
  printf("Here we have %d strings\n", in);
  for (j = 0; j < in; j++)
  {
    printf(">%s<\n", p[j]);
  }
  return 0;
}

