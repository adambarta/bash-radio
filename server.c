#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>
#include <signal.h>

#include <dirent.h>

#include <sys/mman.h>

#include <queue.h>
#include <spead_api.h>

static int run = 1;

void handle_us()
{
  run = 0;
}

int register_signals()
{
  struct sigaction sa;

  sigfillset(&sa.sa_mask);
  sa.sa_handler   = handle_us;
  sa.sa_flags     = 0;

  if (sigaction(SIGINT, &sa, NULL) < 0)
    return -1;

  if (sigaction(SIGTERM, &sa, NULL) < 0)
    return -1;

  return 0;
}

void child_process(struct spead_client *cl)
{
#define BUFF   1024
#define REPLY   "HTTP/1.1 200 OK\nServer: pshr\nConnection: Keep-Alive\nContent-Type: audio/mp3\n\n"
#define MODE_CONNECTING 0
#define MODE_SENDING    1  
  int bytes, mode = MODE_CONNECTING;
  char data[BUFF];


#ifdef DEBUG
  fprintf(stderr, "%s: child created with fd[%d]\n", __func__, cl->c_fd);
#endif
  

  while (run){

    switch (mode){
      
      case MODE_CONNECTING:
        
        bytes = read(cl->c_fd, data, BUFF);
        switch (bytes){
          case 0:
#ifdef DEBUG
            fprintf(stderr, "%s: read EOF client[%s:%d]\n", __func__, get_client_address(cl), get_client_port(cl));
#endif
            exit(EXIT_SUCCESS);
            break;

          case -1:
#ifdef DEBUG
            fprintf(stderr, "%s: read error client[%s:%d] (%s)\n", __func__, get_client_address(cl), get_client_port(cl), strerror(errno));
#endif
            exit(EXIT_SUCCESS);
            break;
        }

#ifdef DEBUG
        fprintf(stderr, "%s: [%s:%d] [%s]\n", __func__, get_client_address(cl), get_client_port(cl), data);
#endif

        bytes = write(cl->c_fd, REPLY, sizeof(REPLY));
        switch(bytes){
          case -1:
#ifdef DEBUG
            fprintf(stderr, "%s: read error client[%s:%d] (%s)\n", __func__, get_client_address(cl), get_client_port(cl), strerror(errno));
#endif
            exit(EXIT_SUCCESS);
            break;
        }
        
        

        
        mode = MODE_CONNECTING;

        break;


      case MODE_SENDING:

      
         
        break;

    }

  }


#if 0
  write(cl->c_fd, "hello\n", 6);
  sleep(3);
  write(cl->c_fd, "byee\n", 5);
#ifdef DEBUG
  fprintf(stderr, "%s: child[%d] ending\n", __func__, getpid());
#endif
#endif


  destroy_spead_client(cl);
  //shutdown(cl->c_fd, SHUT_RDWR);
  exit(EXIT_SUCCESS);
}

int main(int argc, char * argv[])
{
  struct spead_socket *x;
  struct spead_client *c;

  if (register_signals() < 0)
    return 1;

  x = create_tcp_socket(NULL, "3333");
  if (x == NULL)
    return 1;

  if (bind_spead_socket(x) < 0){
    destroy_spead_socket(x);
    destroy_shared_mem();
    return 1;
  }

  if (listen_spead_socket(x) < 0) {
    destroy_spead_socket(x);
    destroy_shared_mem();
    return 1;
  }

  while (run){
  
    c = accept_spead_socket(x);
    if (c){
      
      switch(fork()){
        case -1:
#ifdef DEBUG
          fprintf(stderr, "%s: fork err (%s)\n", __func__, strerror(errno));
#endif
          break;
        
        /*child*/
        case 0:
          /*child process takes over the accept object*/
          child_process(c);
          break;

        /*parent*/
        default:
#ifdef DEBUG
          fprintf(stderr, "%s: close the child fd on the parent\n", __func__);
#endif
          /*server closes the file descriptor and frees the object but doesn't 
            shutdown the connection from accept*/
          close(c->c_fd);
          free(c);
          break;
      }
    }

  }

  destroy_spead_socket(x);
  destroy_shared_mem();

  return 0;
}
