#include "BasicTypes.h"
int port = 4567;

int main(int argc, char* argv[])
{
  printf("Run as: ");
  for (int i=0; i<argc; i++)
    printf("'%s' ", argv[i]);
  printf("\n");

  PopulateMsgs();
  
  RunWSServer(port);
  return 0;
}
