#include <includes.h>
#include <bsp_key.h> 


extern OS_SEM SemOfKey;          //��־KEY1�Ƿ񱻵����Ķ�ֵ�ź���
extern CPU_INT32U tim_cnt;
extern CPU_INT32U stoptim_cnt;
extern   OS_TCB   AppTaskGUIBaseTCB;
extern OS_TMR      my_tmr;   //���������ʱ������
void  AppTaskTouchScan  (void )
{

	OS_ERR  os_err;

	while(1)
	{
			GUI_TOUCH_Exec();
			LED2_TOGGLE ;
			//��ʱ����������������ѭ���ڶ�Ӧ������1ms��ʱ
		  //�ر��Ǹ����ȼ�������������ʱ�����������ȼ�������ܻ��޻���ִ��
			OSTimeDly(10, OS_OPT_TIME_DLY,&os_err);
	}
}
	

void  AppTaskTmr_ ( void * p_arg )
{

 	OS_ERR           err;

	CPU_SR_ALLOC();                                       //ʹ�õ��ٽ�Σ��ڹ�/���ж�ʱ��ʱ����ú꣬�ú������Ͷ���һ���ֲ���
                                                        //�������ڱ�����ж�ǰ�� CPU ״̬�Ĵ��� SR���ٽ�ι��ж�ֻ�豣��SR��
                                                        //�����ж�ʱ����ֵ��ԭ��
 (void)p_arg;

	while (DEF_TRUE) {                                    //�����壬ͨ����д��һ����ѭ��    
		
		OSTimeDly ( 1000, OS_OPT_TIME_DLY, & err );         //��ʱ1000��ʱ�ӽ��ģ�1s��
		tim_cnt++;
		OS_CRITICAL_ENTER();                                //�����ٽ�Σ���ϣ�����洮�ڴ�ӡ�⵽�ж�

		printf("\nTime=%02d:%02d",tim_cnt/60,tim_cnt%60);
		OS_CRITICAL_EXIT();                                //�����ٽ�Σ���ϣ�����洮�ڴ�ӡ�⵽�ж�
		
	}
		
}


/*
*********************************************************************************************************
*                                          TMR TASK
*********************************************************************************************************
*/
void TmrCallback (OS_TMR *p_tmr, void *p_arg) //�����ʱ��MyTmr�Ļص�����
{
	CPU_SR_ALLOC();      //ʹ�õ��ٽ�Σ��ڹ�/���ж�ʱ��ʱ����ú꣬�ú������Ͷ���һ���ֲ���
											 //�������ڱ�����ж�ǰ�� CPU ״̬�Ĵ��� SR���ٽ�ι��ж�ֻ�豣��SR��
											 //�����ж�ʱ����ֵ��ԭ��  
  //printf ( "%s", ( char * ) p_arg );

stoptim_cnt++;	

	OS_CRITICAL_ENTER();                 //�����ٽ�Σ���ϣ�����洮�ڴ�ӡ�⵽�ж�
	
printf("stoptime=%d:%d",stoptim_cnt/60,stoptim_cnt%60);
	
	OS_CRITICAL_EXIT();                               

	
}







/*
*********************************************************************************************************
*                                          KEY1 TASK
*********************************************************************************************************
*/
  void  AppTaskKey1 ( void * p_arg )
{
	OS_ERR      err;
	OS_SEM_CTR  ctr;
	CPU_SR_ALLOC();  //ʹ�õ��ٽ�Σ��ڹ�/���ж�ʱ��ʱ����ú꣬�ú������Ͷ���һ���ֲ���
									 //�������ڱ�����ж�ǰ�� CPU ״̬�Ĵ��� SR���ٽ�ι��ж�ֻ�豣��SR��
									//�����ж�ʱ����ֵ��ԭ��	
	uint8_t ucKey1Press = 0;
		
	(void)p_arg;
				 
	while (DEF_TRUE) {                                                         //������
		if( Key_Scan ( macKEY1_GPIO_PORT, macKEY1_GPIO_PIN, 1, & ucKey1Press ) ) //���KEY1������
		{
			ctr = OSSemPend ((OS_SEM   *)&SemOfKey,               //�ȴ����ź��� SemOfKey
								       (OS_TICK   )0,                       //����ѡ�񲻵ȴ����ò���Ч
								       (OS_OPT    )OS_OPT_PEND_BLOCKING,//���û�ź������ò��ȴ�
								       (CPU_TS   *)0,                       //����ȡʱ���
								       (OS_ERR   *)&err);                   //���ش�������
			
			OS_CRITICAL_ENTER();                                  //�����ٽ��
			
			if ( err == OS_ERR_NONE )                      
				printf ( "\r\nKEY1���������ɹ����뵽ͣ��λ��ʣ��%d��ͣ��λ��", ctr );
			else if ( err == OS_ERR_PEND_WOULD_BLOCK )
				printf ( "\r\nkey1��������������˼������ͣ������������ȴ���" );
			
			OS_CRITICAL_EXIT(); 
            OSTaskSuspend ( &AppTaskGUIBaseTCB, & err );
            stoptim_cnt=0;
            OSTmrStart ((OS_TMR   *)&my_tmr, //�����ʱ������
                        (OS_ERR   *)err);    //���ش�������

      


		}
		
		OSTimeDlyHMSM ( 0, 0, 0, 20, OS_OPT_TIME_DLY, & err );  //ÿ20msɨ��һ��
		
	}
	
}


/*
*********************************************************************************************************
*                                          KEY2 TASK
*********************************************************************************************************
*/
  void  AppTaskKey2 ( void * p_arg )
{
	OS_ERR      err;
	OS_SEM_CTR  ctr;
	CPU_SR_ALLOC();  //ʹ�õ��ٽ�Σ��ڹ�/���ж�ʱ��ʱ����ú꣬�ú������Ͷ���һ���ֲ���
									 //�������ڱ�����ж�ǰ�� CPU ״̬�Ĵ��� SR���ٽ�ι��ж�ֻ�豣��SR��
									 //�����ж�ʱ����ֵ��ԭ��
	uint8_t ucKey2Press = 0;
	
	
	(void)p_arg;

					 
	while (DEF_TRUE) {                                                         //������
		if( Key_Scan ( macKEY2_GPIO_PORT, macKEY2_GPIO_PIN, 1, & ucKey2Press ) ) //���KEY2������
		{
		  ctr = OSSemPost((OS_SEM  *)&SemOfKey,                                  //����SemOfKey
							        (OS_OPT   )OS_OPT_POST_ALL,                            //���������еȴ�����
							        (OS_ERR  *)&err);                                      //���ش�������

			OS_CRITICAL_ENTER();                                                   //�����ٽ��
			
			printf ( "\r\nKEY2���������ͷ�1��ͣ��λ��ʣ��%d��ͣ��λ��", ctr );
			
			OS_CRITICAL_EXIT();
			OSTmrStop((OS_TMR   *)&my_tmr,OS_OPT_TMR_NONE,0,(OS_ERR   *)err);
            OSTaskResume ( &AppTaskGUIBaseTCB, & err );

		}
		
		OSTimeDlyHMSM ( 0, 0, 0, 20, OS_OPT_TIME_DLY, & err );                    //ÿ20msɨ��һ��
		
	}
	
}
