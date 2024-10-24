#include "messages.h"
#include "Game.h"


void Msg1::Recv(Player &p)
{
  p.send(Msg2());
}

void Msg2::Recv(Player &p)
{
}

