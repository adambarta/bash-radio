#include <stdio.h>

#define BUF 4096

static volatile int run = 0;

int main(char argc, char *argv[])
{
  unsigned char buf[BUF];
  FILE *fn[argc - 1];
  int i, rb, wb;

  if (argc < 2){
    fprintf(stderr, "usage %s [\"cmd\"] [\"cmd\"] ... [\"cmd\"]\n\tCopy standard input to multiple standard inputs of other commands\n\te.g echo \"abc\" | fan \"tr a z\" \"tr a y\"\n", argv[0]);
    return 0;
  }

  for (i=0; i<argc-1; i++){
    fn[i] = popen(argv[i+1], "w");
    if (fn[i] == NULL){
      fprintf(stderr, "err: popen (%s)\n", argv[i+1]);
      goto unroll;
    }
  }
   
  run = 1;

  while (run){
    rb = fread(buf, sizeof(unsigned char), BUF, stdin);
    if (rb == 0){
      run = 0;
      break;
    }

    for (i=0; i<argc-1; i++){
      wb = fwrite(buf, sizeof(unsigned char), rb, fn[i]);
      if (wb == 0){
        run = 0;
        break;
      }
    }
  }

  i = argc-1;
unroll:
  for (;i>0;i--){
    if (pclose(fn[i-1]) < 0){
      fprintf(stderr, "err: pclose\n");
    }
  }

  return 0;
}
