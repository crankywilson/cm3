#ifndef PLAYER_H
#define PLAYER_H

#include <json_struct.h>
using namespace JS;

#include "uWebSockets.h"
using namespace uWS;

#include "messages.h"

extern std::string msgBuf;

enum Color
{
  R, Y, G, B, C, N
};

struct Player;

typedef WebSocket<false, true, Player> PlayerWS;

template<class Msg>
void SendMsg(Msg msg, PlayerWS& ws)
{
  MsgMapVec& map = *msgMap;
  int msgID = msg.rtid;
  if (msgID < 0 || msgID >= MAXMSGS)
  {
    printf("Unknown msg ID of %d passed to SendMsg\n", msgID);
    return;
  }

  MsgMapEntry& e = map[msgID];
  if (e.className == "")
  {
    printf("Unknown msg ID of %d passed to SendMsg\n", msgID);
    return;
  }
  
  if (e.json)
  {
    serializeJS(msg, msgBuf);
    ws.send(msgBuf, TEXT);
  }
  else
  {
    ws.send(MsgData((char*)&msg, sizeof(Msg)));
  }
}

struct Game;

struct Player
{
  enum Color       color;
  bool             active = false;
  std::string      name   = "";
  std::string      ip     = "";
  bool             starter;
  int              money  = 1000;
  std::vector<int> res    = { 5, 2, 0, 0, 0 };
  bool             hasMule;
  int              score;
  int              rank;

  JS_OBJ(color, active, name, starter, money,
          res, hasMule, score, rank);

  Game*            g;
  PlayerWS*        ws;

  template<class T> 
  void             send(T msg) { SendMsg(msg, *ws); }

  Player();
  Player(enum Color c) : Player() { color = c; }

};

#endif
