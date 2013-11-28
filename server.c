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
#ifdef DEBUG
  fprintf(stderr, "%s: child created with fd[%d]\n", __func__, cl->c_fd);
#endif
  write(cl->c_fd, "hello\n", 6);
  sleep(10);
  write(cl->c_fd, "byee\n", 5);
#ifdef DEBUG
  fprintf(stderr, "%s: child[%d] ending\n", __func__, getpid());
#endif
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
          child_process(c);
          break;

        /*parent*/
        default:
#ifdef DEBUG
          fprintf(stderr, "%s: close the child fd on the parent\n", __func__);
#endif
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
