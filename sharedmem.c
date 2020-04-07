//From https://www.geeksforgeeks.org/posix-shared-memory-api/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/shm.h> 
#include <sys/stat.h> 

typedef struct {
     uint8_t data0;
     uint8_t data1;
} number;


int producer1() 
{ 
    /* strings written to shared memory */
//    const char* message_0 = "Sarayu"; 
//    const char* message_1 = "Managoli\n"; 
  
    number prod1 = {1,2};

    number *prod1_ptr = &prod1;

   // prod1_ptr->data0 = 0;
    /* shared memory file descriptor */
    int file_share; 
  
    /* pointer to shared memory obect */
    number *ptr = NULL; 
  
    /* create the shared memory object */
    file_share = shm_open("Trial_Share", O_RDWR, 0666); 
  
    /* configure the size of the shared memory object */
    //ftruncate(file_share, 100); 
  
    /* memory map the shared memory object */
    ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0); 
  
    /* write to the shared memory object */
    /*sprintf(ptr, "%s", message_0); 
  
    ptr += strlen(message_0); 
    sprintf(ptr, "%s", message_1); 
    ptr += strlen(message_1); */

    memcpy((void *)(&ptr[0]),(void*)prod1_ptr,100);

    close(file_share);

    return 0; 
} 

int producer2()
{
    /* strings written to shared memory */
//    const char* message_0 = "Gitanjali";
//    const char* message_1 = "Suresh\n";

     number prod2 = {3,4};

     number *prod2_ptr = &prod2;
     //prod2_ptr->data0 = 1;
    /* shared memory file descriptor */
    int file_share;

    /* pointer to shared memory obect */
    number *ptr = NULL;


    /* create the shared memory object */
    file_share = shm_open("Trial_Share", O_RDWR, 0666);

    /* configure the size of the shared memory object */
    //ftruncate(file_share, 100);

    /* memory map the shared memory object */
    ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0);

    /* write to the shared memory object */
    /*sprintf(ptr, "%s", message_0);

    ptr += strlen(message_0);
    sprintf(ptr, "%s", message_1);
    ptr += strlen(message_1);*/

    memcpy((void *)(&ptr[1]),(void*)prod2_ptr,100);

    close(file_share);

    return 0;
}

int consumer()
{
	
    number cons;

    number *cons_ptr = &cons;

    /* shared memory file descriptor */
    int file_share;

    /* pointer to shared memory obect */
    number *ptr = NULL;

    /* create the shared memory object */
    file_share = shm_open("Trial_Share", O_RDWR, 0666);
    /* memory map the shared memory object */
    ptr = (number *)mmap(0, 100, PROT_READ, MAP_SHARED, file_share, 0);

    memcpy((void*)cons_ptr,(void*)(&ptr[0]),sizeof(number));
    memcpy((void*)cons_ptr,(void*)(&ptr[1]),sizeof(number));
    /* read from the shared memory object */ 
    printf("%d\n", ptr[0].data0);   
    printf("%d\n", ptr[0].data1);
    printf("%d\n", ptr[1].data0);
    printf("%d\n", ptr[1].data1);
   /* remove the shared memory object */
    shm_unlink("Trial_Share");
    return 0;
}

int main(void)
{
	int file_share = shm_open("Trial_Share",O_CREAT | O_RDWR, 0666);
	if(file_share < 0)
	{ 
		printf("SHM OPEN"); 
	}

	ftruncate(file_share, 4096);
	
	if (close(file_share) < 0) 
	{ 
		printf("FILE CLOSE ERROR"); 
	}

        producer1();
    	producer2();
	consumer();
        return 0;
}
