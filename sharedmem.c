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
     char* firstname;
     char* lastname;
} number;


int producer1() 
{   
    number prod1 = {1,2,"Sarayu","Managoli"};

    number *prod1_ptr = &prod1;

    /* shared memory file descriptor */
    int file_share; 
  
    /* pointer to shared memory obect */
    number *ptr = NULL; 
  
    /* create the shared memory object */
    file_share = shm_open("Trial_Share", O_RDWR, 0666);
  
    /* memory map the shared memory object */
    ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0); 

    memcpy((void *)(&ptr[0]),(void*)prod1_ptr,100);

    close(file_share);

    return 0; 
} 

int producer2()
{
    /* strings written to shared memory */
      
    number prod2 = {3,4,"Gitanjali","Suresh"};

    number *prod2_ptr = &prod2;
 
    /* shared memory file descriptor */
    int file_share;

    /* pointer to shared memory obect */
    number *ptr = NULL;

    /* create the shared memory object */
    file_share = shm_open("Trial_Share", O_RDWR, 0666);

    /* memory map the shared memory object */
    ptr = (number *)mmap(NULL, sizeof(number), PROT_WRITE, MAP_SHARED, file_share, 0);

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
    printf("%s\n", ptr[0].firstname);
    printf("%s\n", ptr[0].lastname);
    printf("%s\n", ptr[1].firstname);
    printf("%s\n", ptr[1].lastname);
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
