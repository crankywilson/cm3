#pragma once

#include "BasicTypes.h"
struct Player;
struct Game;

template<class T> struct BinMsg { int _msgID; BinMsg():_msgID(T::msgID) {} };
template<class T> struct JSMsg {};
struct CliToServ {};
struct ServToCli {};
struct BiDir     {};

inline void NR(int msgID)
{
  LOG("Got unexpected msg ID #%d which has no Recv implemented", msgID);
}

// REGISTERMSGS only gets defined in messages_reg.cpp.
// The following macros are defined such that globals are generated
// only in that file whose initailizations cause the message types to
// get registerred to the 'msgs' vector.
#ifdef REGISTERMSGS  
 #include "messages_reg.h"
#else
 #define JS_REG(CLASS, id)  void Recv(Player&, Game&); static const int msgID;
 #define BIN_REG(CLASS, id) void Recv(Player&, Game&); static const int msgID; JS_OBJ(_msgID);
 #define JS_REG_NORECV(CLASS, id)  void Recv(Player&, Game&) {} static const int msgID;
 #define BIN_REG_NORECV(CLASS, id) void Recv(Player&, Game&) {} static const int msgID; JS_OBJ(_msgID);
#endif

#define List vector

                    
struct JoinGameReq : JSMsg<JoinGameReq>, CliToServ 
{
  string gameName;
  
 JS_OBJ(gameName);
 JS_REG(JoinGameReq, 1)
};


struct CurrentPlayers : JSMsg<CurrentPlayers>, ServToCli 
{
  List<Player> players;
  int yourColor;

 JS_OBJ(players, yourColor);
 JS_REG_NORECV(CurrentPlayers, 2)
};
                 
struct NameUpdate : JSMsg<NameUpdate>, CliToServ                  
{
  string name;

 BIN_REG(NameUpdate, 3)
};

struct ChangeColorReq : BinMsg<ChangeColorReq>, CliToServ
{
 BIN_REG(ChangeColorReq, 4)
};

struct NameFinalized : BinMsg<NameFinalized>, CliToServ
{
 BIN_REG(NameFinalized, 5)
};

struct StartGame : BinMsg<StartGame>, BiDir
{
 BIN_REG(StartGame, 6)
};

struct PlayerEvent : JSMsg<PlayerEvent>, ServToCli 
{
  Color color;
  int money;
  string shortMsg;
  string lotKey;
  bool addLot;

 JS_OBJ(color, money, shortMsg, lotKey, addLot);
 JS_REG_NORECV(PlayerEvent, 11)
};

struct PEventText : JSMsg<PEventText>, ServToCli 
{
  string longMsg;
  bool beneficial;

 JS_OBJ(longMsg, beneficial);
 JS_REG_NORECV(PEventText, 12)
};

struct UpdateGameState : BinMsg<UpdateGameState>, ServToCli 
{
  GameState gs;

 BIN_REG_NORECV(UpdateGameState, 16)
};

struct MulesAvail : BinMsg<MulesAvail>, ServToCli 
{
  int num;
  int price;

 BIN_REG_NORECV(MulesAvail, 19)
};

struct ShortageMsg : JSMsg<ShortageMsg>, ServToCli 
{
  string msg;

 JS_OBJ(msg);
 JS_REG_NORECV(ShortageMsg, 98)
};

struct EndMsg : JSMsg<EndMsg>, ServToCli 
{
  string msg;
  int score;

 JS_OBJ(msg, score);
 JS_REG_NORECV(EndMsg, 99)
};
