#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sysexits.h>
#include <signal.h>

#include <dirent.h>

#include <sys/mman.h>

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

  int n, on, tries;
  
  int *ornd;

  if (register_signals() < 0){
    fprintf(stderr, "e: signals\n");
    return EX_USAGE;
  }

  dir = get_current_dir_name();
  if (dir == NULL){
    fprintf(stderr, "e: %s\n", strerror(errno)); 
    return EX_USAGE;
  }

  ornd = NULL;
  on = 0;
  tries = 0;

  //srand(atoi(argv[1]));
  srand(time(NULL));

#if 0
  do {
    fprintf(stdout, "%d\n", rand());
  } while (tries++ < 10);
#endif

#if 1
  while (run){
  

    n = scandir(dir, &nl, 0, alphasort);
    if (n < 0){
      fprintf(stderr, "e: %s\n", strerror(errno)); 
      goto fnl;
    }
    
    if (ornd == NULL || n != on){
      ornd = realloc(ornd, n*sizeof(int));
      if (ornd == NULL){
        fprintf(stderr, "e: %s\n", strerror(errno)); 
        goto fnl;
      }
      memset(ornd, 0, n*sizeof(int));
    }

    tries = 0;
rndize:
    rnd = rand() % n;

    on = n;

    switch (nl[rnd]->d_name[0]){
      case '.':
        //fprintf(stderr, "dot file %s\n", nl[rnd]->d_name);
        continue;
    }
    
    if (ornd[rnd] != 0){
      tries++;
      if (tries == n){
        tries = 0;
        memset(ornd, 0, n*sizeof(int));
        run = 0;
        goto end;
      }
      goto rndize;
    }
    
    ornd[rnd] = 1;

    fprintf(stderr, "[%d] %s\n", tries, nl[rnd]->d_name);

  }
#endif
  

#if 0
  while(n--){
    fprintf(stderr, "%s\n", nl[n]->d_name);
  }
#endif
end:
  if (ornd)
    free(ornd);
  
fnl:
  if (nl)
    free(nl);

fd:
  if (dir)
    free(dir);
  
  return 0;
}
