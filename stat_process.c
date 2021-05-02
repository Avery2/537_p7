// #include <stdio.h>
// #include <stdlib.h>
#include "helper.h"
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>	  /* For O_* constants */

typedef struct
{
	pthread_t pt_id;
	int num_static;
	int num_dynamic;
} slot_t;

int main(int argc, char *argv[])
{
	if (argc != 4)
		exit(1);
	char *shm_name = argv[1];
	int sleeptime_ms = atoi(argv[2]);
	int num_threads = atoi(argv[3]);
	int i = 0;

	if (num_threads <= 0)
		exit(1);

	if (sleeptime_ms < 0){
		exit(1);
	}

	int shm_fd = shm_open(shm_name, O_RDWR, 0660);
	if (shm_fd == -1)
		exit(1);

	void *shm_ptr = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shm_ptr == MAP_FAILED)
		exit(1);

	slot_t *shm_slot_ptr = (slot_t *)shm_ptr;

	for (;;)
	{
		fprintf(stdout, "\n%d\n", i);
		for (int j = 0; j < num_threads; ++j)
		{
			fprintf(stdout, "%ul : %u %u %u\n",
					(shm_slot_ptr[i]).pt_id,
					(shm_slot_ptr[i]).num_static + (shm_slot_ptr[i]).num_dynamic,
					(shm_slot_ptr[i]).num_static,
					(shm_slot_ptr[i]).num_dynamic);
		}
		i++;
		// sleep
		usleep(sleeptime_ms);
	}
	exit(0);
}
