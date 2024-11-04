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
  g.send(*this);
  g.continueRecvd.insert(p.color);
  if (g.continueRecvd.size() == g.players.size())
    g.AdvanceToNextState();
}

void ReqLot::Recv(Player& p, Game& g)
{
  if (e < -4 || e > 4 || n < -2 || n > 2) 
  {
    p.send(LotGrantResp{e:e,n:n,granted:false,playerColor:p.color});
    return;
  } 

  if (e == 0 && n == 0) 
  {
    p.send(LotGrantResp{e:e,n:n,granted:false,playerColor:p.color});
    return;
  } 

  LandLotID k(e,n);
  if (g.landlots.find(k) != g.landlots.end())
  {
    if (g.landlots[k].owner != N)
    {
      p.send(LotGrantResp{e:e,n:n,granted:false,playerColor:p.color});
      return;
    }
  }

  g.landlots[k].owner = p.color;
  g.send(LotGrantResp{e:e,n:n,granted:true,playerColor:p.color});
  
  PressedSpaceToContinue cont;
  cont.color = p.color;
  cont.Recv(p, g);
}

void UpdateBidReq::Recv(Player& p, Game& g)
{
  p.currentBid = bid;  // no error checking yet

  CurrentAuctionState st;
  st.highestBid = g.minBid;
  st.lowestAsk = 45;
  st.R = g.player(R).currentBid;
  st.Y = g.player(Y).currentBid;
  st.G = g.player(G).currentBid;
  st.B = g.player(B).currentBid;

  for (Player& p : g.players)
  {
    if (p.buying && p.currentBid > st.highestBid)
      st.highestBid = p.currentBid;
    if (!p.buying && p.currentBid < st.lowestAsk)
      st.lowestAsk = p.currentBid;
  }

  g.send(st);
}

void BuySell::Recv(Player& p, Game& g)
{
}

void MuleBuyReq::Recv(Player& p, Game& g)
{
  if (p.money < g.mulePrice)
  {
    g.send(CantAffordMule());
    return;
  }

  if (p.money < g.mules)
  {
    g.send(NoMoreMules());
    return;
  }

  MuleBought mb;
  mb.color = p.color;
  p.money -= g.mulePrice;
  p.hasMule = true;
  mb.newMoney = p.money;
  g.mules--;
  mb.newMules = g.mules;

  g.send(mb);
}

void ModelPos::Recv(Player&, Game& g)
{
  g.send(*this);
}

void ModelRot::Recv(Player&, Game& g)
{
  g.send(*this);
}

void MuleSellReq::Recv(Player& p, Game& g)
{
  MuleSold ms;
  ms.color = p.color;
  p.money += g.mulePrice;
  p.hasMule = false;
  ms.newMoney = p.money;
  g.mules++;
  ms.newMules = g.mules;
  g.send(ms);
}