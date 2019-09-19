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
//* File : exo3.c
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
//#define ROBOT_A_PRIO        10               // Defining Priority of each task
//#define ROBOT_B_PRIO        9
//#define CONTROLLER_PRIO     7
//#define TRANSPORT_PRIO      8
//#define QTY_MUT_PRIO        6
//#define PEND_MUT_PRIO       5
//
//typedef enum {
//	ROBOT_A_DONE = 1 << 0,
//	ROBOT_B_DONE = 1 << 1,
//	COMMAND_READY = 1 << 2,
//	ROBOT_A_READY = 1 << 3,  //Ready to receive new command
//	ROBOT_B_READY = 1 << 4
//} EV_FLAGS;
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
//
//OS_TCB robotATCB;
//OS_TCB robotBTCB;
//OS_TCB controllerTCB;
//
///*
//*********************************************************************************************************
//*                                           SHARED  VARIABLES
//*********************************************************************************************************
//*/
//
//OS_MUTEX item_count_mutex;
//OS_MUTEX pending_mutex;
//
//OS_FLAG_GRP flags;
//
//volatile int total_item_count = 0;
//volatile int waiting_list = 0;
//
//
///*
//*********************************************************************************************************
//*                                         FUNCTION PROTOTYPES
//*********************************************************************************************************
//*/
//void    prep_robot_A(void* data);
//void    prep_robot_B(void* data);
//void    controller(void* data);
//void    errMsg(OS_ERR err, char* errMSg);
//int     readCurrentTotalItemCount();
//void    writeCurrentTotalItemCount(int qty);
//
///*
//*********************************************************************************************************
//*                                                  MAIN
//*********************************************************************************************************
//*/
//
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
//	/* TODO : Initialiser, creer les taches et demarer */
//	OSTaskCreate(&robotATCB, "tsk1", prep_robot_A, (void*)0, ROBOT_A_PRIO, robotAStk, 10, TASK_STK_SIZE, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
//	OSTaskCreate(&robotBTCB, "tsk2", prep_robot_B, (void*)0, ROBOT_B_PRIO, robotBStk, 10, TASK_STK_SIZE, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
//	OSTaskCreate(&controllerTCB, "tsk3", controller, (void*)0, CONTROLLER_PRIO, controllerStk, 10, TASK_STK_SIZE, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
//	OSFlagCreate(&flags, "flag_group", (OS_FLAGS)0, &err);
//	OSMutexCreate(&item_count_mutex, "items_mutex", &err);
//	OSMutexCreate(&pending_mutex, "pending_mutex", &err);
//	OSStart(&err);
//	while (DEF_ON); // Vous ne devriez pas rentrer dans cette boucle
//
//
//	return;
//}
//
//
//
///*
//*********************************************************************************************************
//*                                            TASK FUNCTIONS
//*********************************************************************************************************
//*/
//
//void prep_robot_A(void* data)
//{
//	OS_ERR err;
//	CPU_TS ts;
//	int startTime = 0;
//	int orderNumber = 1;
//	int itemCount;
//	printf("ROBOT A @ %d : DEBUT.\n", OSTimeGet(&err) - startTime);
//	while (DEF_ON)
//	{
//
//		/* TODO : Remplisser la boucle afin de permettre l'utilisation des flags. N'oubliez pas de proteger vos donnees*/
//		itemCount = (rand() % 7 + 1) * 10;
//		//On met à jour le temps consacré à la préparation
//		OSFlagPend(&flags, COMMAND_READY , (OS_TICK)0, (OS_OPT)OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME, &ts, &err);
//
//		OSMutexPend(&item_count_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//
//		int current = readCurrentTotalItemCount();
//		writeCurrentTotalItemCount(current + itemCount);
//		OSMutexPost(&item_count_mutex, OS_OPT_POST_NONE, &err);
//
//		int counter = 0;
//		while (counter < itemCount * 1000) { counter++; }
//		printf("ROBOT A COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCount, OSTimeGet(&err) - startTime);
//		OSMutexPend(&pending_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//		--waiting_list;
//		OSMutexPost(&pending_mutex, OS_OPT_POST_NONE, &err);
//
//		if (waiting_list == 0) {
//
//		}
//
//		OSFlagPost(&flags, ROBOT_A_DONE, (OS_OPT)OS_OPT_POST_FLAG_SET, &err);
//		orderNumber++;
//	}
//}
//
//void prep_robot_B(void* data)
//{
//	OS_ERR err;
//	CPU_TS ts;
//
//	int startTime = 0;
//	int orderNumber = 1;
//	int itemCount;
//	printf("ROBOT B @ %d : DEBUT. \n", OSTimeGet(&err) - startTime);
//	while (1)
//	{
//
//		itemCount = (rand() % 6 + 2) * 10;
//
//		/* TODO : Remplisser la boucle afin de permettre l'utilisation des flags. N'oubliez pas de proteger vos donnees*/
//		OSFlagPend(&flags, ROBOT_A_DONE,(OS_TICK)0,(OS_OPT)OS_OPT_PEND_FLAG_SET_ANY + OS_OPT_PEND_FLAG_CONSUME,&ts,&err);
//
//		//On met à jour le temps consacré à la préparation
//		OSMutexPend(&item_count_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//		int current = readCurrentTotalItemCount();
//		writeCurrentTotalItemCount(current + itemCount);
//		OSMutexPost(&item_count_mutex, OS_OPT_POST_NONE, &err);
//		int counter = 0;
//		while (counter < itemCount * 1000) { counter++; }
//		printf("ROBOT B COMMANDE #%d avec %d items @ %d.\n", orderNumber, itemCount, OSTimeGet(&err) - startTime);
//		orderNumber++;
//	}
//}
//
//void controller(void* data)
//{
//	OS_ERR err;
//	CPU_TS ts;
//
//	int startTime = 0;
//	int randomTime = 0;
//	printf("CONTROLLER @ %d : DEBUT. \n", OSTimeGet(&err) - startTime);
//	for (int i = 1; i < 11; i++)
//	{
//		randomTime = (rand() % 9 + 5) * 10;
//		/*TODO : Effectuer un delai d'une duree de randomTime*/
//		OSTimeDly(randomTime, OS_OPT_TIME_PERIODIC, &err);
//		/* TODO : Remplisser la boucle afin de permettre l'utilisation des flags. N'oubliez pas de proteger vos donnees*/
//
//		printf("CONTROLLER @ %d : COMMANDE #%d. \n", OSTimeGet(&err) - startTime, i);
//
//		OSMutexPend(&pending_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
//		++waiting_list;
//		OSMutexPost(&pending_mutex, OS_OPT_POST_NONE, &err);
//		OSFlagPost(&flags, COMMAND_READY, (OS_OPT)OS_OPT_POST_FLAG_SET, &err);
//
//	}
//}
//
//int readCurrentTotalItemCount()
//{
//	OS_ERR err;
//
//	/*TODO : Effectuer un delai d'une duree de 2 ticks*/
//	OSTimeDly(2, OS_OPT_TIME_PERIODIC, &err);
//	return total_item_count;
//}
//void writeCurrentTotalItemCount(int newCount)
//{
//	OS_ERR err;
//	/*TODO : Effectuer un delai d'une duree de 2 ticks*/
//	total_item_count = newCount;
//	OSTimeDly(2, OS_OPT_TIME_PERIODIC, &err);
//}
//
//void errMsg(OS_ERR err, char* errMsg)
//{
//	if (err != OS_ERR_NONE)
//	{
//		printf(errMsg);
//		exit(1);
//	}
//}