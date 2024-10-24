#pragma once
#include <string_view>
#include <vector>
#include <json_struct.h>

using namespace std;

namespace uWS
{
  template <bool, bool, typename>
  struct WebSocket;
}

struct Player;

struct PlayerRef 
{
  Player *player;
};

typedef uWS::WebSocket<false, true, PlayerRef> WebSock;

typedef string_view MsgData;
typedef void (*RecvFunc)(WebSock *, const MsgData);

enum Color { R, Y, G, B, C, N };

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
  vector<int>   k = vector<int>((size_t)2);
  Color         owner = N;
  int           mNum  = 0;         // number of mounds
  vector<float> mg;                // mound Geometry
  int           res   = -1;        // resource being produced
  int           crys  = 0;         // crystite level - not sent to client

  JS_OBJ(owner, mNum, mg, res);
};

struct MsgMapData
{
  RecvFunc recvFunc = nullptr;
  string className;
  bool json = false;
  int msgSize = -1;
};

extern vector<MsgMapData> msgs;
void PopulateMsgs();

extern JS::SerializerContext jsSendContext;
extern string jsSendContextBacking;

void RunWSServer(int port);



