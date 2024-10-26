#include "messages.h"
#include "Game.h"


void Msg1::Recv(Player &p)
{
  p.send(Msg4 { ifield: 4, x: -2.34f });
}

void Msg4::Recv(Player &p)
{
}

