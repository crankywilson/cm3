#include "BasicTypes.h"
int port = 4567;

#include "messages.h"

int main(int argc, char* argv[])
{
  PopulateMsgs();
  
  printf("params %d %s\n", argc, argv[0]);
  RunWSServer(port);
  return 0;
}
