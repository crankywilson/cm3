#include "Game.h"
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;


inline bool Continue(Game& g, int auctionID)
{
  return g.AuctionID() == auctionID && g.auctionTime > 0;
}


void TimerThread(Game *game_ptr, int auctionID)
{
  Game& g = *game_ptr;
  g.auctionTime = 10000;

  while (true)
  {
    g.tradeMovement = false;
    this_thread::sleep_for(std::chrono::milliseconds(250));

    if (!Continue(g, auctionID))
      break;

    if (g.activeTrading) continue;

    if (g.tradeMovement) 
      g.auctionTime -= 40;
    else
      g.auctionTime -= 250;
  }
  
  g.tradeCond.notify_all();  // auction over
}

