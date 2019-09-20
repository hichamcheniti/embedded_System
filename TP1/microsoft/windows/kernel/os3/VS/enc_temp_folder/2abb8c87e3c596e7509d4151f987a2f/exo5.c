/*
*********************************************************************************************************
*                                                 uC/OS-III
*                                          The Real-Time Kernel
*                                               PORT Windows
*
*
*												Benjamin Heinen
*                                  Ecole Polytechnique de Montreal, Qc, CANADA
*                                                  05/2019
*
* File : exo4.c
*
*********************************************************************************************************
*/

// Main include of µC-III
#include  <cpu.h>
#include  <lib_mem.h>
#include  <os.h>

#include  "os_app_hooks.h"
#include  "app_cfg.h"
/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define TASK_STK_SIZE       16384            // Size of each task's stacks (# of WORDs)

#define ROBOT_AT1_PRIO        8                // Defining Priority of each task
#define ROBOT_AT2_PRIO        9
#define ROBOT_BT1_PRIO        10                // Defining Priority of each task
#define ROBOT_BT2_PRIO        11

#define CONTROLLER_PRIO     22
#define MUT_PREP_TIME_PRIO  6
#define MUT_CONTROLLER_PRIO 5

/*
*********************************************************************************************************
*                                             VARIABLES
*********************************************************************************************************
*/

CPU_STK           prepRobotAT1Stk[TASK_STK_SIZE];  //Stack of each task
CPU_STK           prepRobotBT1Stk[TASK_STK_SIZE];
CPU_STK           prepRobotAT2Stk[TASK_STK_SIZE];  //Stack of each task
CPU_STK           prepRobotBT2Stk[TASK_STK_SIZE];
CPU_STK           transportStk[TASK_STK_SIZE];
CPU_STK           controllerStk[TASK_STK_SIZE];


OS_TCB robotAT1TCB;
OS_TCB robotBT1TCB;
OS_TCB robotAT2TCB;
OS_TCB robotBT2TCB;
OS_TCB controllerTCB;


/*
*********************************************************************************************************
*                                           SHARED  VARIABLES
*********************************************************************************************************
*/
OS_SEM sem_robot_B_to_robot_A;

OS_MUTEX total_prep_time_mutex;;
OS_MUTEX controller_done_mutex;

OS_Q Q_controller_to_robot_A;
void* A_prep_time_msg[10];

OS_Q Q_robot_A_to_robot_B_T1;
void* B_T1_prep_time_msg[10];

OS_Q Q_robot_A_to_robot_B_T2;
void* B_T2_prep_time_msg[10];

volatile int total_prep_time = 0;
int controller_done = 0;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void    robotA(void* data);
void    robotB(void* data);
void    controller(void* data);
void    errMsg(OS_ERR err, char* errMSg);
int     readCurrentTotalPrepTime();
void    writeCurrentTotalPrepTime(int qty);

/*
*********************************************************************************************************
*                                             STRUCTURES
*********************************************************************************************************
*/

typedef struct work_data {
	int work_data_a;
	int work_data_b;
} work_data;

/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main(void)
{
	OS_ERR err;


	/* TODO : Initialiser, creer les taches et demarer */

	CPU_IntInit();

	Mem_Init();                                                 // Initialize Memory Managment Module                   
	CPU_IntDis();                                               // Disable all Interrupts                               
	CPU_Init();                                                 // Initialize the uC/CPU services                       

	OSInit(&err);                                            // Initialize uC/OS-III                                 
	if (err != OS_ERR_NONE) {
		while (1);
	}

	App_OS_SetAllHooks();                                       // Set all applications hooks                      

	OSTaskCreate(&robotAT1TCB, "tsk1a", robotA, (void*)1, ROBOT_AT1_PRIO, prepRobotAT1Stk, 10, TASK_STK_SIZE / 2, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
	OSTaskCreate(&robotAT2TCB, "tsk2a", robotA, (void*)2, ROBOT_AT2_PRIO, prepRobotAT2Stk, 10, TASK_STK_SIZE / 2, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
	
	OSTaskCreate(&robotBT1TCB, "tsk1b", robotB, (void*)1, ROBOT_BT1_PRIO, prepRobotBT1Stk, 10, TASK_STK_SIZE / 2, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
	OSTaskCreate(&robotBT2TCB, "tsk2b", robotB, (void*)2, ROBOT_BT2_PRIO, prepRobotBT2Stk, 10, TASK_STK_SIZE / 2, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);
	
	OSTaskCreate(&controllerTCB, "tsk0", controller, (void*)0, CONTROLLER_PRIO, controllerStk, 10, TASK_STK_SIZE / 2, 5, 100000, (void*)0, OS_OPT_TASK_STK_CHK + OS_OPT_TASK_STK_CLR, &err);

	OSSemCreate(&sem_robot_B_to_robot_A, "SemControl", 0, &err);

	OSMutexCreate(&total_prep_time_mutex, "MutexPrepTime", &err);
	OSMutexCreate(&controller_done_mutex, "MutexController", &err);


	OSQCreate(&Q_controller_to_robot_A, "controllerA_Queue", 10, &err);
	OSQCreate(&Q_robot_A_to_robot_B_T1, "ABT1_Queue", 10, &err);
	OSQCreate(&Q_robot_A_to_robot_B_T2, "ABT2_Queue", 10, &err);

	OSStart(&err);

	while (DEF_ON); // Vous ne devriez pas rentrer dans cette boucle
	return;
}

/*
*********************************************************************************************************
*                                            TASK FUNCTIONS
*********************************************************************************************************
*/

void doWorkRobotA(work_data workData, int orderNumber, int startTime, void* data)
{
	OS_ERR err;
	CPU_TS ts;
	//Avertir Robot B
	/*TODO : Envoi de message*/

	/* TODO : Proteger ce bloc avec un mutex*/
	//On met à jour le temps consacré à la préparation

	OSMutexPend(&total_prep_time_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);

	int currentTotalPrepTime = readCurrentTotalPrepTime();
	writeCurrentTotalPrepTime(currentTotalPrepTime + workData.work_data_a);

	OSMutexPost(&total_prep_time_mutex, OS_OPT_POST_NONE, &err);
	printf("%d: TACHE PREP_ROBOT_A COMMANDE #%d @ %d : Début préparation robot A.\n", OSPrioCur, orderNumber, OSTimeGet(&err) - startTime);
	if ((int)data == 1) {
		OSQPost((OS_Q*)& Q_robot_A_to_robot_B_T1, (void*) & (workData.work_data_b), (OS_MSG_SIZE)sizeof(void*), (OS_OPT)OS_OPT_POST_FIFO, (OS_ERR*)& err);
	}
	else if ((int)data == 2) {
		OSQPost((OS_Q*)& Q_robot_A_to_robot_B_T2, (void*) & (workData.work_data_b), (OS_MSG_SIZE)sizeof(void*), (OS_OPT)OS_OPT_POST_FIFO, (OS_ERR*)& err);
	}
	else {
		errMsg(err, "Erreur de transmission du message du robot A vers le robot B. \n");
	}
	/* TODO : Faites un delai d'une duree de work_data_a	*/
	OSTimeDly(workData.work_data_a, OS_OPT_TIME_PERIODIC, &err);
	errMsg(err, "Error while trying to access sem_robot_B_to_robot_A");


	orderNumber++;
}
void robotA(void* data)
{
	OS_ERR err;
	CPU_TS ts;
	int startTime = 0;
	int orderNumber = 1;
	work_data* workData;
	OS_MSG_SIZE msg_size;
	printf("%d: TACHE PREP_ROBOT_A @ %d : DEBUT.\n", OSPrioCur, OSTimeGet(&err) - startTime);


	while (1)
	{
		//Acquiert mutex protection variable controller_done

		/* TODO : Recuperer la quantite*/
		workData = OSQPend(&Q_controller_to_robot_A, 0, OS_OPT_PEND_BLOCKING, (OS_MSG_SIZE*)& msg_size, &ts, &err);


		doWorkRobotA(*workData, orderNumber, startTime,(int)data);


		/* TODO : Liberer la memoire*/

		orderNumber++;
	}
}

void robotB(void* data)
{
	OS_ERR err;
	CPU_TS ts;
	OS_MSG_SIZE msg_size;

	int startTime = 0;
	int orderNumber = 1;
	int* robotBPrepTime;
	printf("%d : TACHE PREP_ROBOT_B @ %d : DEBUT. \n", OSPrioCur, OSTimeGet(&err) - startTime);

	while (1)
	{
		/* TODO : Recuperer la quantite */

		if ((int)data == 1) {
			robotBPrepTime = OSQPend(&Q_robot_A_to_robot_B_T1, 0, OS_OPT_PEND_BLOCKING, (OS_MSG_SIZE*)& msg_size, &ts, &err);
		}
		else if ((int)data == 2) {
			robotBPrepTime = OSQPend(&Q_robot_A_to_robot_B_T2, 0, OS_OPT_PEND_BLOCKING, (OS_MSG_SIZE*)& msg_size, &ts, &err);
		}
		else {
			robotBPrepTime = NULL;
			errMsg(err, "Erreur de reception du message du robot A vers le robot B. \n");
			exit(1);
		}
		/* TODO : On met à jour le temps consacré à la préparation */
	

		printf("%d: TACHE PREP_ROBOT_B COMMANDE #%d @ %d : Début préparation robot B.\n", OSPrioCur, orderNumber, OSTimeGet(&err) - startTime);

		/* TODO : Faite un delqi d'une duree de robotBPrepTime*/
		OSTimeDly(*robotBPrepTime, OS_OPT_TIME_PERIODIC, &err);

		/* TODO : Gerer la synchronisation*/
		OSSemPost(&sem_robot_B_to_robot_A, OS_OPT_POST_NONE, &err);
	



		orderNumber++;
	}
}

void controller(void* data)
{
	OS_ERR err;
	CPU_TS ts;

	int startTime = 0;
	int randomTime = 0;
	work_data* workData;
	printf("TACHE CONTROLLER @ %d : DEBUT. \n", OSTimeGet(&err) - startTime);
	while (1)
	{
		for (int i = 1; i < 12; i++)
		{
			/* TODO : Création d'une commande */
			workData = malloc(sizeof(work_data));

			workData->work_data_a = (rand() % 8 + 3) * 10;
			workData->work_data_b = (rand() % 8 + 3) * 10;

			

			printf("TACHE CONTROLLER @ %d : COMMANDE #%d. \n prep time A = %d, prep time B = %d\n", OSTimeGet(&err) - startTime, i, workData->work_data_a, workData->work_data_b);

			/* TODO :  Envoi de la commande aux différentes tâches */
			OSQPost((OS_Q*)& Q_controller_to_robot_A, (void*)workData, sizeof(workData), (OS_OPT)OS_OPT_POST_FIFO, (OS_ERR*)& err);

			/* TODO :  Délai aléatoire avant nouvelle commande */
			randomTime = (rand() % 9 + 5) * 10;
			OSTimeDly(randomTime, OS_OPT_TIME_PERIODIC, &err);

		}
		free(workData);


		/* TODO : Protection de ce bloc avec un mutex.*/
		OSMutexPend(&controller_done_mutex, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
		controller_done = 1;
		OSMutexPost(&controller_done_mutex, OS_OPT_POST_NONE, &err);

		printf("FIN DE L'ENVOI DE COMMANDE @ %d\n", OSTimeGet(&err) - startTime);

		OSTaskSuspend(&robotAT1TCB, &err);
		OSTaskSuspend(&robotBT1TCB, &err);
		OSTaskSuspend(&robotAT2TCB, &err);
		OSTaskSuspend(&robotBT2TCB, &err);
		OSTaskSuspend(&controllerTCB, &err);
	}
}

int readCurrentTotalPrepTime()
{
	OS_ERR err;

	/*TODO : Effectuer un delai d'une duree de 2 ticks*/
	OSTimeDly(2, OS_OPT_TIME_PERIODIC, &err);

	return total_prep_time;
}
void writeCurrentTotalPrepTime(int newCount)
{
	OS_ERR err;

	/*TODO : Effectuer un delai d'une duree de 2 ticks*/
	total_prep_time = newCount;
	OSTimeDly(2, OS_OPT_TIME_PERIODIC, &err);

}

void errMsg(OS_ERR err, char* errMsg)
{
	if (err != OS_ERR_NONE)
	{
		printf(errMsg);
		exit(1);
	}
}