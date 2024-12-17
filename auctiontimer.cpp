#include "Game.h"
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;

extern uWS::Loop* appLoop;

inline bool Continue(Game& g, int auctionID, int auctionTime)
{
  return g.AuctionID() == auctionID && auctionTime > 0;
}


void TimerThread(Game *game_ptr, int auctionID)
{
  Game& g = *game_ptr;
  int auctionTime = 10000;
  int lastpct = 99;

  while (true)
  {
    g.tradeMovement = false;
    int pct = auctionTime / 100;
    if (pct < lastpct)
      appLoop->defer([=]{game_ptr->send(AuctionTime{pct:pct});});

    this_thread::sleep_for(std::chrono::milliseconds(250));

    if (!Continue(g, auctionID, auctionTime))
      break;

    if (g.activeTradingPrice > 0) continue;

    if (g.tradeMovement) 
      auctionTime -= 40;
    else
      auctionTime -= 250;
  }
  
  appLoop->defer([=]{game_ptr->EndAuction();});
}

