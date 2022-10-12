#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

void
mv(char *src, char *dst)
{
  if(link(src, dst) < 0)
    printf(2, "mv: link %s %s failed)\n", src, dst);
  
  if(unlink(src) < 0)
    printf(2, "mv: unlink %s failed)\n", src); 
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    printf(2, "mv: incorrect number of arguments (2 require)\n");
    exit();
  }
  mv(argv[1], argv[2]);
  exit();
}
