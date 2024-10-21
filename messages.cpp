#include "messages.h"

const int Register(const int id, void(*fn)(MsgData), const char *className, bool json)
{
  printf("Registerred %s as %d\n", className, id);
  return id;
}

void Msg1::Recv(MsgData msg)
{
}

void Msg2::Recv(MsgData msg)
{
}
