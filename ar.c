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


int main(int argc, char *argv[])
{
  char *dir;
  struct dirent **nl;
  
  long int rnd;

  int n, i;

  if (register_signals() < 0){
    fprintf(stderr, "e: signals\n");
    return EX_USAGE;
  }
  
  if(argc == 2)
    dir = argv[1];
  else
    return 0;

  srand(time(NULL));

  n = scandir(dir, &nl, 0, alphasort);
  if (n < 0){
    fprintf(stderr, "e: %s\n", strerror(errno)); 
    //goto fnl;
    return 1;
  }

  struct priority_queue *pq;
  struct dirent *data;

  pq = create_priority_queue();

  for (i=0; i<n; i++){
    rnd = rand() % n;
    insert_with_priority_queue(pq, rnd, nl[i]);
  }

  for (i=0; i<n; i++){
    if (pull_highest_priority(pq, (void **) &data) == 0){
      if (data->d_name[0] != '.')
        fprintf(stdout, "%s\n", data->d_name);
    }
  }
  
  while(n--){
    if (nl[n])
      free(nl[n]);
  }
  if (nl)
    free(nl);

  destroy_priority_queue(pq, NULL);
  destroy_shared_mem();

  return 0;
}
