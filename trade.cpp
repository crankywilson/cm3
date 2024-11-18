#include "Game.h"
#include <chrono>

using namespace std::chrono;

typedef time_point<high_resolution_clock> timepoint;

inline timepoint now()
{
  return high_resolution_clock::now();
}

timepoint millis(int millis)
{
  return now() + milliseconds(millis);
}

bool GetConfirms(Player &buyer, Player &seller, int auctionID, Game& g, unique_lock<mutex>& lk)
{
  int delay = 500;
  timepoint waituntil = millis(delay);

  Player* currentbuyer = nullptr, *currentseller = nullptr;
  while (g.GetBuyerAndSeller(&currentbuyer, &currentseller) &&
         currentbuyer == &buyer &&
         currentseller == &seller &&
         g.AuctionID() == auctionID)
  {
    g.tradeCond.wait_until(lk, waituntil);

    if (buyer.confirmed && seller.confirmed)
      return true;

    if (now() < waituntil) continue;
  }

  return false;
}

void TradeUnits(Player &buyer, Player &seller, int auctionID, Game& g, unique_lock<mutex>& lk)
{
  int delay = 1250;
  timepoint waituntil = millis(delay);

  Player* currentbuyer = nullptr, *currentseller = nullptr;
  while (g.GetBuyerAndSeller(&currentbuyer, &currentseller) &&
         currentbuyer == &buyer &&
         currentseller == &seller &&
         g.AuctionID() == auctionID)
  {
    g.tradeCond.wait_until(lk, waituntil);
    if (now() < waituntil) continue;

    g.tradeConfirmID++;
    buyer.confirmed = false;
    seller.confirmed = false;
    buyer.send(ConfirmTrade{});

    if (GetConfirms(buyer, seller, auctionID, g, lk))
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
      if (!buyer.confirmed)
      {
        // send buyer back to buy line
      }
      if (!seller.confirmed)
      {
        // send seller back to sell line
      }

      // no confirmation means end of trade
      buyer.confirmed = false;
      seller.confirmed = false;
      return;
    }
  }

  buyer.confirmed = false;
  seller.confirmed = false;
}

bool StartTrade(unique_lock<mutex>& lk, int auctionID, Game &g)
{
  while (g.auctionTime <= 0 && g.AuctionID() == auctionID)
  {
    Player* buyer = nullptr, *seller = nullptr;
    if (g.GetBuyerAndSeller(&buyer, &seller))
    {
      TradeUnits(*buyer, *seller, auctionID, g, lk);
    }
  }
  
  return false;
}

void TradeThread(Game *game_ptr, int auctionID)
{
  Game& g = *game_ptr;
  unique_lock lk(g.tradeMutex);

  while (!StartTrade(lk, auctionID, g))
  {
    if (g.AuctionID() != auctionID)
      return;   // this is bad, should have got to 0 time first

    if (g.auctionTime <= 0)
    {
      g.EndAuction();
      return;
    }

    g.tradeCond.wait(lk);
  }


  // g.state == SEnd, end thread
}

