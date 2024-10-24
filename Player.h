#pragma once

#include "BasicTypes.h"
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
  bool        hasMule;
  int         score;
  int         rank;

  JS_OBJ(color, active, name, starter, money,
          res, hasMule, score, rank);

  Game*            g;
  WebSock*         ws;

  template<class T> 
  void             send(T msg) { SendMsg(msg, *ws); }

  Player();
  Player(enum Color c) : Player() { color = c; }

};
