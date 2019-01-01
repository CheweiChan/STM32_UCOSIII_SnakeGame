/**
  ******************************************************************************
  * @file    main.c
  * @author  chewei
  * @version V1.0
  * @date    2018-12-10
  * @brief   emwin + ucosIII 
  ******************************************************************************
  * ʵ��ƽ̨:����  STM32 F103 ������ 
  */


/*
*********************************************************************************************************
*                                             �����ļ�
*********************************************************************************************************
*/

#include <includes.h>
#include <bsp_key.h> 
#include "stdio.h"
/*
*********************************************************************************************************
*                                               �궨��
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             ������ƿ�TCB
*********************************************************************************************************
*/
OS_TCB   AppTaskStartTCB;
OS_TCB   AppTaskKeyScanTCB;
OS_TCB   AppTaskGUIBaseTCB;
OS_TCB   AppTaskTimerBaseTCB;
OS_TCB   AppTaskKey1TCB;
OS_TCB   AppTaskKey2TCB;
OS_TMR   my_tmr;   //���������ʱ������

OS_SEM SemOfKey;          //��־KEY1�Ƿ񱻵����Ķ�ֵ�ź���
CPU_INT32U tim_cnt=0;
CPU_INT32U stoptim_cnt=0;


/*
*********************************************************************************************************
*                                              ջ�ռ�STACKS
*********************************************************************************************************
*/
__align(8) static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];
__align(8) static  CPU_STK  AppTaskKeyScanStk[APP_TASK_KEY_SCAN_STK_SIZE];
__align(8) static  CPU_STK  AppTaskGUIBaseStk[APP_TASK_GUI_BASE_STK_SIZE];
__align(8) static  CPU_STK  AppTaskTimerBaseStk[APP_TASK_GUI_BASE_STK_SIZE];
__align(8) static  CPU_STK  AppTaskKey1Stk [ 128 ];
__align(8) static  CPU_STK  AppTaskKey2Stk [ 128 ];


/*
*********************************************************************************************************
*                                             ��������
*********************************************************************************************************
*/

static  void  AppTaskStart  (void *p_arg);
static  void  BSPTaskCreate (void);
static  void  AppTaskCreate(void);
void AppTaskTouchScan(void );
void AppTaskTmr_ ( void * p_arg );
void TmrCallback (OS_TMR *p_tmr, void *p_arg);
void AppTaskKey1 ( void * p_arg );
void AppTaskKey2 ( void * p_arg );
void Test_flash(void);
extern  void Snack_Task(void);
/*
*********************************************************************************************************
*                                            
*********************************************************************************************************
*/


/**
  * @brief  ������
  * @param  ��  
  * @retval ��
  */
int  main (void)
{
    OS_ERR  os_err;

    OSInit(&os_err);                                               		/* ��ʼ�� uC/OS-III.                                      */
		//������������
    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                // ����TCB                               
                 (CPU_CHAR   *)"App Task Start", 								// ��������                             
                 (OS_TASK_PTR ) AppTaskStart,									  // ������ָ��                                
                 (void       *) 0,																	// ��ѡ��������
                 (OS_PRIO     ) APP_TASK_START_PRIO,							// ���ȼ�
                 (CPU_STK    *)&AppTaskStartStk[0],							// ����ջ����ַ
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,		// ջ��ˮӡ������
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,        //ջ��С
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),//��ѡ����
                 (OS_ERR     *)&os_err);															//�������

		//��ʼִ�����񣬴Ӵ˴���ʼ��ucosϵͳ����
    OSStart(&os_err);                                              
		
		
}


/**
  * @brief  ��������������Ҫ���ucos��BSP��GUI�Լ���������ĳ�ʼ��
  * @param  p_arg: OSTaskCreate����ʱ���������ָ��
  * @retval ��
  */
static  void  AppTaskStart (void *p_arg)
{
    OS_ERR      os_err;
	  CPU_INT32U     cpu_clk_freq;
   (void)p_arg;

	BSP_Init();  //��ʼ��BSP  		
	CPU_Init();	//��ʼ��CPU
	BSP_Tick_Init();//��ʼ��systick
  Mem_Init(); 	//��ʼ���洢������                                     
	Key_Initial();
  Test_flash();

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&os_err); //����������ʱCPUʹ����                            
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

	APP_TRACE_DBG(("���ڴ���Ӧ������...\n\r"));

	//������������
	BSPTaskCreate();
	//����Ӧ������
	AppTaskCreate();  
  cpu_clk_freq = BSP_CPU_ClkFreq();     

 //������ѭ��
	while (DEF_TRUE) 
	{                                        
	    //LED2_TOGGLE ;
	    printf("cpuʹ����:%.2f%%,%d\n,",((float)OSStatTaskCPUUsage/100),cpu_clk_freq);
	
  		/*
                  ��ʱ����������������ѭ���ڶ�Ӧ������1ms��ʱ
		  �ر��Ǹ����ȼ�������������ʱ�����������ȼ�������ܻ��޻���ִ��
                */
	    OSTimeDly(1000u,OS_OPT_TIME_DLY,&os_err);
	}	
		
}

/**
  * @brief  �������������簴������ʱɨ���
  * @param  ��
  * @retval ��
  */
static  void  BSPTaskCreate (void)
{
	OS_ERR  os_err;

		//����ɨ������
    OSTaskCreate((OS_TCB     *)&AppTaskKeyScanTCB,             // ����TCB                               
		 (CPU_CHAR   *)"Key Scan", 										// ��������                             
	         (OS_TASK_PTR ) AppTaskTouchScan,									// ������ָ��                                
		 (void       *) 0,						// ��ѡ��������
		 (OS_PRIO     ) APP_TASK_KEY_SCAN_PRIO,				// ���ȼ�
		 (CPU_STK    *)&AppTaskKeyScanStk[0],				// ����ջ����ַ
		 (CPU_STK_SIZE) APP_TASK_KEY_SCAN_STK_SIZE / 10,	 	// ջ��ˮӡ������
		 (CPU_STK_SIZE) APP_TASK_KEY_SCAN_STK_SIZE,        		//ջ��С
		 (OS_MSG_QTY  ) 0u,
		 (OS_TICK     ) 0u,
		 (void       *) 0,
		 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	//��ѡ����
		 (OS_ERR     *)&os_err);					//�������


    OSTaskCreate((OS_TCB     *)&AppTaskTimerBaseTCB,             // ����TCB                               
                 (CPU_CHAR   *)"GUI Base Test",                                // ��������                             
                 (OS_TASK_PTR ) AppTaskTmr_,                                   // ������ָ��                                
                 (void       *) 0,                                             // ��ѡ��������
                 (OS_PRIO     ) 2,                  // ���ȼ�
                 (CPU_STK    *)&AppTaskTimerBaseStk[0],                        // ����ջ����ַ
                 (CPU_STK_SIZE) 128 / 10,               // ջ��ˮӡ������
                 (CPU_STK_SIZE) 128,                //ջ��С
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),//��ѡ����
                 (OS_ERR     *)&os_err);    


 /* ������ֵ�ź��� SemOfKey */
    OSSemCreate((OS_SEM      *)&SemOfKey,    //ָ���ź���������ָ��
                (CPU_CHAR    *)"SemOfKey",    //�ź���������
                (OS_SEM_CTR   )5,             //��ʾ������Դ��Ŀ
                (OS_ERR      *)&os_err);         //��������
                          
          

/* ���� AppTaskKey1 ���� */
    OSTaskCreate((OS_TCB     *)&AppTaskKey1TCB,                             //������ƿ��ַ
                 (CPU_CHAR   *)"App Task Key1",                             //��������
                 (OS_TASK_PTR ) AppTaskKey1,                                //������
                 (void       *) 0,                                          //���ݸ����������β�p_arg����ʵ��
                 (OS_PRIO     ) 2,                      //��������ȼ�
                 (CPU_STK    *)&AppTaskKey1Stk[0],                          //�����ջ�Ļ���ַ
                 (CPU_STK_SIZE) 128 / 10,             //�����ջ�ռ�ʣ��1/10ʱ����������
                 (CPU_STK_SIZE) 128,                  //�����ջ�ռ䣨��λ��sizeof(CPU_STK)��
                 (OS_MSG_QTY  ) 5u,                                         //����ɽ��յ������Ϣ��
                 (OS_TICK     ) 0u,                                         //�����ʱ��Ƭ��������0��Ĭ��ֵOSCfg_TickRate_Hz/10��
                 (void       *) 0,                                          //������չ��0����չ��
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //����ѡ��
                 (OS_ERR     *)&os_err);                                       //���ش�������

        /* ���� AppTaskKey2 ���� */
    OSTaskCreate((OS_TCB     *)&AppTaskKey2TCB,                             //������ƿ��ַ
                 (CPU_CHAR   *)"App Task Key2",                             //��������
                 (OS_TASK_PTR ) AppTaskKey2,                                //������
                 (void       *) 0,                                          //���ݸ����������β�p_arg����ʵ��
                 (OS_PRIO     ) 3,                      //��������ȼ�
                 (CPU_STK    *)&AppTaskKey2Stk[0],                          //�����ջ�Ļ���ַ
                 (CPU_STK_SIZE) 128 / 10,             //�����ջ�ռ�ʣ��1/10ʱ����������
                 (CPU_STK_SIZE) 128,                  //�����ջ�ռ䣨��λ��sizeof(CPU_STK)��
                 (OS_MSG_QTY  ) 5u,                                         //����ɽ��յ������Ϣ��
                 (OS_TICK     ) 0u,                                         //�����ʱ��Ƭ��������0��Ĭ��ֵOSCfg_TickRate_Hz/10��
                 (void       *) 0,                                          //������չ��0����չ��
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //����ѡ��
                 (OS_ERR     *)&os_err);                                       //���ش�������

   /* ���������ʱ�� */
   OSTmrCreate ((OS_TMR              *)&my_tmr,             //�����ʱ������
                (CPU_CHAR            *)"MySoftTimer",       //���������ʱ��
                (OS_TICK              )10,                  //��ʱ����ʼֵ����10Hzʱ�����㣬��Ϊ1s
                (OS_TICK              )10,                  //��ʱ����������ֵ����10Hzʱ�����㣬��Ϊ1s
                (OS_OPT               )OS_OPT_TMR_PERIODIC, //�����Զ�ʱ
                (OS_TMR_CALLBACK_PTR  )TmrCallback,         //�ص�����
                (void                *)"Timer Over!",       //����ʵ�θ��ص�����
                (OS_ERR              *)os_err);                //���ش�������
                              
     /* ���������ʱ�� */                      
      
                    



}

/**
  * @brief  ����Ӧ������ĺ��������ᱻ������������Դ���Ӧ��
  * @param  ��
  * @retval ��
  */
static  void  AppTaskCreate (void)
{
	OS_ERR  os_err;

		//����Ӧ������
	OSTaskCreate((OS_TCB     *)&AppTaskGUIBaseTCB,             // ����TCB                               
							 (CPU_CHAR   *)"GUI Base Test", 									// ��������                             
							 (OS_TASK_PTR ) Snack_Task,									  // ������ָ��                                
							 (void       *) 0,																	// ��ѡ��������
							 (OS_PRIO     ) APP_TASK_GUI_BASE_PRIO,					// ���ȼ�
							 (CPU_STK    *)&AppTaskGUIBaseStk[0],							// ����ջ����ַ
							 (CPU_STK_SIZE) APP_TASK_GUI_BASE_STK_SIZE / 10,				// ջ��ˮӡ������
							 (CPU_STK_SIZE) APP_TASK_GUI_BASE_STK_SIZE,        		//ջ��С
							 (OS_MSG_QTY  ) 0u,
							 (OS_TICK     ) 0u,
							 (void       *) 0,
							 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),//��ѡ����
							 (OS_ERR     *)&os_err);															//�������


}




/*********************************************END OF FILE**********************/

