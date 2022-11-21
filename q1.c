#include "types.h"
#include "user.h"

/*
 * Programme pour tester le fonctionnement de sleep/wait avec un
 * argument.
 *
 * Prend un argument entier sur la ligne de commande, la valeur
 * que exit doit renvoyer a wait. Si tout se passe bien :
 *   q1 123
 * doit afficher :
 *   exit(123) -> wait retourne 123
 * 
 */

int
main(int argc, char *argv[])
{
  int n, status;

  if(argc != 2){
    printf(2, "usage: %s n\n", argv[0]);
    exit(0);
  }

  n = atoi(argv[1]);

  switch (fork())
  {
    case -1:
      printf(2, "cannot fork\n");
      exit(0);
    case 0:
      exit(n);
    default:
      if(wait(&status)==-1) {
        printf(2, "cannot wait\n");
      }
      printf(1, "exit(%d) -> wait retourne %d\n", n, status);
  }
  exit(0);
}
