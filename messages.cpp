#include "messages.h"
#include "Game.h"

void remove(List<Color>& v, int c)
{
  for (int i=0; i<v.size(); i++)
    if (v[i] == c) { v.erase(v.begin()+i); return; }
}

void JoinGameReq::Recv(Player &p, Game &g)
{
  p.starter = g.players.size() == 0;
  List<Color> avail = { R, Y, G, B };
  for (Player other : g.players)
    remove(avail, other.color);
  p.color = avail[0];
  p.name = "(" + ColorName((Color)p.color) + ")";
  g.players.push_back(p);
  
  g.send(CurrentPlayers { players:g.players, yourColor:p.color });
}


