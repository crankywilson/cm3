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

extern uWS::Loop* appLoop;

void CancelIfConfirmNotReceived(Game *g, int tradeConfirmID)
{
  Player *buyer = g->tradingBuyer;
  Player *seller = g->tradingSeller;
  
  if (buyer == nullptr || seller == nullptr)
  {
    printf("something very wrong\n");
    return;
  }

  buyer->confirmed = buyer->color == C;
  seller->confirmed = seller->color == C;

  int delay = 700;
  timepoint waituntil = millis(delay);

  unique_lock lk(g->tradeMutex);

  while (now() < waituntil && g->tradeConfirmID == tradeConfirmID && (
         !buyer->confirmed || !seller->confirmed))
    g->tradeCond.wait_until(lk, waituntil);

  if (now() >= waituntil && g->tradeConfirmID == tradeConfirmID && (
      !buyer->confirmed || !seller->confirmed))
    appLoop->defer([=]{ g->TradeConfirmNotReceived(tradeConfirmID); });
}

void StartNewTrade(Game *g, int tradeConfirmID)
{
  unique_lock lk(g->tradeMutex);
  int delay = 1250 - (200*g->unitsTraded);
  if (delay < 300)
    delay = 200;

  timepoint waituntil = millis(delay);
  while (now() < waituntil && g->tradeConfirmID == tradeConfirmID)
    g->tradeCond.wait_until(lk, waituntil);
  
  if (g->tradeConfirmID == tradeConfirmID)
    appLoop->defer([=]{ g->StartTradeConfirmation(tradeConfirmID); });
}

