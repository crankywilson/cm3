#include "Game.h"
#include <chrono>

using namespace std::chrono;

typedef time_point<high_resolution_clock> timepoint;

struct TradeData
{
  Game& g;
  int auctionID;
  unique_lock<mutex>& lk;
};

inline timepoint now()
{
  return high_resolution_clock::now();
}

timepoint millis(int millis)
{
  return now() + milliseconds(millis);
}

inline bool Continue(Game& g, int auctionID)
{
  return g.AuctionID() == auctionID && g.auctionTime > 0;
}

inline bool Continue(TradeData& t)
{
  return t.g.AuctionID() == t.auctionID && t.g.auctionTime > 0;
}


bool GetConfirms(TradeData& t)
{
  int delay = 700;
  timepoint waituntil = millis(delay);

  Player* buyer = nullptr, *seller = nullptr;
  while (t.g.GetNextBuyerAndSeller(&buyer, &seller) &&
         t.g.tradingBuyer  == buyer  &&
         t.g.tradingSeller == seller &&
         Continue(t))
  {
    t.g.tradeCond.wait_until(t.lk, waituntil);

    if (buyer->confirmed && seller->confirmed)
      return true;

    if (now() >= waituntil) break;
  }

  return false;
}

void TradeUnits(TradeData& t)
{
  int delay = 1250;
  timepoint waituntil = millis(delay);

  Player* buyer = nullptr, *seller = nullptr;
  while (t.g.GetNextBuyerAndSeller(&buyer, &seller) &&
         t.g.tradingBuyer  == buyer  &&
         t.g.tradingSeller == seller &&
         Continue(t))
  {
    t.g.tradeCond.wait_until(t.lk, waituntil);
    if (now() < waituntil) continue;

    if (!Continue(t)) break;

    t.g.tradeConfirmID++;
    buyer->confirmed = false;
    seller->confirmed = false;
    buyer->send(ConfirmTrade{});
    seller->send(ConfirmTrade{});

    if (GetConfirms(t))
    {
      // trade money and goods

      // if buyer can't afford any more, move back to buy line and return
      // if seller has no more to sell, move back to sell line and return

      delay -= 100;
      if (delay < 300)
        delay = 300;

      waituntil = millis(delay);
      continue;
    }
    else  // confirmation was not received
    {
      if (!buyer->confirmed)
      {
        // send buyer back to buy line
      }
      if (!seller->confirmed)
      {
        // send seller back to sell line
      }

      break;
    }
  }

  t.g.tradingBuyer->confirmed = false;
  t.g.tradingSeller->confirmed = false;
  t.g.tradingBuyer = nullptr;
  t.g.tradingSeller = nullptr;
}

struct SetActiveTrading
{
  Game &g;
  SetActiveTrading(Game& g) : g(g) { g.activeTrading = true; }
  ~SetActiveTrading() { g.activeTrading = false; }
};

void StartTrade(TradeData &t)
{
  // allow short delay for websocket latency
  int delay = 200;
  timepoint waituntil = millis(delay);

  SetActiveTrading scope(t.g);  // sets t.g.activeTrading = true (and false on function return)

  while (now() < waituntil && Continue(t))
    t.g.tradeCond.wait_until(t.lk, waituntil);

  if (!Continue(t))
    return;

  Player *buyer = nullptr, *seller = nullptr;
  if (!t.g.GetNextBuyerAndSeller(&buyer, &seller))
    return;

  t.g.tradingBuyer = buyer;
  t.g.tradingSeller = seller;

  TradeUnits(t);

  t.g.tradingBuyer = nullptr;
  t.g.tradingSeller = nullptr;
}


void TradeThread(Game *game_ptr, int auctionID)
{
  Game& g = *game_ptr;
  unique_lock lk(g.tradeMutex);

  g.tradingBuyer = nullptr;
  g.tradingSeller = nullptr;

  while (Continue(g, auctionID))
  {
    Player* buyer = nullptr;
    Player* seller = nullptr;

    while (!g.GetNextBuyerAndSeller(&buyer, &seller))
    {
      g.tradeCond.wait(lk);
      if (!Continue(g, auctionID)) return;
    }

    TradeData t = {g:g, auctionID:auctionID, lk:lk};

    StartTrade(t);
  }
}

