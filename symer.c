#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <dirent.h>
#include <sysexits.h>

#include <stack.h>
#include <spead_api.h>

struct newpaths {
  char *odir;
  struct stack *paths;
};

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

int print_help(char *name)
{
  fprintf(stderr, "usage:\n\t%s -o [output directory] [input directories]\n\n", name); 
  return 0;
}

int setup_symlinks(void *so, void *data)
{
  char *dir, *odir, *ffname, *newpath;
  struct dirent **namelist;
  int n, len, nlen;
  
  struct newpaths *nps;

  
  dir     = so;
  nps     = data;
  ffname  = NULL;
  newpath = NULL;

  if (dir == NULL || nps == NULL || nps->odir == NULL)
    return -1;

  odir    = nps->odir;

  n = scandir(dir, &namelist, NULL, alphasort);
  if (n < 0){
#ifdef DEUBG
    fprintf(stderr, "%s: error (%s)\n", __func__, strerror(errno));
#endif
    return -1;
  }

  while (n--) {

    switch (namelist[n]->d_name[0]){
      case '.':
        free(namelist[n]);
        continue;
    }
  
    len = snprintf(NULL, 0, "%s/%s", dir, namelist[n]->d_name);
    if (len <= 0){
      free(namelist[n]);
      continue;
    }
    ffname = realloc(ffname, sizeof(char)*(len+1));
    if (ffname == NULL){
      free(namelist[n]);
      continue;
    }
    snprintf(ffname, len+1, "%s/%s", dir, namelist[n]->d_name);
    
    nlen = snprintf(NULL, 0, "%s/%s", odir, namelist[n]->d_name);
    if (nlen <= 0){
      free(namelist[n]);
      free(ffname);
      ffname = NULL;
      continue;
    }
    newpath = malloc(sizeof(char)*(nlen+1));
    if (newpath == NULL){
      free(namelist[n]);
      free(ffname);
      ffname = NULL;
      continue;
    }
    snprintf(newpath, nlen+1, "%s/%s", odir, namelist[n]->d_name);

    if (namelist[n])
      free(namelist[n]);
    
    if (symlink(ffname, newpath) < 0){
      free(ffname);
      ffname = NULL;
      free(newpath);
      newpath = NULL;
      continue;
    }

    push_stack(nps->paths, newpath);

#ifdef DEBUG
    fprintf(stderr, "%s linkto %s\n", ffname, newpath);
#endif
    
  }

  free(namelist);

  if (ffname)
    free(ffname);
  
  return 0;
}

int remove_symlinks(void *so, void *data)
{
  char *path;

  path = so;
  if (path == NULL)
    return -1;
  
  if (unlink(path) < 0){
#ifdef DEBUG
    fprintf(stderr, "%s: unable to unlink file\n", __func__);
#endif
    free(path);
    return -1;
  }

  free(path);

  return 0;
}

int main(int argc, char *argv[])
{
  int i, j, c;
  
  char *odir;

  struct stack *dirlist;
  struct newpaths nps;

  i     = 1;
  j     = 1;
  odir  = NULL;

  dirlist   = create_stack();
  nps.paths = create_stack();
  
  while (i < argc){
    if (argv[i][0] == '-'){
      c = argv[i][j];

      switch(c){
        case '\0':
          j = 1;
          i++;
          break;
        case '-':
          j++;
          break;

        case 'h':
          destroy_stack(dirlist, NULL);
          destroy_stack(nps.paths, NULL);
          destroy_shared_mem();
          return print_help(argv[0]);

        case 'o':
          j++;
          if (argv[i][j] == '\0'){
            j =0;
            i++;
          }
          if (i >= argc){
            destroy_stack(nps.paths, NULL);
            destroy_stack(dirlist, NULL);
            destroy_shared_mem();
            fprintf(stderr, "%s: option -%c requires a parameter\n", argv[0], c);
            return EX_USAGE;
          }
          switch(c){
            case 'o':
              odir = argv[i] + j;
              break;
          }
          i++;
          j = 1;
          break;
        
        default:
          fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
          destroy_stack(nps.paths, NULL);
          destroy_stack(dirlist, NULL);
          destroy_shared_mem();
          return EX_USAGE;
      }

    } else {
      push_stack(dirlist, argv[i]);
      i++;
      j = 1;
    }
    
  }

 /* if (odir == NULL){
    destroy_stack(dirlist, NULL);
    destroy_shared_mem();
    return print_help(argv[0]);
  }*/

  nps.odir = odir;

  if (funnel_stack(dirlist, NULL, &setup_symlinks, &nps) < 0){
    destroy_stack(nps.paths, &free);
    destroy_stack(dirlist, NULL);
    destroy_shared_mem();
    return print_help(argv[0]);
  }
  
  register_signals();

  while(run){
    pause();
  }

#ifdef DEBUG
  fprintf(stderr, "%s: shutting down\n", __func__);
#endif

  if (funnel_stack(nps.paths, NULL, &remove_symlinks, NULL) < 0){
    destroy_stack(nps.paths, NULL);
    destroy_stack(dirlist, NULL);
    destroy_shared_mem();
    return print_help(argv[0]);
  }

  destroy_stack(nps.paths, NULL);
  destroy_stack(dirlist, NULL);
  destroy_shared_mem();
  
  return 0;
}
