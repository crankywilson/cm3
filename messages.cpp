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

  if (p.starter)
    g.startedFromUnityEditor = devEditor;

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
  if (g.continueRecvd.count(p.color) > 0) return;
  color = p.color;
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

  if (g.continueRecvd.size() > 0)  // this gets set back to 0 from above call if we advanced...
    g.CheckForRemainingLots();
}

int constrainToLowestSell(int bid, Game &g)
{
  int lowest = SELL;
  for (Player& p : g.players)
  {
    if (p.ws != nullptr && !p.buying)
      lowest = min(lowest, p.currentBid);
  }

  if (lowest > 35 * g.minIncr + g.resPrice[g.auctionType])
    if (g.colony.res[g.auctionType] > 0)
      lowest = 35 * g.minIncr + g.resPrice[g.auctionType];

  return min(bid, lowest);
}

int constrainToHighestBuy(int bid, Game &g)
{
  int highest = g.minBid;
  for (Player& p : g.players)
  {
    if (p.ws != nullptr && p.buying)
      highest = max(highest, p.currentBid);
  }

  return max(bid, highest);
}

void UpdateBidReq::Recv(Player& p, Game& g)
{
  if (&p != &g.colony)  // special case to send data after trade end
  {
    if (p.buying) bid = constrainToLowestSell(bid, g);
    else bid = constrainToHighestBuy(bid, g);
    p.currentBid = bid;
    g.tradeMovement = true;
  }

  CurrentAuctionState st;
  st.highestBid = BUY;
  st.lowestAsk = SELL;
  st.R = g.player(R).currentBid;
  st.Y = g.player(Y).currentBid;
  st.G = g.player(G).currentBid;
  st.B = g.player(B).currentBid;

  // determine highestBid and lowestAsk
  for (Player& p : g.players)
  {
    if (p.ws == nullptr) continue;
    if (p.buying && p.currentBid > st.highestBid)
    {
      //printf("buying = %d, currentBid = %d, setting highestBID\n", p.buying, p.currentBid);
      st.highestBid = p.currentBid;
    }
    if (!p.buying && p.currentBid < st.lowestAsk)
    {
      //printf("buying = %d, currentBid = %d, setting lowestAsk\n", p.buying, p.currentBid);
      st.lowestAsk = p.currentBid;
    }
  }

  if (st.highestBid == BUY && g.auctionType != LAND &&
      g.minBid == g.resPrice[g.auctionType])
  {
    st.highestBid = g.resPrice[g.auctionType];
  }

  if (st.lowestAsk == SELL && g.auctionType != LAND && 
      g.colony.res[g.auctionType] > 0 &&
      g.minBid == g.resPrice[g.auctionType])
  {
    st.lowestAsk = 35 * g.minIncr + g.resPrice[g.auctionType];
  }
  
  if (st.highestBid > g.minBid + (35 * g.minIncr))
  {
    g.minBid = st.highestBid - (35 * g.minIncr);
    st.minBid = g.minBid;
    for (Player& p : g.players)
    {
      if (p.currentBid < g.minBid && p.currentBid > BUY)
      {
        p.currentBid = p.buying ? BUY : g.minBid;
        switch (p.color)
        {
          case R: st.R = p.currentBid; break;
          case Y: st.Y = p.currentBid; break;
          case G: st.G = p.currentBid; break;
          case B: st.B = p.currentBid; break;
        }
      }
    }
  }
  else
  {
    st.minBid = g.minBid;
  }
   
  g.send(st);

  if (g.auctionType == LAND)
  {
    Player *buyer, *seller;
    g.GetNextBuyerAndSeller(&buyer, &seller);
    g.send(LandAuctionLeader{lead:buyer ? buyer->color : N});
  }

  if (&p == &g.colony)  // special case to send data after trade end
  {
    return;  // don't start/end trade activities here...
             // this case is just for sending the current state
  }

  if (g.activeTradingPrice > 0)
  {
    if (g.tradingBuyer->currentBid != g.activeTradingPrice || 
        g.tradingSeller->currentBid != g.activeTradingPrice)
    {
      g.EndExistingTrade();   // sets g.activeTradingPrice == 0
    }
    
  }

  if (st.highestBid == st.lowestAsk && g.activeTradingPrice == 0)
  {
    Player *buyer, *seller;
    if (g.GetNextBuyerAndSeller(&buyer, &seller))
      g.StartNewTrade(buyer, seller);
    else
      printf("something is bad here\n");
  }
}

void BuySell::Recv(Player& p, Game& g)
{
  color = p.color;
  g.send(*this);
  p.buying = buy;
  if (buy)
    p.currentBid = BUY;
  else
    p.currentBid = SELL;
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

void InstallMule::Recv(Player& p, Game& g)
{
  c = p.color;
  LandLot& ldata = g.landlots[lot];
  switchWithExisting = ldata.res != NONE;
  existingRes = (ResType)ldata.res;
  ldata.owner = p.color;
  ldata.res = res;
  p.hasMule = switchWithExisting;
  g.send(*this);
}

void OutfitMule::Recv(Player& p, Game& g)
{
  newMoney = p.money - (res+1)*25;
  if (newMoney < 0)
  {
    p.send(*this);
    return;
  }

  p.money = newMoney;
  c = p.color;
  g.send(*this);
}

void Cantina::Recv(Player& p, Game& g)
{
  int mult = (int)(g.month / 4);
  c = p.color;
  winnings = 50;
  winnings += 50 * mult;
  winnings += (int)timeLeft;
  p.money += winnings;
  newMoney = p.money;
  g.send(*this);
}

void ConfirmTrade::Recv(Player& p, Game& g)
{
  if (&p == g.tradingBuyer || &p == g.tradingSeller)
  {
    g.TradeConfirmed(tradeConfirmID, p);
  }
}

void ForceAdvanceState::Recv(Player& p, Game& g)
{
  int existingAuctionType = g.auctionType;
  // let's try to kill the auction timer
  g.auctionType = -1;
  g.tradeCond.notify_all();
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  
  g.auctionType = existingAuctionType;
  g.AdvanceToNextState();
}

void AssayReq::Recv(Player& p, Game& g)
{
  g.send(AssayResult{e:e,n:n,level:g.landlots[{e,n}].crys});
}

void AuctionLotReq::Recv(Player& p, Game& g)
{
  if (find(g.auctionLots.begin(), g.auctionLots.end(), 
      LandLotID(e,n)) == g.auctionLots.end())

        g.auctionLots.push_back(LandLotID(e,n));
}

void DisableTimers::Recv(Player& p, Game& g)
{
  g.useTimers = !g.useTimers;
  disable = !g.useTimers;

  g.send(*this);
}

void WumpusMsg::Recv(Player& p, Game& g)
{
  this->c = p.color;
  int amt = 100;
  if (g.month == 12) amt = 400;
  else if (g.month >= 8) amt = 300;
  else if (g.month >= 4) amt = 200;
  p.money += amt;
  this->newMoney = p.money;

  g.send(*this);
}