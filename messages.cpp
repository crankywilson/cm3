#include "messages.h"
#include "Game.h"


void Msg1::Recv(Player &p)
{
  p.send(Msg4());
}

void Msg4::Recv(Player &p)
{
}

