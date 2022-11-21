#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

#define MAX 512

#define MIN(a,b)	((a)<(b)?(a):(b))

void
testlseek(char* filename, int o, int n)
{
  int fd, ref, r;
  char buf[MAX];
  if((fd = open(filename, O_RDONLY)) < 0){
    printf(2, "testlseek: cannot open %s\n", filename);
  }

  // Si o supérieur ou égal à 0, on lit a partir du début
  //Sinon, on lit à partir de la fin
  ref = ( o>=0 ) ? SEEK_SET : SEEK_END;

  //On se positionne ou début, ou à la fin, avec un offset définit
  if (lseek(fd, o, ref) < 0){ 
    printf(
      2,
      "testlseek: cannot lseek %d from %s\n",
      o,
	    (ref == SEEK_SET) ? "SEEK_SET" : "SEEK_END"
    );
    exit(0);
  }
  printf(2, "testlseek: %s fd=%d o=%d n=%d ref=%d\n", filename,fd,o,n,ref);

  //On écrit tout sur la sortie standard, tant que l'on a pas atteint n.
  while(n > 0 && (r = read(fd, buf, MIN(n, sizeof buf))) > 0)
  {
    write (1, buf, r) ;
    n -= r ;
  }
  //On ferme le fichier
  if (close(fd) == -1) {
    printf(2, "testlseek: cannot close %s\n", filename);
    exit(0);
  }

  lseek(0, 0, 0);
}

int
main(int argc, char *argv[])
{
  if(argc != 4)
  {
    printf(2, "testlseek: incorrect number of arguments (3 require)\n");
    exit(0);
  }
  testlseek(argv[1], atoi(argv[2]), atoi(argv[3]));
  exit(0);
}
