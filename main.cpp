#include "BasicTypes.h"
int port = 4567;

int main(int argc, char* argv[])
{
  printf("Run as: ");
  for (int i=0; i<argc; i++)
    printf("'%s' ", argv[i]);
  printf("\n");

  PopulateMsgs();
  
  if (argc > 1)
  {
    int p = atoi(argv[1]);
    if (p > 0)
      port = p;
  }

  RunWSServer(port);

  printf("WS Server completed without kill. (?)");
  return 0;
}
