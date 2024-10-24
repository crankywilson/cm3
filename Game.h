#pragma once

#include "BasicTypes.h"
#include "Player.h"  // game depends on player, but not other way around

extern Player NoPlayer;
inline bool Exists(Player &p) { return &p != &NoPlayer; }

struct Game
{
  vector<LandLot> landlots   = vector<LandLot>((size_t)9*5);
  int             month      = 1;
  string          name       = "Default";
  vector<Player>  players;
  Player          colony     = {C};
  GameState       state      = SSetup;
  int             mules      = 14;
  int             mulePrice  = 100;
  vector<int>     resPrice   = { 15, 10, 40, 100, 500 };

  JS_OBJ(landlots, month, name, players, colony, state, mules,
          mulePrice, resPrice);

  Game();
  void NewConnection(PlayerRef &pr, WebSock* ws, const string &ip);
  void Disconnect(Player &p);
  void Start();
  
  Player& player(enum Color c)
  {
    for (Player& p : players)
      if (p.color == c) return p;

    return NoPlayer;
  }

  LandLot& landlot(int e, int n) { return landlots[(9 * (n+2)) + e + 4]; }
};
