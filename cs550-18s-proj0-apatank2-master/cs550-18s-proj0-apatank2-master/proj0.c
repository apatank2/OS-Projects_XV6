#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;
  printf(1,"CS550 proj0 print in user space:");
  for(i = 1; i < argc; i++)
   // printf(1, "%s%s", argv[i], i+1 < argc ? " " : "\n");
  {
      printf(1," %s",argv[i]);
  }
  printf(1,"\n");
  exit();
}

