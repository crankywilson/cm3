#include "Game.h"
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;


inline bool Continue(Game& g, int auctionID, int auctionTime)
{
  return g.AuctionID() == auctionID && auctionTime > 0;
}


void TimerThread(Game *game_ptr, int auctionID)
{
  Game& g = *game_ptr;
  int auctionTime = 10000;

  while (true)
  {
    g.tradeMovement = false;
    this_thread::sleep_for(std::chrono::milliseconds(250));

    if (!Continue(g, auctionID, auctionTime))
      break;

    if (g.activeTradingPrice > 0) continue;

    if (g.tradeMovement) 
      auctionTime -= 40;
    else
      auctionTime -= 250;
  }
  
  g.tradeCond.notify_all();  // auction over
}

