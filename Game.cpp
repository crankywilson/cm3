#include "Game.h"
#include "uWebSockets.h"
#include <ctime>

using namespace JS;

Player NoPlayer;

const char* ColorName(Color c)
{
  switch (c) {
    case R: return "Red";
    case Y: return "Yellow";
    case G: return "Green";
    case B: return "Blue";
  }
  return "?";
}

Game& GetGame(Player *p) { return *(p->g); }

const char *ltime()
{
  static char tbuf[30];
  time_t now = time(0);
  tm *ltm = localtime(&now);
  strftime(tbuf, 30, "%I:%M:%S %p", ltm);
  return tbuf;
}

Game::Game()
{
  srand(time(NULL));
}

void Game::Disconnect(Player &p)
{
  p.ws = nullptr;

  // send disconnect msg to other players?
  
  if (state == GameState::SSetup)
  {
    bool newStarter = p.starter;
    for (auto i=players.begin(); i!=players.end(); i++)
    {
      if (i->color == p.color)
      {
        players.erase(i);
        SynchronizeWSPlayerPtrsWithGamePlayers();
        break;
      }
    }
    if (players.size() > 0)
    {
      players[0].starter = true;
      void SendCurrentPlayers(Game &g);
      SendCurrentPlayers(*this);
    }
  }
}

void Game::SynchronizeWSPlayerPtrsWithGamePlayers()
{
  for (Player& p : players)
   if (p.ws != nullptr)
    p.ws->getUserData()->player = &p;
}

void Game::NewConnection(WebSock* ws, const string &ip)
{
  if (state == SSetup)
  {
    if (players.size() >= 4)
    {
      LOG("Can't handle connection to game with >= 4 players\n");
      return;
    }

    Player p;
    p.ip = ip;
    p.ws = ws;
    p.g = this;
    p.color = N;  // gets assigned in NewPlayerReq::Recv
    players.push_back(p);
    
    SynchronizeWSPlayerPtrsWithGamePlayers();
    
    // send gamestate msg?
  }
  else // active game rejoin?
  {
    for (Player &p : players)
    {
      if (p.ws == nullptr && p.ip == ip)
      {
        p.ws = ws;
        SynchronizeWSPlayerPtrsWithGamePlayers();
         // send gamestate msg?
        return;
      }
    }

    LOG("Can't handle unknown connection to active game\n");
    return;
  }
}

inline float r(float f)
{
  int i = (int)(f * 100);
  return (float)i / 100.0f;
}

inline float rand_0_1()
{
  return (float)rand()/RAND_MAX;
}

void Game::DistributeCrystitie(LandLotID k, int lvl)
{
  if (landlots[k].crys < lvl)
    landlots[k].crys = lvl;
  if (lvl > 1)
  {
    if (k.e > -4)
      DistributeCrystitie(LandLotID{k.e-1, k.n}, lvl-1);
    if (k.e < 4)
      DistributeCrystitie(LandLotID{k.e+1, k.n}, lvl-1);
    if (k.n > -2)
      DistributeCrystitie(LandLotID{k.e, k.n-1}, lvl-1);
    if (k.n < 2)
      DistributeCrystitie(LandLotID{k.e, k.n+1}, lvl-1);
  }
}

struct GameData : JSMsg<GameData>, ServToCli 
{
  Game gd;
  void Recv(Player&, Game&) {} 
  static const int msgID; /* = 17 */
 JS_OBJ(gd);
};
typedef void (*RecvFunc)(WebSock *, const MsgData);
int Register(int id, RecvFunc fn, const char *className, bool json, int size);
const int GameData::msgID = { Register(17, nullptr, "GameData", true, -1) };

void Game::StartNextMonth(bool sendState)
{
  month++;

  landlotdata.clear();
  for (auto i : landlots)
    landlotdata.push_back({i.first.e, i.first.n, i.second});
  
  List<string> resourceShortages;
  if (colony.res[FOOD] == 0) resourceShortages.push_back("food");
  if (colony.res[ENERGY] == 0) resourceShortages.push_back("energy");
  if (colony.res[ORE] < 2) resourceShortages.push_back("smithore");
  if (resourceShortages.size() > 0 && month <= 12)
  {
    string resList = resourceShortages[0];
    if (resourceShortages.size() == 2)
      resList += " and " + resourceShortages[1];
    else if (resourceShortages.size() > 2)
      resList = "food, energy, and smithore";

    send(ShortageMsg{msg:"The colony has a shortage of " + resList + "!"});
  }

  BuildMules();
  UpdateResPrices();
  send(MulesAvail{num:mules, price:mulePrice});
  UpdateScores();
    
  send(GameData{gd:*this});

  if (month > 12)
  {
    int colonyScore = 0;
    for(Player& p : players)
    {
      if (&p == &colony) continue;
      colonyScore += p.score;
    }
    int scoreKey = 0;
    for (auto eti : et)
      if (colonyScore >= eti.first) scoreKey = eti.first;

    send(EndMsg{msg:et[scoreKey], score:colonyScore});
    UpdateGameState(SEnd, sendState);
    return;
  }

  UpdateGameState(SRankings, sendState);
}

using namespace uWS;
Loop* appLoop = nullptr;

void StartAuctionIn2Secs(Game *g)
{
  std::this_thread::sleep_for(std::chrono::seconds(2));
  appLoop->defer([=]{g->AdvanceToNextState();});
}

void Game::DetermineLandAuctions()
{
  int r = rand() % 100;
  int numToAuction = 0;
  if (r > 93)
    numToAuction = 3;
  else if (r > 69)
    numToAuction = 2;
  else if (r > 31)
    numToAuction = 1;

  vector<LandLotID> possible;
  possible.reserve(40);

  for (auto i : landlots)
  {
    if (i.second.owner >= C && !(i.first.e == 0 && i.first.n == 0))
      possible.push_back(i.first);
  }

  if (possible.size() < numToAuction)
    numToAuction = possible.size();

  while (numToAuction > 0)
  {
    auctionLots.push_back(possible[rand() % possible.size()]);
    numToAuction--;
  }

  if (auctionLots.size() == 0)
  {
    send(NoLandAuctions{});
    appLoop->defer([=]
    {
      this_thread::sleep_for(chrono::milliseconds(2500));
      AdvanceAfterLandAuction();
    }
    );
  }
  else
  {
    currentAuctionLot = *(auctionLots.end()-1);
    auctionLots.pop_back();
    send(LotForSale{e:currentAuctionLot.e, n:currentAuctionLot.n});
  }
}

void Game::AdvanceAfterLandAuction()
{
  if (auctionLots.size() == 0)
  {
    auctionType = -1;
    state = SDevelop;
    send(AdvanceState{newState:SDevelop});
    SendPlayerEvents();
  }
  else
  {
    currentAuctionLot = *(auctionLots.end()-1);
    auctionLots.pop_back();
    send(LotForSale{e:currentAuctionLot.e, n:currentAuctionLot.n});
    send(AdvanceState{newState:SLandAuctionShowLot});
  }
}


void Game::AdvanceToNextState()
{
  continueRecvd.clear();

    switch (state)
    {
      case SRankings:
        state = SLandGrant;
        for (Player& p : players)
          if (p.ws != nullptr)
            p.send(LotGrantResp{e:0,n:0,granted:false,playerColor:p.color}); 
        break;
      case SLandGrant:
        DetermineLandAuctions();
        state = SLandAuctionShowLot;
        break;
      case SLandAuctionShowLot:
        auctionType = LAND;
        state = SAuctionIn3;
        PreAuction();
        auctionTimerThread = thread(StartAuctionIn2Secs, this);
        auctionTimerThread.detach();
        break;
      case SDevelop:
        state = SPreAuction;
        auctionType = ORE;
        PreAuction();
        // need to send all the data about current auction
        //  (type, how much was used and needed, etc.

        break;              // but not implemented yet
      case SPostProduction:
        state = SPreAuction;
        auctionType = ORE;
        PreAuction();
        break;
      case SPreAuction:
        if (startedFromUnityEditor)
        {
          // skip 3 sec delay
          state = SAuction;
          StartAuction();
          break;
        }
        state = SAuctionIn3;
        auctionTimerThread = thread(StartAuctionIn2Secs, this);
        auctionTimerThread.detach();
        break;
      case SAuctionIn3:
        state = SAuction;
        StartAuction();
        break;
      case SAuction:
      case SAuctionOver:
        state = SPreAuction;
        switch (auctionType)
        {
          case FOOD:   auctionType = ENERGY; break;
          case ENERGY: auctionType = NONE;   break; 
          case ORE:    auctionType = CRYS;   break;
          case CRYS:   auctionType = FOOD;   break;
          case LAND: { AdvanceAfterLandAuction(); return; }
        }

        if (auctionType == NONE) 
          StartNextMonth(false); /* <- updates state to SRankings */ 
        else
          PreAuction();

        break;
    }

    send(AdvanceState{newState:state});
}

void Game::UpdateScores()
{
    list<Player *> rankList;

    for (Player& p : players)
    {
      p.score = 0;
      for (auto lli : landlots) 
      {
        auto ll = lli.second;
        if (ll.owner == p.color) 
        {
          p.score += 500; if (ll.res > -1) 
            p.score += mulePrice; 
        }
      }

      p.res[LAND] = p.score;
      p.score += p.money;
      
      for (int r = 0; r<4; r++)
        p.score += resPrice[r] * p.res[r];

      rankList.push_back(&p);
    }

    rankList.sort([](Player* a, Player* b) 
                  { return a->score > b->score; });
    
    int rank = 0;
    for (Player* p : rankList) 
      p->rank = ++rank;
}



void Game::UpdateGameState(GameState gs, bool sendState)
{
  state = gs;

  if (sendState)
  {
    struct UpdateGameState ugs{gs:gs};
    send(ugs);
  }
}

void Game::BuildMules()
{
  while (mules < 14 && colony.res[ORE] > 2)
  {
    mules++;
    colony.res[ORE] -= 2;
  }
}

int Avg(List<int>& l)
{
  int sum = 0;
  for (int i : l) sum += i;
  return sum/l.size();
}

void Game::UpdateResPrices()
{
  mulePrice = (resPrice[ORE] * 2) + 20;
  mulePrice = ((int)ceil(mulePrice / 10.0))*10;
  if (mules < 5)
    mulePrice += 50;

  for (int res=FOOD; res<CRYS; res++)
  {
    if (tradePrices[res].size() == 0)
      resPrice[res] = (int)ceil(resPrice[res] * 1.15);
    else
    {
      int avg = Avg(tradePrices[res]);

      if (avg < resPrice[res] + 5)
        resPrice[res] -= tradePrices[res].size();

      else if (avg > resPrice[res] + 20)
        resPrice[res] = avg + (tradePrices[res].size() * 2);
    }

    if (colony.res[res] == 0)
      resPrice[res] = (int)ceil(resPrice[res] * 1.15);

    tradePrices[res].clear();
  }

  resPrice[FOOD] = max(resPrice[FOOD], 15);
  resPrice[ENERGY] = max(resPrice[ENERGY], 10);
  resPrice[ORE] = max(resPrice[ORE], 45);
  tradePrices[CRYS].clear();

  int r = rand() % 200;
  if (r < 40) r += 40;

  resPrice[CRYS] = r;
}

double randDouble()
{
  return (double)rand() / RAND_MAX;
}

int PopRandom(List<int>& l, Game &g)
{
  if (l.size() == 0) return -1;
  int i = rand() % l.size();
  int ret = l[i];
  l.erase(l.begin() + i);

  if (l.size() == 0 && &l == &g.possibleGoodPlayerEvents) for(int i=0; i<13; i++) l.push_back(i);
  if (l.size() == 0 && &l == &g.possibleBadPlayerEvents) for(int i=13; i<13+9; i++) l.push_back(i); 

  return ret;
}

string Replace(std::string& str, const std::string& from, const std::string& to) 
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return str;
    str.replace(start_pos, from.length(), to);
    return str;
}

int Game::NumLots(Player& p, int r)
{
  int n = 0;
  for (auto i : landlots)
    if (i.second.owner == p.color && i.second.res == r) n++;
  return n;
}

void Game::SendPlayerEvents()
{
  for (Player &p : players)
  {
    string shortMsg = "?", longMsg = "?";
    string lotKey;
    bool    addLot = false;
    int m = 0;
    p.plEvent = -1;
    double goodEvtProb = (double)p.rank/(players.size() + 2);
    double badEventProb = (double)(players.size()-p.rank)/(players.size() + 3);
    double r = randDouble();
    if (r >= (1.0 - goodEvtProb))
      p.plEvent = PopRandom(possibleGoodPlayerEvents, *this);
    else if (r < badEventProb)
      p.plEvent = PopRandom(possibleBadPlayerEvents, *this);

    if (p.plEvent > -1)
      longMsg = pe[p.plEvent];

    switch (p.plEvent)
    {
      case 0:
        shortMsg = "+3 F, +2 E";
        p.res[FOOD] += 3;
        p.res[ENERGY] += 2;
        break;
      case 1:
        shortMsg = "+2 Sm";
        p.res[ORE] += 2;
        break;
      case 2:
        m = ((rand()%month)+1)*25;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg,"?", to_string(m));
        break;
      case 3:
        m = ((rand()%month)+1)*25;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 4:
        m = ((rand()%3)+1)*25*NumLots(p, FOOD);
        if (m <= 0) { p.plEvent = -1; continue; }
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "1?", to_string(m/NumLots(p, FOOD)));
        longMsg = Replace(longMsg, "2?", to_string(m));
        break;
      case 5:
        m = ((rand()%2)+1)*25;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 6:
        m = ((int)(month/4)+1)*200;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 7:
        m = ((int)(month/4)+1)*50;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 8:
        m = ((int)(month/3)+1)*50;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 9:
        m = ((rand()%40)+11)*month;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 10:
        m = ((rand()%(15))+11)*month;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 11:
        m = ((int)(month/3)+1)*50;
        shortMsg = "+(₿) " + to_string(m);
        p.money += m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 12: {
        List<LandLotID> l;
        for (auto lli : landlots)
          if (lli.second.owner == N) l.push_back(lli.first);
        if (l.size() == 0) { p.plEvent = -1; continue; }
        auto k = l[rand() % l.size()];
        landlots[k].owner = p.color;
        lotKey = k;
        addLot = true;
        shortMsg = "Grant " + lotKey;
        break; }
      case 13:
        m = (int)(p.res[FOOD]/2);
        if (m == 0) { p.plEvent = -1; continue; }
        shortMsg = "-" + to_string(m) + " Food";
        p.res[FOOD] -= m;
        break;
      case 14:
        m = ((rand()%month)+1)*25;
        if (m >= p.money || month == 1) { p.plEvent = -1; continue; }
        p.money -= m;
        shortMsg = "-(₿) " + to_string(m);
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 15: {
        m = ((int)(month/3)+1)*25;
        longMsg = Replace(longMsg, "1?", to_string(m));
        m *= (NumLots(p, ORE) + NumLots(p, CRYS));
        if (m == 0 || m >= p.money) { p.plEvent = -1; continue; }
        p.money -= m;
        shortMsg = "-(₿) " + to_string(m);
        longMsg = Replace(longMsg, "2?", to_string(m));
        break; }
      case 16: {
        m = ((int)(month/3)+1)*25;
        longMsg = Replace(longMsg, "1?", to_string(m));
        m *= NumLots(p, ENERGY);
        if (m == 0 || m >= p.money) { p.plEvent = -1; continue; }
        p.money -= m;
        shortMsg = "-(₿) " + to_string(m);
        longMsg = Replace(longMsg, "2?", to_string(m));
        break; }
      case 17:
        m = ((rand()%50)+21)*(int)(month/4);
        if (m >= p.money) { p.plEvent = -1; continue; }
        shortMsg = "-(₿) " + to_string(m);
        p.money -= m;
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 18:
        m = ((rand()%40)+11)*month;
        if (m >= p.money) { p.plEvent = -1; continue; }
        p.money -= m;
        shortMsg = "-(₿) " + to_string(m);
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 19:
        m = ((rand()%15)+11)*month;
        if (m >= p.money) { p.plEvent = -1; continue; }
        p.money -= m;
        shortMsg = "-(₿) " + to_string(m);
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 20:
        m = ((int)(month/4)+1)*200;
        if (m >= p.money) { p.plEvent = -1; continue; }
        p.money -= m;
        shortMsg = "-(₿) " + to_string(m);
        longMsg = Replace(longMsg, "?", to_string(m));
        break;
      case 21: {
        List<LandLotID> l;
        for (auto kv : landlots)
          if (kv.second.owner == p.color) l.push_back(kv.first);
        if (l.size() == 0) { p.plEvent = -1; continue; }
        auto k = l[rand()%l.size()];
        landlots[k].owner = p.color;
        lotKey = k;
        addLot = false;
        shortMsg = "Lost " + lotKey;
        break; }
    }

    if (p.plEvent > -1)
    {
      send(PlayerEvent {color: p.color, money: p.money, shortMsg: shortMsg,
        lotKey: lotKey, addLot: addLot });
      p.send(PEventText { longMsg:longMsg, beneficial:p.plEvent < 13});
    }
    else
    {
      send(PlayerEvent {color: p.color, money: p.money, shortMsg: "",
        lotKey: "", addLot: false });
      p.send(PEventText { longMsg:"", beneficial:true});
    }
    
  }
}

bool Game::GetNextBuyerAndSeller(Player **buyer, Player **seller)
{
  Player *highBuy = nullptr;
  Player *lowSell = nullptr;

  for (Player &p : players)
  {
    if (p.ws == nullptr) continue;
    if (p.buying)
    {
      if (p.currentBid == BUY) continue;
      if (highBuy == nullptr) highBuy = &p;
      else if (p.currentBid > highBuy->currentBid)
        highBuy = &p;
      else if (p.currentBid == highBuy->currentBid && p.rank < highBuy->rank)
         highBuy = &p;
    }
    else // p is selling
    {
      if (p.currentBid == SELL) continue;
      if (lowSell == nullptr) lowSell = &p;
      else if (p.currentBid < lowSell->currentBid)
        lowSell = &p;
      else if (p.currentBid == lowSell->currentBid && p.rank < lowSell->rank)
         lowSell = &p;
    }
  }

  if (auctionType != LAND)
  {
    if (lowSell != nullptr && highBuy == nullptr &&
        lowSell->currentBid == resPrice[auctionType])
    {
      highBuy = &colony;
      colony.currentBid = lowSell->currentBid;
    }

    if (highBuy != nullptr && lowSell == nullptr &&
        highBuy->currentBid == 35 * minIncr + resPrice[auctionType] &&
        colony.res[auctionType] > 0 &&
        minBid == resPrice[auctionType])
    {
      lowSell = &colony;
      colony.currentBid = highBuy->currentBid;
    }
  }

  *buyer = highBuy;
  *seller = lowSell;

  return highBuy != nullptr && lowSell != nullptr && 
          highBuy->currentBid == lowSell->currentBid;
}

int Game::AuctionID()
{
  return month * 10 + auctionType;
}

void Game::Start()
{
  List<LandLotID> availMoundPlots;
  List<LandLotID> availHCPlots;

  /* init landlots */
  for (int row=-2; row<=2; row++)
  {
    for (int col=-4; col<=4; col++)
    {
      LandLotID k{col, row};
      landlots[k];
      if (col != 0)
        availMoundPlots.push_back(k);
      availHCPlots.push_back(k);
    }
  }

  /* init mounds */
  for (int i=0; i<9; i++)
  {
    int ri = rand() % (availMoundPlots.size());
    LandLotID k = availMoundPlots[ri];
    availMoundPlots.erase(availMoundPlots.begin() + ri);
    landlots[k].mNum = (i % 3) + 1;
    int x = k.e;
    int z = k.n;
    auto& g = landlots[k].mg;
    for (int j=0; j<landlots[k].mNum; j++)
    {
      g.push_back(r(x * 10.0f - 3.0f + rand_0_1()*6.0f));
      g.push_back(r(rand_0_1()*360));
      g.push_back(r(z * 10.0f - 3.0f + rand_0_1()*6.0f));
      g.push_back(r(1 + rand_0_1()/2 - .2f));
      g.push_back(r(1 + rand_0_1()/2 - .2f));
      g.push_back(r(1 + rand_0_1()/2 - .2f));
    }
  }

  /* init crystite */
  for (int i=0; i<3; i++)
  {
    int ri = rand() % availHCPlots.size();
    LandLotID k = availHCPlots[ri];
    availHCPlots.erase(availHCPlots.begin() + ri);
    DistributeCrystitie(k, 3);
  }

  landlots.erase(landlots.find(LandLotID{0,0}));

  colony.g = this;
  colony.color = C;
  colony.rank = 0;
  colony.money = INT_MAX / 2;
  colony.res[FOOD] = 8;
  colony.res[ENERGY] = 8;
  colony.res[ORE] = 8;
  started = true;

  state = SRankings;

landlots[LandLotID(1,1)].owner = R;
landlots[LandLotID(2,1)].owner = R;

  StartNextMonth(true);  // set param to true when running game normally

  /* This is how to go directly to auction for testing 
  
  (change StartNextMonth param to false ^)
*/
  // test setup:
  
/*

  for (Player& p : players)
  {
    p.res[ORE] = 3;
  }

  AdvanceToNextState();
  /**/
}

int Game::Surplus(int resType, Player& p, bool nextMonth)
{
  if (resType == LAND) return 0;

  int amt = p.res[resType];
  int m = month;
  if (nextMonth) m++;

  if (resType == FOOD)
  {
    if (m < 5) amt -= 3;
    else if (m < 9) amt -= 4;
    else if (m < 13) amt -= 5;
  }

  if (resType == ENERGY)
  {
    for (auto &l : landlots)
    {
      if (l.second.owner == p.color && l.second.res > NONE && l.second.res != ENERGY)
        amt--;
    }
    if (nextMonth)
      amt--;
  }

  return amt;
}

void Game::PreAuction()
{
  AuctionData ad;
  List<AuctionStartPlayerData> pdl;

  for (Player& p : players)
  {
    if (p.ws == nullptr) continue;
    AuctionStartPlayerData pd;
    pd.c = p.color;
    pd.current = p.res[auctionType];
    pd.money = p.money;

    if (auctionType == LAND)
      pd.buying = true;
    else
    {
      pd.surplus = Surplus(auctionType, p, true);
      pd.buying = pd.surplus <= 0;
      pd.produced = p.produced[auctionType];
      pd.spoiled = p.spoiled[auctionType];
      pd.used = p.used[auctionType];
    }

    pdl.push_back(pd);

    p.buying = pd.buying;
    p.currentBid = (p.buying ? BUY : SELL);
  }

  ad.auctionType = auctionType;
  ad.month = month;

  if (auctionType == LAND)
  {
    ad.colonyBuyPrice = 500;
    ad.colonyNumUnits = 0;
  }
  else
  {
    ad.colonyBuyPrice = resPrice[auctionType];
    ad.colonyNumUnits = colony.res[auctionType];
  }
  ad.playerData = pdl;
  send(ad);
}

void Game::StartAuction()
{
  minIncr = auctionType >= CRYS ? 4 : 1;
  minBid = resPrice[auctionType];
  void TimerThread(Game *game_ptr, int auctionID);
  auctionTimerThread = thread(TimerThread, this, AuctionID());
  auctionTimerThread.detach();
}

int NumRLots(map<LandLotID,
      LandLot>    &landlots)
{
  int count = 0;
  for (auto i:landlots)
  {
    if (i.second.owner == R) count++;
  }
  return count;
}

void Game::EndAuction(int auctionID)
{
  if (AuctionID() != auctionID)
    return;

  if (auctionType == LAND)
  {
    if (!currentAuctionLot.IsCenter())
    {
      Player *buyer, *unused;
      GetNextBuyerAndSeller(&buyer, &unused);
      if (buyer != nullptr)
      {
        buyer->money -= buyer->currentBid;
        landlots[currentAuctionLot].owner = buyer->color;
        landlotdata.clear();
        for (auto i : landlots)
          landlotdata.push_back({i.first.e, i.first.n, i.second});
        send(GameData{gd:*this});
      }
      currentAuctionLot.SetCenter();
    }
  }

  state = SAuctionOver;
  send(AdvanceState{newState:SAuctionOver});
  std::this_thread::sleep_for(std::chrono::seconds(3));
  AdvanceToNextState();
}

void Game::EndExistingTrade()
{
  tradeConfirmID++;
  activeTradingPrice = 0;
  unitsTraded = 0;
  tradingBuyer = nullptr;
  tradingSeller = nullptr;
  tradeCond.notify_all();  // kill tradeThread

  send(EndTradeMsg{});
}

bool Game::StartNewTrade(Player *buyer, Player *seller)
{
  if (buyer->currentBid != seller->currentBid)
    return false;

  tradeConfirmID++;
  activeTradingPrice = buyer->currentBid;
  tradingBuyer = buyer;
  tradingSeller = seller;
  
  void StartNewTrade(Game *g, int tradeConfirmID); 
  tradeThread = std::thread(StartNewTrade, this, (int)tradeConfirmID);
  tradeThread.detach();

  send(StartTradeMsg{buyer:buyer->color, seller:seller->color});

  return true;
}

void Game::StartTradeConfirmation(int confirmID)
{
  if (tradeConfirmID == confirmID && activeTradingPrice > 0 && 
      tradingBuyer != nullptr && tradingSeller != nullptr)
  {
    ConfirmTrade confirm{tradeConfirmID:confirmID, price:activeTradingPrice};

    if (tradingBuyer->color != C)
      tradingBuyer->send(confirm);
  
    if (tradingSeller->color != C)
      tradingSeller->send(confirm);
   
    void CancelIfConfirmNotReceived(Game *g, int tradeConfirmID);
    tradeThread = std::thread(CancelIfConfirmNotReceived, this, (int)tradeConfirmID);
    tradeThread.detach();
  }
}

void SendCurrentAuctionState(Game *g)
{
  UpdateBidReq fake;
  fake.Recv(g->colony, *g);
}

void Game::TradeConfirmed(int confirmID, Player &p)
{
  if (confirmID == tradeConfirmID)
  {
    p.confirmed = true;
    if (tradingBuyer->confirmed && tradingSeller->confirmed)
    {
      tradeConfirmID++;
      tradeCond.notify_all();  // kill tradeThread waiting for timeout

      // ok actually do trade here and send msg

      unitsTraded++;
      tradingBuyer->money -= activeTradingPrice;
      tradingSeller->money += activeTradingPrice;
      tradingBuyer->res[auctionType]++;
      tradingSeller->res[auctionType]--;

      UnitsTraded ut;
      ut.unitsTraded = unitsTraded;
      ut.buyer = tradingBuyer->color;
      ut.seller = tradingSeller->color;
      ut.newBuyerMoney = tradingBuyer->money;
      ut.newBuyerUnits = tradingBuyer->res[auctionType];
      ut.newSellerMoney = tradingSeller->money;
      ut.newSellerUnits = tradingSeller->res[auctionType];
      ut.buyersurplus = Surplus(auctionType, *tradingBuyer);
      ut.sellersurplus = Surplus(auctionType, *tradingSeller);
  
      send(ut);

      if (tradingBuyer->money < activeTradingPrice)
      {
        tradingBuyer->currentBid = BUY;
        send(EndTradeMsg{reason:NSF,player:tradingBuyer->color});
        tradingBuyer = nullptr;
      }

      if (tradingSeller->res[auctionType] == 0)
      {
        tradingSeller->currentBid = SELL;
        send(EndTradeMsg{reason:SOLDOUT,player:tradingSeller->color});
        tradingSeller = nullptr;
      }

      else if (Surplus(auctionType, *tradingSeller) == 0)
      {
        tradingSeller->currentBid = SELL;
        send(EndTradeMsg{reason:CRITICAL,player:tradingSeller->color});
        tradingSeller = nullptr;
      }

      if (tradingBuyer == nullptr || tradingSeller == nullptr)
      {
        unitsTraded = 0;
        if (!GetNextBuyerAndSeller(&tradingBuyer, &tradingSeller))
        {
          EndExistingTrade();
          SendCurrentAuctionState(this);
          return;
        }
      }

      StartNewTrade(tradingBuyer, tradingSeller);
    }
  }
}

void Game::TradeConfirmNotReceived(int confirmID)
{
  if (confirmID == tradeConfirmID)
  {
    Player* buyer = tradingBuyer;
    Player* seller = tradingSeller;

    EndExistingTrade(); // sets tradingBuyer/Seller to null

    if (!buyer->confirmed)
    {
      buyer->currentBid == BUY;
      send(EndTradeMsg{reason:NOCONFIRM,player:buyer->color});
    }
    if (!seller->confirmed)
    {
      seller->currentBid == SELL;
      send(EndTradeMsg{reason:NOCONFIRM,player:seller->color});
    }

    if (GetNextBuyerAndSeller(&buyer, &seller))
      StartNewTrade(buyer, seller);
  }
}

int runningPort = -1;

vector<Game> games;

void Listening(us_listen_socket_t *param)
{
  printf("Socket ptr %p listening on port %d \n", param, runningPort);
}

void NewConnection(WebSock *ws)
{
  string ip(ws->getUserData()->ip);
  LOG("New connection from '%s' (%s)\n", ip.c_str(), ltime());

  // For dev, let's kill the game if new connection after setup
  if (games.size() > 0 && games[0].state != SSetup)
  {
    for (Player &p : games[0].players)
    {
      if (p.ws != nullptr)
        p.ws->close();
    }

    games.clear();
  }

  // we'll only support one game for now
  if (games.size() == 0)
    games.push_back(Game());
  
  games[0].NewConnection(ws, ip);
}

void Recv(WebSock *ws, MsgData msg, OpCode opCode)
{
  const std::size_t intSize = sizeof(int);
  std::size_t msgSize = msg.size();
  if (msgSize < intSize)
  {
    LOG("Received a message that was only %lu bytes\n", msgSize);
    return;
  }

  const char *msgData = msg.data();
  int msgID = *(int*)msgData;
  if (msgID < 0 || msgID >= msgs.size())
  {
    LOG("Received a message with ID %d (must be < %d)\n", msgID, (int)msgs.size());
    return;
  }

  const MsgMapData &e = msgs[msgID];
  if (e.recvFunc == nullptr)
  {
    LOG("Received a message with ID %d '%s' that has no registerred recv\n", msgID, e.className.c_str());
    return;
  }

  LOGMSG(msgID, " -> Recv from %c: %s\n", ColorName(ws->getUserData()->player->color)[0], e.className.c_str());

  if (e.json) {
    LOGJSON(msgData + 4, msgSize - 4); }
  else if (msgID != 38)
    LOGBIN(msgData, msgSize);

  e.recvFunc(ws, msg);
}

void ConnectionClosed(WebSock *ws, int, MsgData)
{
  Player &p = *((ws->getUserData())->player);
  LOG("Connection closed %s (%s)\n", ColorName(p.color), ltime());
  p.g->Disconnect(p);
}

void Upg(HttpResponse<false> *res, HttpRequest *req, struct us_socket_context_t *context)
{
  res->upgrade(
    PlayerRef{nullptr, string(req->getHeader("x-remote-ip"))},
    req->getHeader("sec-websocket-key"),
    req->getHeader("sec-websocket-protocol"),
    req->getHeader("sec-websocket-extensions"),
    context
  );
}

void RunWSServer(int port)
{
  App app;
  runningPort = port;
  app.ws<PlayerRef>("/*", 
   {
    .idleTimeout = 30,    // this in connection with sendPingsAutomatically (= true by default) seems to keep Chrome WS alive when idle/minimized (esp. on Windows)
    .upgrade = Upg,
    .open = NewConnection,
    .message = Recv,
    .close = ConnectionClosed
   }
  );
  appLoop = app.getLoop();
  app.listen(port, Listening);
  app.run();
}
