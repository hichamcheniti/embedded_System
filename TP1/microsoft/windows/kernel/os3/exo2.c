///*
//*********************************************************************************************************
//*                                                 uC/OS-III
//*                                          The Real-Time Kernel
//*                                               PORT Windows
//*
//*
//*												Benjamin Heinen
//*                                  Ecole Polytechnique de Montreal, Qc, CANADA
//*                                                  05/2019
//*
//* File : exo2.c
//*
//*********************************************************************************************************
//*/
//
//// Main include of µC-III
//#include  <cpu.h>
//#include  <lib_mem.h>
//#include  <os.h>
//
//#include  "os_app_hooks.h"
//#include  "app_cfg.h"
///*
//*********************************************************************************************************
//*                                              CONSTANTS
//*********************************************************************************************************
//*/
//
//#define TASK_STK_SIZE       16384            // Size of each task's stacks (# of WORDs)
//
//#define ROBOT_A_PRIO        9                // Defining Priority of each task
//#define ROBOT_B_PRIO        8
//#define CONTROLLER_PRIO     7
//#define MUT_PRIO            6
//
///*
//*********************************************************************************************************
//*                                             VARIABLES
//*********************************************************************************************************
//*/
//
//CPU_STK           robotAStk[TASK_STK_SIZE];  //Stack of each task
//CPU_STK           robotBStk[TASK_STK_SIZE];
//CPU_STK           controllerStk[TASK_STK_SIZE];
//
//OS_TCB robotATCB;
//OS_TCB robotBTCB;
//OS_TCB controllerTCB;
//
//
///*
//*********************************************************************************************************
//*                                           SHARED  VARIABLES
//*********************************************************************************************************
//*/
//OS_SEM sem_controller_to_robot_A;
//OS_SEM sem_robot_A_to_robot_B;
//OS_SEM sem_robot_B_to_robot_A;
//OS_MUTEX total_item_cout_mutex;
//
//volatile int total_item_count = 0;
//
///*
//*********************************************************************************************************
//*                                         FUNCTION PROTOTYPES
//*********************************************************************************************************
//*/
//void    robotA(void* data);
//void    robotB(void* data);
//void    controller(void* data);
//void    errMsg(OS_ERR err, char* errMSg);
//int     readCurrentTotalCount();
//void    writeCurrentTotalCount(int qty);
//
///*
//*********************************************************************************************************
//*                                                  MAIN
//*********************************************************************************************************
//*/
//
//void main(void)
//{
//	OS_ERR err;
//
//	CPU_IntInit();
//
//	Mem_Init();                                                 // Initialize Memory Managment Module                   
//	CPU_IntDis();                                               // Disable all Interrupts                               
//	CPU_Init();                                                 // Initialize the uC/CPU services                       
//
//	OSInit(&err);                                            // Initialize uC/OS-III                                 
//	if (err != OS_ERR_NONE) {
//		while (1);
//	}
//
//	App_OS_SetAllHooks();                                       // Set all applications hooks                      
//
//
//	/* TODO : Creer Semaphore */
//	OSSemCreate(&sem_controller_to_robot_A, "SemControl", 0, &err);
//	OSSemCreate(&sem_robot_A_to_robot_B, "SemRobotA", 0, &err);
//	OSSemCreate(&sem_robot_B_to_robot_A, "SemRobotB", 0, &err);
//
//	/* TODO : Creer Mutex */
//	OSMutexCreate(&total_item_cout_mutex, "Mutex", &err);
//	
//
//	/* TODO : Creer tache */
//	OSTaskCreate(&robotATCB, "tsk1", robotA, (void*)0, ROBOT_A_PRIO, robotAStk, 10, TASK_STK_SIZE, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
//	OSTaskCreate(&robotBTCB, "tsk2", robotB, (void*)0, ROBOT_B_PRIO, robotBStk, 10, TASK_STK_SIZE, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
//	OSTaskCreate(&controllerTCB, "tsk3", controller, (void*)0, CONTROLLER_PRIO, controllerStk, 10, TASK_STK_SIZE, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
//	/* TODO : Start */
//	OSStart(&err);
//	while (DEF_ON); // Vous ne devriez pas rentrer dans cette boucle
//
//	return;
//}
//
///*
//*********************************************************************************************************
//*                                            TASK FUNCTIONS
//*********************************************************************************************************
//*/
//
//
//void robotA(void* data)
//{ 
//	OS_ERR err;
//	CPU_TS ts;
//
//	int startTime = 0;
//	int orderNumber = 1;
//	int itemCount;
//	printf("ROBOT A @ %d : DEBUT.\n", OSTimeGet(&err) - startTime);
//	while (DEF_ON)
//	{
//		itemCount = (rand() % 7 + 1) * 10;
//		/* TODO : Gerer la synchronisation avec les semaphores*/
//	
//		OSSemPend(&sem_controller_to_robot_A, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//		/* TODO : Proteger ce bloc avec un mutex*/
//
//		OSMutexPend(&total_item_cout_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//		int current = readCurrentTotalCount(); 	//On met à jour le temps consacré à la préparation
//		writeCurrentTotalCount(current + itemCount);
//		OSMutexPost(&total_item_cout_mutex, OS_OPT_POST_NONE, &err);
//
//		int counter = 0;
//		while (counter < itemCount * 1000) { counter++; }
//		printf("ROBOT A COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCount, OSTimeGet(&err) - startTime);
//
//
//		/* TODO : Gerer la synchronisation avec les semaphores */
//		OSSemPost(&sem_robot_A_to_robot_B, OS_OPT_POST_NONE, &err);
//		OSSemPend(&sem_robot_B_to_robot_A, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//		
//
//		orderNumber++;
//	}
//
//}
//
//void robotB(void* data)
//{
//	OS_ERR err;	
//	CPU_TS ts;
//
//	int startTime = 0;
//	int orderNumber = 1;
//	int itemCount;
//	printf("ROBOT B @ %d : DEBUT. \n", OSTimeGet(&err) - startTime);
//	while (DEF_ON)
//	{
//		itemCount = (rand() % 6 + 2) * 10;
//		/* TODO : Gerer la synchronisation avec les semaphores */
//		OSSemPend(&sem_robot_A_to_robot_B, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//
//		OSMutexPend(&total_item_cout_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//		/* TODO : Proteger ce bloc avec un mutex*/
//		int current = readCurrentTotalCount();		//On met à jour le temps consacré à la préparation
//		writeCurrentTotalCount(current + itemCount);
//		OSMutexPost(&total_item_cout_mutex, OS_OPT_POST_NONE, &err);
//
//
//		int counter = 0;
//		while (counter < itemCount * 1000) { counter++; }
//		printf("ROBOT B COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCount, OSTimeGet(&err) - startTime);
//
//
//		/* TODO : Gerer la synchronisation avec les semaphores */
//		OSSemPost(&sem_robot_B_to_robot_A, OS_OPT_POST_NONE, &err);
//		orderNumber++;
//	}
//}
//
//void controller(void* data)
//{
//	OS_ERR err;
//	int startTime = 0;
//	int randomTime = 0;
//	printf("CONTROLLER @ %d : DEBUT. \n", OSTimeGet(&err) - startTime);
//	for (int i = 1; i < 11; i++)
//	{
//		randomTime = (rand() % 9 + 5) * 10;
//		/*TODO : Effectuer un delai d'une duree de randomTime*/
//		OSTimeDly(randomTime, OS_OPT_TIME_PERIODIC, &err);
//		printf("CONTROLLER @ %d : COMMANDE #%d. \n", OSTimeGet(&err) - startTime, i);
//
//		/* TODO : Gerer la synchronisation avec les semaphores */
//		OSSemPost(&sem_controller_to_robot_A, OS_OPT_POST_NONE, &err);
//
//	}
//}
//
//int readCurrentTotalCount()
//{
//	OS_ERR err;
//	/*TODO : Effectuer un delai d'une duree de 2 ticks*/
//	OSTimeDly(2, OS_OPT_TIME_PERIODIC, &err);
//	return total_item_count;
//}
//void writeCurrentTotalCount(int newCount)
//{
//	OS_ERR err;
//	/*TODO : Effectuer un delai d'une duree de 2 ticks*/
//	total_item_count = newCount;
//	OSTimeDly(2, OS_OPT_TIME_PERIODIC,&err);
//}
//void errMsg(OS_ERR err, char* errMsg)
//{
//	if (err != OS_ERR_NONE)
//	{
//		printf(errMsg);
//		printf("%d", err);
//		while (DEF_ON);
//	}
//}