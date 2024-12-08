#pragma once

#include "BasicTypes.h"
#include "Player.h"  // game depends on player, but not other way around
#include <condition_variable>

extern Player NoPlayer;
inline bool Exists(Player &p) { return &p != &NoPlayer; }

struct LandLotData
{
  int e;
  int n;
  LandLot l;
  JS_OBJ(e,n,l);
};

struct Game
{
  map<LandLotID,
      LandLot>    landlots;
  int             month      = 0;
  string          name       = "Default";
  vector<Player>  players;
  Player          colony     = {C};
  GameState       state      = SSetup;
  int             mules      = 14;
  int             mulePrice  = 100;
  vector<int>     resPrice   = { 15, 10, 40, 100, 500 };

  JS_OBJ(month, name, players, colony, state, mules,
          mulePrice, resPrice, landlotdata);

  /* /\ these 2 need the same list of members \/ */

  Game(const Game &g) : month(g.month), name(g.name), players(g.players), 
    colony(g.colony), state(g.state), mules(g.mules), mulePrice(g.mulePrice),
    resPrice(g.resPrice), landlotdata(g.landlotdata) {}

  List<LandLotData> landlotdata;
  List<int> possibleColonyEvents = {-1,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,3,-1};
  List<int> possibleGoodPlayerEvents = {0,1,2,3,4,5,6,7,8,9,10,11,12};
  List<int> possibleBadPlayerEvents  = {13,14,15,16,17,18,19,20,21};
  
  List<int> tradePrices[4];

  int colonyEvent = -1;

  bool started = false;
  Player* starter = nullptr;
  bool active = true;

  set<Color> continueRecvd;

  int minBid = 10;
  int minIncr = 1;

  Game();

  thread              tradeThread;            // create new when auction starts
  thread              auctionTimerThread;     // create new when auction starts
  mutex               tradeMutex;
  condition_variable  tradeCond;              // when moving during auction, call notify on this to wake up tradeThread
  atomic_bool         tradeMovement = false;  // when moving during auction, set this to true which auctionTimerThread will periodically look at
  atomic_bool         activeTrading = false;  // tradeThread updates this, auctionTimerThread reads from it and stops the clock while set
  atomic_int32_t      auctionTime;
  int                 tradeConfirmID = 0;
  Player*             tradingBuyer  = nullptr; // read these in MessageHandler with tradeMutex ... only notify if tradingBuyer/tradingSeller moves or a potential buyer and seller meet
  Player*             tradingSeller = nullptr; // write these in tradeThread with mutex

  bool GetNextBuyerAndSeller(Player **buyer, Player **seller);


  void SynchronizeWSPlayerPtrsWithGamePlayers(); 
    // ^ has to be called whenever players list/vector
    //   has items added or removed...
  
  void NewConnection(WebSock* ws, const string &ip);
  void Disconnect(Player &p);
  void Start();
  
  Player& player(enum Color c)
  {
    for (Player& p : players)
      if (p.color == c) return p;

    return NoPlayer;
  }

  template<class T> 
  void send(const T& msg) 
  { 
    for (Player& p : players)
      if (p.ws != nullptr)
        Send(*(p.ws), msg); 
  }

  void AdvanceToNextState();
  void StartAuction();
  void EndAuction();
  int  AuctionID();

  private:
  int auctionType = NONE;
  void DistributeCrystitie(LandLotID k, int lvl);
  void StartNextMonth();
  void UpdateScores();
  void UpdateGameState(GameState);
  void BuildMules();
  void UpdateResPrices();
  void SendPlayerEvents();
  int NumLots(Player p, int r);



  string pe[22] = {
/*0*/"You just received a package from your home-world relatives containing 3 food and 2 energy units.",
/*1*/"A wandering space traveler repaid your hospitality by leaving two bars of smithore.",
/*2*/"Your MULE was judged \"best built\" at the colony fair. You won (₿) ?",  //150 r8
/*3*/"Your MULE won the colony tap-dancing contest. You collected (₿) ?.",  //200 r4
/*4*/"The colony council for agriculture awarded you (₿) 1? for each food lot you have developed. The total grant is (₿) 2?.",  //50 r2
/*5*/"The colony awarded you (₿) ? for stopping the wart worm infestation.", //200 r7
/*6*/"The museum bought your antique personal computer for (₿) ?.",  //600 r8
/*7*/"You won the colony swamp eel eating contest and collected (₿) ?. (Yuck!)",  //150 r8
/*8*/"A charity from your home-world took pity on you and sent (₿) ?.", //150 r7
/*9*/"Your offworld investments in artificial dumbness paid (₿) ? in dividends.",
/*10*/"A distant relative died and left you a vast fortune٬ but after taxes you only got (₿) ?.",  //200 r6
/*11*/"You found a dead moose rat and sold the hide for (₿) ?.",  //50 r1
/*12*/"You received an extra lot of land to encourage colony development.",
// above are good, below are bad
/*13*/"Mischievous glac-elves broke into your storage shed and stole half your food.",
/*14*/"One of your MULEs lost a bolt. Repairs cost you (₿) ?.", //225 r9
/*15*/"Your mining MULEs have deteriorated from heavy use and cost (₿) 1? each to repair. The total cost is (₿) 2?.",
/*16*/"The solar collectors on your energy MULEs are dirty. Cleaning cost you (₿) 1? Each for a total of (₿) 2?.",
/*17*/"Your space gypsy inlaws made a mess of the settlement. It cost you (₿) ? to clean it up.",
/*18*/"Flying cat-bugs ate the roof off your dwelling. Repairs cost (₿) ?.",
/*19*/"You lost (₿) ? betting on the two-legged kazinga races.",  //200 r5
/*20*/"Your child was bitten by a bat lizard and the hospital bill cost you (₿) ?.",
/*21*/"You lost a lot of land because the claim was not recorded."
};

// wampus 100 1-4, 200 5-8, 300 9-

  string ce[9] = {
/*0*/"A planetquake reduces mining production!",                             // 15% 3 times max
/*1*/"A pest attack on lot ? causes all food to be destroyed!",              // 15% 3 times max
/*2*/"Sunspot activity increases energy production!",                        // 15% 3 times max
/*3*/"Acid rain increases food production, but decreases energy production.",// 15% 3 times max
/*4*/"A fire at the settlement destroys all colony-held goods!",             // 10% 2 times max
/*5*/"An asteroid smashes into lot ?٬ making a new crystite deposit!",       // 10% 2 times max
/*6*/"Space radiation destroys the MULE at lot ?!",                          // 10% 2 times max
/*7*/"Space pirates steal all crystite!",                                    // 10% 2 times max
/*8*/"A federation ship has returned to review the colony's performance"
};

  map<int,string> et = {
  {0,      "Overall٬ the colony failed...dismally. The Federation debtors' prison is your new home!"},
  {20000,  "Overall٬ the colony failed...The Federation will no longer send trade ships. You are on your own!"},
  {40000,  "Overall٬ the colony survived...barely. You will be living in tents. Few trading ships will come your way!"},
  {60000,  "Overall٬ the colony was a success. You have met the minimum standards set by the Federation٬ but your life will not be easy!"},
  {80000,  "Overall٬ the colony succeeded. The Federation is pleased by your efforts. You will live comfortably!"},
  {100000, "Overall٬ the colony succeeded...extremely well. You can now retire in elegant estates!"},
  {120000, "Overall٬ the colony delighted the Federation with your exceptional achievement. Your retirement will be luxurious!"}
  };
};
