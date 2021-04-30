#include "helper.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

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


int main(int argc, char *argv[])
{
  int listenfd, connfd, port, clientlen;
  struct sockaddr_in clientaddr;
  // new cl args
  int threads, buffers;
  char* shm_name;

  getargs(&port, &threads, &buffers, &shm_name, argc, argv);
  // test values for threads and buffer are positve integers
  if (threads < 0 || buffers < 0 || port < 22)
	  exit(1);

  //
  // CS537 (Part B): Create & initialize the shared memory region...
  //

  // 
  // CS537 (Part A): Create some threads...
  //

  listenfd = Open_listenfd(port);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    // 
    // CS537 (Part A): In general, don't handle the request in the main thread.
    // Save the relevant info in a buffer and have one of the worker threads 
    // do the work. Also let the worker thread close the connection.
    // 
    requestHandle(connfd);
    Close(connfd);
  }
}
