#pragma once

#include "BasicTypes.h"
#include "messages_send.h"
struct Game;

struct Player
{
  Color       color;
  bool        active = false;
  string      name   = "";
  string      ip     = "";
  bool        starter;
  int         money  = 1000;
  vector<int> res    = { 5, 2, 0, 0, 0 };
  bool        hasMule = false;
  int         score;
  int         rank;

  JS_OBJ(color, active, name, starter, money,
          res, hasMule, score, rank);

  Game*            g;
  WebSock*         ws;

  template<class T> 
  void send(const T& msg) { if (ws != nullptr) Send(*ws, msg); }

  Player();
  Player(enum Color c) : Player() { color = c; }

  bool buying;
  int currentBid = BUY;
  int plEvent;
  int used[4];
  int spoiled[4];
  int startingRes[4];
  int produced[4];
  bool energyShort;
};
