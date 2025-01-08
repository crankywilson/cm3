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
  bool   devEditor;

 JS_OBJ(gameName, devEditor);
 JS_REG(JoinGameReq, 1)
};


struct CurrentPlayers : JSMsg<CurrentPlayers>, ServToCli 
{
  List<Player> players;
  Color yourColor;

 JS_OBJ(players, yourColor);
 JS_REG_NORECV(CurrentPlayers, 2)
};
                 
struct NameUpdate : JSMsg<NameUpdate>, CliToServ                  
{
  string name;

 JS_OBJ(name);
 JS_REG(NameUpdate, 3)
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


struct PressedSpaceToContinue : BinMsg<PressedSpaceToContinue>, BiDir
{
  Color color;

 BIN_REG(PressedSpaceToContinue, 7)
};


struct AdvanceState : BinMsg<AdvanceState>, ServToCli
{
  GameState newState;
  
 BIN_REG_NORECV(AdvanceState, 8)
};


struct UpdateBidReq : BinMsg<UpdateBidReq>, CliToServ
{
  int bid;

 BIN_REG(UpdateBidReq, 9)
};

struct CurrentAuctionState : BinMsg<CurrentAuctionState>, ServToCli
{
  short R;
  short Y;
  short G;
  short B;
  short lowestAsk;
  short highestBid;
  short minBid;
  
 BIN_REG_NORECV(CurrentAuctionState, 10)
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


struct CurrentAuction : BinMsg<CurrentAuction>, ServToCli
{
  int auctionType;
  int month;
  int minBid;
  int unitsAvail;
  
 BIN_REG_NORECV(CurrentAuction, 13)
};

struct BuySell : BinMsg<BuySell>, BiDir
{
  Color color;
  bool buy;

 BIN_REG(BuySell, 14)
};

struct LotAuction : BinMsg<LotAuction>, ServToCli
{
  int e;
  int n;

 BIN_REG_NORECV(LotAuction, 15)
};

struct UpdateGameState : BinMsg<UpdateGameState>, ServToCli 
{
  GameState gs;

 BIN_REG_NORECV(UpdateGameState, 16)
};

// GameData (msg 17) defined in Game.cpp

// msg 18 has become 43

struct MulesAvail : BinMsg<MulesAvail>, ServToCli 
{
  int num;
  int price;

 BIN_REG_NORECV(MulesAvail, 19)
};


struct MuleBuyReq : BinMsg<MuleBuyReq>, CliToServ 
{
 BIN_REG(MuleBuyReq, 20)
};

struct MuleBought : BinMsg<MuleBought>, ServToCli 
{
  Color color;
  int newMoney;
  int newMules;

 BIN_REG_NORECV(MuleBought, 21)
};

struct ModelRot : BinMsg<ModelRot>, BiDir 
{
  Color color;
  float yrot;
  float dirx;
  float dirz;
  float x;
  float z;

 BIN_REG(ModelRot, 23)
};

struct ModelPos : BinMsg<ModelPos>, BiDir 
{
  Color color;
  bool stop;
  float x;
  float z;

 BIN_REG(ModelPos, 24)
};

struct MuleSellReq : BinMsg<MuleSellReq>, CliToServ 
{
 BIN_REG(MuleSellReq, 25)
};

struct MuleSold : BinMsg<MuleSold>, ServToCli 
{
  Color color;
  int newMoney;
  int newMules;
  
 BIN_REG_NORECV(MuleSold, 26)
};

struct ReqLot : BinMsg<ReqLot>, CliToServ 
{
  int e;
  int n;
  
 BIN_REG(ReqLot, 27)
};

// msg sent with 0,0 to activate key handler
struct LotGrantResp : BinMsg<LotGrantResp>, ServToCli
{
  int e;
  int n;
  bool granted;
  Color playerColor;
  
 BIN_REG_NORECV(LotGrantResp, 28)
};

struct CantAffordMule : BinMsg<CantAffordMule>, ServToCli
{
 BIN_REG_NORECV(CantAffordMule, 29)
};

struct NoMoreMules : BinMsg<NoMoreMules>, ServToCli
{
 BIN_REG_NORECV(NoMoreMules, 30)
};


struct OutfitMule : BinMsg<OutfitMule>, BiDir
{
  Color c;
  ResType res;
  int newMoney; // if < 0, disregard and show 'can't afford'

 BIN_REG(OutfitMule, 31)
};

struct InstallMule : BinMsg<OutfitMule>, BiDir
{
  Color c;
  LandLotID lot;
  ResType res;
  bool switchWithExisting;
  ResType existingRes;

 BIN_REG(InstallMule, 32)
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

struct Cantina : BinMsg<Cantina>, BiDir
{
  Color c;
  int winnings;
  int newMoney;

 BIN_REG(Cantina, 33)
};

struct ConfirmTrade : BinMsg<ConfirmTrade>, BiDir
{
  int tradeConfirmID;
  int price;
  bool accept;
  
 BIN_REG(ConfirmTrade, 34)
};

struct StartTradeMsg : BinMsg<StartTradeMsg>, ServToCli
{
  Color buyer;
  Color seller;
  
 BIN_REG_NORECV(StartTradeMsg, 35)
};

enum EndTradeReason
{
  BIDCHANGE = 0,
  NOCONFIRM = 1,
  NSF = 2,
  CRITICAL = 3,
  SOLDOUT = 4
};

struct EndTradeMsg : BinMsg<EndTradeMsg>, ServToCli
{
  int reason = 0;             
  Color player = N;

 BIN_REG_NORECV(EndTradeMsg, 36)
};

struct UnitsTraded : BinMsg<UnitsTraded>, ServToCli
{
  Color buyer;
  Color seller;
  short unitsTraded;
  short newBuyerMoney;
  short newBuyerUnits;
  short newSellerMoney;
  short newSellerUnits;
  short buyersurplus;
  short sellersurplus;
  
 BIN_REG_NORECV(UnitsTraded, 37)
};

struct AuctionTime : BinMsg<AuctionTime>, ServToCli
{
  int pct;
  
 BIN_REG_NORECV(AuctionTime, 38)
};

struct AuctionStartPlayerData
{
  Color c;
  short used;
  short spoiled;
  short produced;
  short current;
  short surplus;
  short money;
  bool  buying;

  JS_OBJ(c,used,spoiled,produced,current,surplus,money,buying);
};

struct AuctionData : JSMsg<AuctionData>, ServToCli 
{
  short auctionType;
  short month;
  short colonyNumUnits;
  short colonyBuyPrice;
  List<AuctionStartPlayerData> playerData;

 JS_OBJ(auctionType,month,colonyNumUnits,colonyBuyPrice,playerData);
 JS_REG_NORECV(AuctionData, 39)
};

struct ForceAdvanceState : BinMsg<ForceAdvanceState>, CliToServ
{
 BIN_REG(ForceAdvanceState, 40)
};

struct NoLandAuctions : BinMsg<NoLandAuctions>, ServToCli
{
 BIN_REG_NORECV(NoLandAuctions, 41)
};

struct LotForSale : BinMsg<LotForSale>, ServToCli
{
  int e;
  int n;
 BIN_REG_NORECV(LotForSale, 42)
};

struct ColonyEvent : JSMsg<ColonyEvent>, ServToCli 
{
  int colonyEvent;
  string fullMsg;
  string lotKey;
  bool beforeProd;
 
 JS_OBJ(colonyEvent, fullMsg, lotKey, beforeProd);
 JS_REG_NORECV(ColonyEvent, 43)
};

struct Production : JSMsg<Production>, ServToCli 
{
  List<LandLotID> rkeys; 

 JS_OBJ(rkeys);
 JS_REG_NORECV(Production, 44)
};

struct AssayReq : BinMsg<AssayReq>, CliToServ
{
  int e;
  int n;

 BIN_REG(AssayReq, 45)
};

struct AssayResult : BinMsg<AssayResult>, ServToCli
{
  int e;
  int n;
  int level;

 BIN_REG_NORECV(AssayResult, 46)
};

struct AuctionLotReq : BinMsg<AuctionLotReq>, CliToServ
{
  int e;
  int n;

 BIN_REG(AuctionLotReq, 47)
};

struct DisableTimers : BinMsg<DisableTimers>, BiDir
{
  bool disable;            // not read by server, only used to 
                           // communicate current state to client
 BIN_REG(DisableTimers, 48)
};

struct LandAuctionLeader : BinMsg<LandAuctionLeader>, ServToCli
{
  Color lead;

 BIN_REG_NORECV(LandAuctionLeader, 49)
};

struct NoSellerNoAuction : BinMsg<NoSellerNoAuction>, ServToCli
{
 BIN_REG_NORECV(NoSellerNoAuction, 50)
};

struct WumpusMsg : BinMsg<WumpusMsg>, BiDir
{
  Color c;
  int newMoney;

 BIN_REG(WumpusMsg, 51)
};

struct FoodAmt : BinMsg<FoodAmt>, ServToCli
{
  int units;

 BIN_REG_NORECV(FoodAmt, 52)
};