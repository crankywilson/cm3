#include "Game.h"
#include "uWebSockets.h"
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

Game::Game()
{
  jsSendContext.serializer.setOptions(SerializerOptions::Compact);
}

void Game::Disconnect(Player &p)
{
  p.ws = nullptr;
  // send disconnect msg to other players?
}

void Game::NewConnection(PlayerRef &pr, WebSock* ws, const string &ip)
{
  if (state == SSetup)
  {
    if (players.size() >= 4)
    {
      printf("Can't handle connection to game with >= 4 players\n");
      return;
    }

    Player p;
    p.starter = players.size() == 0;
    p.ip = ip;
    p.ws = ws;
    p.g = this;
    p.color = N;  // gets assigned in NewPlayerReq::Recv
    players.push_back(p);
    pr.player = &player(p.color);  
    // now the player pointer in WebSock references
    // the same player object in the game players vector

    // send gamestate msg?
  }
  else // active game rejoin?
  {
    for (Player &p : players)
    {
      if (p.ws == nullptr && p.ip == ip)
      {
        p.ws = ws;
        pr.player = &p;
         // send gamestate msg?
        return;
      }
    }

    printf("Can't handle unknown connection to active game\n");
    return;
  }
}

void Game::Start()
{
}

int runningPort = -1;

vector<Game> games;

void Listening(us_listen_socket_t *param)
{
  printf("Socket ptr %p listening on port %d \n", param, runningPort);
}

void NewConnection(WebSock *ws)
{
  string ip(ws->getRemoteAddressAsText());
  printf("New connection from '%s'\n", ip.c_str());

  // we'll only support one game for now
  if (games.size() == 0)
    games.push_back(Game());
  
  games[0].NewConnection(*(ws->getUserData()), ws, ip);
}

using namespace uWS;

void Recv(WebSock *ws, MsgData msg, OpCode opCode)
{
  const std::size_t intSize = sizeof(int);
  std::size_t msgSize = msg.size();
  if (msgSize < intSize)
  {
    printf("Received a message that was only %lu bytes\n", msgSize);
    return;
  }

  const char *msgData = msg.data();
  int msgID = *(int*)msgData;
  if (msgID < 0 || msgID >= msgs.size())
  {
    printf("Received a message with ID %d (must be < %d)\n", msgID, (int)msgs.size());
    return;
  }

  const MsgMapData &e = msgs[msgID];
  if (e.recvFunc == nullptr)
  {
    printf("Received a message with ID %d '%s' that has no registerred recv\n", msgID, e.className.c_str());
    return;
  }

  printf(" -> Recv from %c: %s\n", ColorName(ws->getUserData()->player->color)[0], e.className.c_str());
  ((char*)msgData)[msgSize] = 0;
  printf("%s\n", msgData + 4);
  e.recvFunc(ws, msg);
}

void ConnectionClosed(WebSock *ws, int, MsgData)
{
  Player &p = *((ws->getUserData())->player);
  printf("Connection closed %d\n", p.color);
  p.g->Disconnect(p);
}

void RunWSServer(int port)
{
  App app;
  runningPort = port;
  app.ws<PlayerRef>("/*", 
   {
    .open = NewConnection,
    .message = Recv,
    .close = ConnectionClosed
   }
  );
  app.listen(port, Listening);
  app.run();
}