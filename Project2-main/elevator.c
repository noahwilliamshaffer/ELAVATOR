#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/list.h>
MODULE_LICENSE("GPL");

//Animal weights
#define CAT_WEIGHT 15 
#define DOG_WEIGHT 45 
#define LIZ_WEIGHT 5

//Elevator statuses
#define OFFLINE 0
#define IDLE 1
#define LOADING 2
#define UP 3
#define DOWN 4

//Floors
#define NUM_FLOORS 10

#define NUM_ANIMAL_TYPES 3
#define MAX_ANIMALS 10

#define ANIMAL_CAT 0
#define ANIMAL_DOG 1
#define ANIMAL_LIZ 2

#define ENTRY_NAME "elevator"
#define PERMS 0644
#define ENTRY_SIZE 10000
#define PARENT NULL

static char *message;
static int read_p;
static struct file_operations fops;

//Global variables
//extern int passengers;
int numPassengers;		// Number of animals on elevator
int currWeight;		// Current weight on elevator
int status;		// Elevator status
int currFloor;		// Current Floor of elevator
int numLizards;		// Count for each type of animal on elevator
int numCats;	
int numDogs;
int numWaiting;		// Total number of animals waiting on every floor
int numServiced;	// Total throughput of elevator
int deactivating_elevator = 0;	// Bool for if stop_elevator is called
int numOnFloor[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int direction = UP;
int load = 0, unload = 0;


/*
typedef struct{

    struct list_head list;
}floors;*/


/*
typedef struct{

    struct list_head list;
}floors;*/


struct list_head floors[NUM_FLOORS]; 
struct list_head elevator;

// Struct for each individual animal, keeps track of various stats
typedef struct {
	int id;
	int weight;
	int startFloor;
	int destFloor;
	struct list_head list;
} animal;

struct thread_parameter {
	int id;
	//int cnt[CNT_SIZE];
	struct task_struct *kthread;
	struct mutex mutex;
};

struct thread_parameter thread1;

/*************************************************************************************************/
int print_animals(void){
//line may need to be changed 139
 
    int i;
	animal *a;
	struct list_head *temp;
	


	char *buf = kmalloc(sizeof(char) * 100, __GFP_RECLAIM);

	/*if (buf == NULL) {
		printk(KERN_WARNING "print_animals");
		return -ENOMEM;
	}*/
    
    printk(KERN_WARNING " 1ST IN PRINT_ANIMAL \n");

    strcpy(message, " ");

    printk(KERN_WARNING "2ND IN PRINT_ANIMAL\n");

    switch (status) {
		case 0:
			sprintf(buf, "Elevator state: OFFLINE\n");
			break;
		case 1:
            sprintf(buf, "Elevator state: IDLE\n");
			break;
		case 2:
            sprintf(buf, "Elevator state: LOADING\n");
			break;
		case 3:
            sprintf(buf, "Elevator state: UP\n");
			break;
		case 4:
            sprintf(buf, "Elevator state: DOWN\n");
			break;
		default:
			return -1;
	}

    strcat(message, buf);

    sprintf(buf, "Current floor: %d\n", currFloor);
    strcat(message, buf);
    
    sprintf(buf, "Current weight: %d\n", currWeight);
    strcat(message, buf);

    sprintf(buf, "Elevator status: %d C, %d D, %d L\n", numCats, numDogs, numLizards); 
    strcat(message, buf);

    sprintf(buf, "Number of passengers: %d\n", numPassengers);
    strcat(message, buf);

    sprintf(buf, "Number of passengers waiting: %d\n", numWaiting);
    strcat(message, buf);

    sprintf(buf, "Number passengers serviced: %d\n\n\n", numServiced);
    strcat(message, buf);

    printk(KERN_WARNING "3rd\n");
    

    for(i = 9; i >= 0; --i){

        if(currFloor == i + 1){
            sprintf(buf, "[*] Floor %d: %d ", i + 1, numOnFloor[i]);
        }
        else{
            sprintf(buf, "[ ] Floor %d: %d ", i + 1, numOnFloor[i]);
        }

        strcat(message, buf);

        if(numOnFloor[i] != 0){
            list_for_each(temp, &floors[i]) { /* forwards*/

                a = list_entry(temp, animal, list);

                switch (a->id) { // changed from a to animal, not sure if correct
                    case ANIMAL_CAT:
                        sprintf(buf, " C");
                        break;
                    case ANIMAL_DOG:
                        sprintf(buf, " D");
                        break;
                    case ANIMAL_LIZ:
                        sprintf(buf, " L");
                        break;
                    default:
                        return -1;
                }

                strcat(message, buf);
            }
        
        }

        strcat(message, "\n");

    printk(KERN_WARNING "4th");
    }

    strcat(message, "\n");

    printk(KERN_WARNING "5th\n");

    kfree(buf);


	return 0;


}
// You will also need to print the following for each floor of the building: 
// •An indicator for whether or not the elevator is on the floor
// •The count of the waiting passengers 
// •For each waiting passenger, a character indicating the passenger type 

/*************************************************************************************************/
int newWaitingAnimal(int start_floor, int destination_floor, int type){

    int id;
	int weight;
	animal *newAnimal;

	
	switch (type) {
		case ANIMAL_CAT:
			id = ANIMAL_CAT;
			weight = CAT_WEIGHT;
			break;
		case ANIMAL_DOG:
			id = ANIMAL_DOG;
			weight = DOG_WEIGHT;
			break;
		case ANIMAL_LIZ:
			id = ANIMAL_LIZ;
			weight = LIZ_WEIGHT;
			break;
		default:
			return -1;
	}

    newAnimal = kmalloc(sizeof(animal) * 1, __GFP_RECLAIM);
    if (newAnimal == NULL)
		return -ENOMEM;

    newAnimal->id = id;
    newAnimal->weight = weight;
    newAnimal->startFloor = start_floor;
    newAnimal->destFloor = destination_floor;

    list_add_tail(&newAnimal->list, &floors[start_floor - 1]); /* insert at back of list */

    ++numWaiting;
    ++numOnFloor[start_floor - 1];

    return 0;
}
/*************************************************************************************************/
int addToElevator (void){

    struct list_head move_list;
	struct list_head *temp1, *temp2;
	struct list_head *dummy1, *dummy2;
    int willSleep = 0;
    animal *a , *b;



    if(numPassengers == 10){
        return -1;//this is an attrocity
    }
    
    list_for_each_safe(temp1, dummy1, &floors[currFloor - 1]) { /* forwards */
        printk("inside add to elevator after list_for\n");
        a = list_entry(temp1, animal, list);

        if(currWeight + a->weight < 100){
            printk("inside WILLSLEEP IN ADD TO ANIMAL\n");
            willSleep = 1;
        }

    }

    if(willSleep == 1){

        status = LOADING;

        mutex_unlock(&thread1.mutex); 
        ssleep(1);
        mutex_lock_interruptible(&thread1.mutex);

        INIT_LIST_HEAD(&move_list);

        printk("inside add to elevator before list_for\n");


        list_for_each_safe(temp2, dummy2, &floors[currFloor - 1]) { /* forwards */
            printk("inside add to elevator after list_for\n");
            b = list_entry(temp2, animal, list);

            

            if(currWeight + b->weight > 100){
                printk("i messed up 1\n");
                return -1;
            }
            if(numPassengers == 10){
                printk("i messed up 2\n");
                return -1;
            }
            else{
                
                list_move_tail(&b->list, &elevator);
                //list_add_tail(&a->list, &elevator); /* insert at back of list */
                /* removes entry from list */
                --numWaiting;
                --numOnFloor[currFloor - 1];
                ++numPassengers;
                currWeight += b->weight;

                switch (b->id) {
                    case ANIMAL_CAT:
                        ++numCats;
                        break;
                    case ANIMAL_DOG:
                        ++numDogs;
                        break;
                    case ANIMAL_LIZ:
                        ++numLizards;
                        break;
                    default:
                        return -1;
                }

            }
        }
    }

    else{
        return -1;
    }

    return 0;
    
}
/*************************************************************************************************/
void offloadElevator(void){
    struct list_head move_list;
	struct list_head *temp1, *temp2;
	struct list_head *dummy1, *dummy2;
    int willSleep = 0;
    animal *a, *b;


    list_for_each_safe(temp1, dummy1, &elevator) { /* forwards */

    b = list_entry(temp1, animal, list);

        if(b->destFloor == currFloor){
            printk("offset trigered true\n");
            willSleep = 1;
        }
    }

    if(willSleep == 1){

        status = LOADING;
        mutex_unlock(&thread1.mutex); 
        ssleep(1);
        mutex_lock_interruptible(&thread1.mutex);

        INIT_LIST_HEAD(&move_list);

        if(numPassengers > 0){

            list_for_each_safe(temp2, dummy2, &elevator) { /* forwards */
        
                a = list_entry(temp2, animal, list);

                if(a->destFloor == currFloor){

                    numServiced++;

                    currWeight -= a->weight;
                    --numPassengers;

                    switch (a->id) {
                        case ANIMAL_CAT:
                            --numCats;
                            break;
                        case ANIMAL_DOG:
                            --numDogs;
                            break;
                        case ANIMAL_LIZ:
                            --numLizards;
                            break;
                        default:
                            ;
                    }

                    list_del(temp2);	/* removes entry from list */
                    kfree(a);
                }
            
            }
        }      
    }
}
/*************************************************************************************************/
//By the time we call move elevator there will only be elevators 
//outside of the function we have to handle the scenario where we set the elevator direction 
//where there are no animals on the elevator
//
int moveElevator(void){
    if(numWaiting == 0 && numPassengers == 0){
        status = IDLE;
        return 1;
    }

    if(status == IDLE){
        status = UP;
    }
    if(status == UP){
        if(currFloor == 10){
            direction = DOWN;
            status = DOWN;
            --currFloor;
        }
        else{
            ++currFloor;
        }
    }
    else if(status == DOWN){
        if(currFloor == 1){
            direction = UP;
            status = UP;
             ++currFloor;
        }
        else{
            --currFloor;
        }
    }

    return 0;

}
/*************************************************************************************************/
int elevator_run(void *data){

    struct thread_parameter *param = data;

    while (!kthread_should_stop()) {

        ssleep(1);

        if (mutex_lock_interruptible(&param->mutex) == 0) {

            if(numPassengers != 0 && status != OFFLINE)
            {
                //if(offloadElevator() == 0);  
                offloadElevator();  

            }

            if(deactivating_elevator != 1 && status != OFFLINE){
                addToElevator();
            }
            if(status != OFFLINE){

                if(numWaiting != 0 || numPassengers != 0){

                    printk("inside of moveEle\n");
                    status = direction;
                    mutex_unlock(&param->mutex);
                    ssleep(2);
                    mutex_lock_interruptible(&param->mutex);
                    moveElevator();

                }
                else{
                    status = IDLE;
                }
                
            }
            if(deactivating_elevator == 1 && numPassengers == 0)
            {
                status = OFFLINE;
            }
            //printk("hey im here\n");

            mutex_unlock(&param->mutex);
        }
    }
    return 0;
}
/*************************************************************************************************/
void elevator_init_parameter(struct thread_parameter *parm) {
	static int id = 1;

	parm->id = id++;
	mutex_init(&parm->mutex);
	parm->kthread = kthread_run(elevator_run, parm, "thread example %d", parm->id);
}

    
/*************************************************************************************************/
extern long (*STUB_start_elevator)(void);
extern long (*STUB_stop_elevator)(void);
extern long (*STUB_issue_request)(int, int , int);
long start_elevator(void) {
    if(status != OFFLINE){
        return 1;
    }
    else if(status == OFFLINE){
        status = IDLE;
        deactivating_elevator = 0;
        return 0;
    }
    else{
        return -1;  //needs to be change to correct ERRORNUM
    }
    
}
long stop_elevator(void) {
    //copy main loop that executes elevator without picking up
    //return true when elevator is empty and stop officially
    if (deactivating_elevator == 1){
        return 1;
    }

    deactivating_elevator = 1;
    return 0;
}
/*************************************************************************************************/
long issue_request(int start_floor, int destination_floor, int type) {
    
    if(start_floor < 1 || start_floor > 10){
        printk("start_floor out of range\n");
        return 1;
    }
    if(destination_floor < 1 || deactivating_elevator > 10){
        printk("destination_floor out of range\n");
        return 1;
    }
    if(type < 0 || type > 2 ){
        printk("incorrect type\n");
        return 1;
    }
    else{
        newWaitingAnimal(start_floor, destination_floor, type);
        return 0;
    }
    
}
/*************************************************************************************************/
ssize_t elevator_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {
	int len = strlen(message);
	
	read_p = !read_p;
	if (read_p)
		return 0;
		
	copy_to_user(buf, message, len);
	return len;
}
/*************************************************************************************************/
int elevator_proc_open(struct inode *sp_inode, struct file *sp_file) {
	read_p = 1;

    printk(KERN_WARNING "IM IN ELEVATOR PROC OPEN\n");
	

    message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk(KERN_WARNING "MY MESSAGE IS MESStED UP\n");
		return -ENOMEM;
	}

    print_animals();

    //sprintf(message, " IM ALIVE AND WELL\n");

	return 0;
}
/*************************************************************************************************/
int elevator_proc_release(struct inode *sp_inode, struct file *sp_file) {
	kfree(message);
	return 0;
}
/*************************************************************************************************/

static int elevator_init(void) {
    printk("INSIDE ELEVATOR INIT\n");
    fops.owner = THIS_MODULE;
    fops.open = elevator_proc_open;
    fops.read = elevator_proc_read;
    fops.release = elevator_proc_release;

    STUB_start_elevator = start_elevator;
    STUB_stop_elevator = stop_elevator;
    STUB_issue_request = issue_request;
	
	
    numPassengers = 0;
    currWeight = 0;
    status = 1;
    currFloor = 0;
    numLizards = 0;	
    numCats = 0;	
    numDogs = 0;
    numWaiting = 0;	
    numServiced = 0;

    status = 0;

    int i;
    //
    for(i = 0 ; i < NUM_FLOORS; ++i){
	    INIT_LIST_HEAD(&floors[i]);
    }
    INIT_LIST_HEAD(&elevator);

    if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk(KERN_WARNING "elevator_init\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}
//69YOLOSWAGKUSH 

   elevator_init_parameter(&thread1);

    if (IS_ERR(thread1.kthread)) {
		printk(KERN_WARNING "error spawning thread");
		remove_proc_entry(ENTRY_NAME, NULL);
		return PTR_ERR(thread1.kthread);
	}

   
    
    return 0;
}
module_init(elevator_init);
/*************************************************************************************************/
static void elevator_exit(void) {
    STUB_start_elevator = NULL;
    STUB_stop_elevator = NULL;
    STUB_issue_request = NULL;

    kthread_stop(thread1.kthread);
	remove_proc_entry(ENTRY_NAME, NULL);
	mutex_destroy(&thread1.mutex);
	printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);

}
module_exit(elevator_exit);





/*************************************************************************************************/
