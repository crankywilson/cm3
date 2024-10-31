#include "messages.h"
#include "Game.h"

void remove(List<Color>& v, int c)
{
  for (int i=0; i<v.size(); i++)
    if (v[i] == c) { v.erase(v.begin()+i); return; }
}

void SendCurrentPlayers(Game &g)
{
  CurrentPlayers cp;
  cp.players = g.players;
  for (Player& op : g.players)
  {
    cp.yourColor = op.color;
    op.send(cp);
  }
}

void JoinGameReq::Recv(Player &p, Game &g)
{
  p.starter = g.players.size() == 1;
  List<Color> avail = { R, Y, G, B };
  for (Player& other : g.players)
    remove(avail, other.color);
  p.color = avail[0];
  p.name = string("(") + ColorName((Color)p.color) + string(")");  
  SendCurrentPlayers(g);
}

void NameUpdate::Recv(Player &p, Game &g)
{
    p.name = this->name;
    SendCurrentPlayers(g);
}

void ChangeColorReq::Recv(Player &p, Game &g)
{
  List<Color> avail = { R, Y, G, B };
  for (Player& other : g.players)
  {
    if (p.color == other.color) continue;
    remove(avail, other.color);
  }

  if (avail.size() > 1)
  {
    auto curColorIt = find(avail.begin(), avail.end(), p.color);
    if (++curColorIt == avail.end()) 
      p.color = avail[0];
    else
      p.color = *curColorIt;

    if (p.name[0] == '(') 
      p.name = string("(") + ColorName(p.color) + ")";
  }

  SendCurrentPlayers(g);
}

void NameFinalized::Recv(Player &p, Game &g)
{

}

void StartGame::Recv(Player &p, Game &g)
{
  g.send(StartGame());
  g.Start();
}

void PressedSpaceToContinue::Recv(Player &p, Game &g)
{
  g.continueRecvd.insert(p.color);
  if (g.continueRecvd.size() == g.players.size())
    g.send(AdvanceState{});
}

