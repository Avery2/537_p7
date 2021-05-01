#include "helper.h"
#include "request.h"
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <unistd.h> // ftruncate
#include <signal.h>

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

int fill_ptr = 0;
int use_ptr = 0;
int count = 0;
int buffers = -1;
char *shm_name;
void *shm_ptr;
// int *the_buffer = NULL;
// int *my_buffer = NULL;
int my_buffer[256];
pthread_cond_t empty, fill;
pthread_mutex_t mutex;
// pthread_cond_init
// pthread_mutex_init

void put(int value)
{
  my_buffer[fill_ptr] = value;
  fill_ptr = (fill_ptr + 1) % buffers;
  count++;
}

int get()
{
  int tmp = my_buffer[use_ptr];
  use_ptr = (use_ptr + 1) % buffers;
	count--;
	return tmp;
}

static void *
worker_func(void *arg) {
    for (;;)
    {
      pthread_mutex_lock(&mutex);
      while (count == 0)
        pthread_cond_wait(&fill, &mutex);
      int tmp = get();
      pthread_cond_signal(&empty);
      pthread_mutex_unlock(&mutex);
      requestHandle(tmp);
      Close(tmp);
    }
    return NULL;
}

// CS537: Parse the new arguments too
void getargs(int *port, int *threads, int *buffers, char** shm_name, int argc, char *argv[])
{
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  *port = atoi(argv[1]);
  *threads = atoi(argv[2]);
  *buffers = atoi(argv[3]);
  *shm_name = argv[4];
}

void custom_sig() {
  if (munmap(shm_ptr, sysconf(_SC_PAGE_SIZE)) == -1)
    exit(1);
  if (shm_unlink(shm_name) == -1)
    exit(1);
  exit(0);
}

int main(int argc, char *argv[])
{
  signal(SIGINT, custom_sig);
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  // new cl args
  int threads;

  getargs(&port, &threads, &buffers, &shm_name, argc, argv);
  // test values for threads and buffer are positve integers
  if (threads <= 0 || buffers <= 0 || port < 2000)
	  exit(0);

  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //

  // 
  // CS537 (Part A): Create some threads...
  //

  // create buffer
  // int temp_buffer[buffers];
  // my_buffer = temp_buffer;

  // init cv
  pthread_cond_init(&empty, NULL);
  pthread_cond_init(&fill, NULL);
  pthread_mutex_init(&mutex, NULL);

  pthread_t workers[threads];
  for (int i = 0; i < threads; ++i) {
      pthread_create(&workers[i], NULL, worker_func, NULL);
  }

  // create shared memory
  int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0660);
  if (shm_fd == -1)
    exit(1);

  int pg_sz = sysconf(_SC_PAGE_SIZE);

  int ret = ftruncate(shm_fd, pg_sz);
  if (ret == -1)
    exit(1);

  shm_ptr = mmap(NULL, pg_sz, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_ptr == MAP_FAILED)
    exit(1);

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    // 
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. Also let the worker thread close the connection.
    // 

  pthread_mutex_lock(&mutex);
  while (count == buffers)
    pthread_cond_wait(&empty, &mutex);
  put(connfd);
  pthread_cond_signal(&fill);
  pthread_mutex_unlock(&mutex);
  }
}
