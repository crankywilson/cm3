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

const char* ColorName(Color c);
const char* ColorName(Player *p);

namespace JS {
template<>
struct TypeHandler<Color>
{
  public:
    static inline Error to(Color &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        to_type = (Color)(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void from(const Color &from_type, Token &token, Serializer &serializer)
    {
        std::string buf = std::to_string(from_type);
        token.value_type = Type::Number;
        token.value.data = buf.data();
        token.value.size = buf.size();
        serializer.write(token);
    }
  };
}

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

namespace JS {
template<>
struct TypeHandler<GameState>
{
  public:
    static inline Error to(GameState &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        to_type = (GameState)(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void from(const GameState &from_type, Token &token, Serializer &serializer)
    {
        std::string buf = std::to_string(from_type);
        token.value_type = Type::Number;
        token.value.data = buf.data();
        token.value.size = buf.size();
        serializer.write(token);
    }
  };
}

enum ResType
{
  NONE = -1,
  FOOD = 0,
  ENERGY = 1,
  ORE = 2,
  CRYS = 3,
  LAND = 4
};


namespace JS {
template<>
struct TypeHandler<ResType>
{
  public:
    static inline Error to(ResType &to_type, ParseContext &context)
    {
        char *pointer;
        unsigned long value = strtoul(context.token.value.data, &pointer, 10);
        to_type = (ResType)(value);
        if (context.token.value.data == pointer)
            return Error::FailedToParseInt;
        return Error::NoError;
    }

    static void from(const ResType &from_type, Token &token, Serializer &serializer)
    {
        std::string buf = std::to_string(from_type);
        token.value_type = Type::Number;
        token.value.data = buf.data();
        token.value.size = buf.size();
        serializer.write(token);
    }
  };
}

struct LandLotID
{
  int e;
  int n;

  bool operator<(const LandLotID o) const { return this->e < o.e && this->n < o.n; } 
  operator string()
  { 
   string ret="";
   if (e < 0) ret += "W" + to_string(-e);
   else if (e == 0) ret += "R0";
   else ret += "E" + to_string(e);
   if (n < 0) ret += "S" + to_string(-n);
   else ret += "N" + to_string(n);
   return ret;
  }
 JS_OBJ(e, n);
};

struct LandLot
{
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

inline void printnothing(const char*, ...) {}
#define JSON 1

#define LOG_NOTHING 0
#define LOG_NONMOVEMSGS_NOJSON 1
#define LOG_ALLMSGS_NOJSON 2
#define LOG_EVERYTHING 3

#define LOG_LEVEL LOG_NONMOVEMSGS_NOJSON

#if LOG_LEVEL == LOG_EVERYTHING
#define LOGJSON printf
#else
#define LOGJSON printnothing
#endif

#if LOG_LEVEL == LOG_NOTHING
#define LOGMSG(msgID) printnothing
#elif LOG_LEVEL == LOG_NONMOVEMSGS_NOJSON
#define LOGMSG(msgID) if (msgID != 23 && msgID != 24) printf
#else
#define LOGMSG(msgID) printf
#endif

#if LOG_LEVEL == LOG_NOTHING
#define LOG printnothing
#else
#define LOG printf
#endif


