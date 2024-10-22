
#ifndef GAME_H
#define GAME_H

#include <json_struct.h>
using namespace JS;

#include "Player.h"

enum GameState
{
  SSetup,
  SRankings,
  SLandGrant,
  SLandAuctionShowLot,
  SLandAuction,
  SPreDevelop,
  SDevelop,
  SPreProduction,
  SProduction,
  SPostProduction,
  SPreAuction,
  SAuction,
  SEnd
};

struct LandLot
{
  std::vector<int>   k     = std::vector<int>((size_t)2);
  enum Color         owner = N;
  int                mNum  = 0;         // number of mounds
  std::vector<float> mg;                // mound Geometry
  int                res   = -1;        // resource being produced
  int                crys  = 0;         // crystite level - not sent to client

  JS_OBJ(owner, mNum, mg, res);
};

struct Game
{
  std::vector<LandLot> landlots   = std::vector<LandLot>((size_t)9*5);
  int                  month      = 1;
  std::string          name       = "(Unnamed)";
  std::vector<Player*> players;
  Player               colony     = { C };
  GameState            state      = SSetup;
  int                  mules      = 14;
  int                  mulePrice  = 100;
  std::vector<int>     resPrice   = { 15, 10, 40, 100, 500 };

  JS_OBJ(landlots, month, name, players, colony, state, mules,
          mulePrice, resPrice);

  Game();
  void NewConnection(Player &p, PlayerWS* ws, const std::string &ip);
  void Disconnect(Player &p);
  void Start();
  
  Player* player(enum Color c)
  {
    for (Player* p : players)
      if (p->color == c) return p;

    return nullptr;
  }

  LandLot& landlot(int e, int n) { return landlots[(9 * (n+2)) + e + 4]; }
};

#endif
