#include <stdio.h>

#include "messages.h"
#include "Player.h"
#include "Game.h"

int port = 4567;

using namespace std;
vector<Game> games;

void Listening(us_listen_socket_t *param)
{
  printf("Listening on port %d socket ptr %p\n", port, param);
}

typedef std::string_view MsgData;

void NewConnection(PlayerWS *ws)
{
  std::string ip(ws->getRemoteAddress());
  printf("New connection from '%s'\n", ip.c_str());

  // we'll only support one game for now
  if (games.size() == 0)
    games.push_back(Game());
  
  games[0].NewConnection(*(ws->getUserData()), ws, ip);
}

void Recv(PlayerWS *ws, MsgData msg, OpCode opCode)
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
  if (msgID < 0 || msgID >= MAXMSGS)
  {
    printf("Received a message with ID %d (not in 0-%d)\n", msgID, (int)MAXMSGS);
    return;
  }

  const MsgMapEntry &e = (*(msgMap))[msgID];
  if (e.recvFunc == nullptr)
  {
    printf("Received a message with ID %d which is undefined\n", msgID);
    return;
  }

  msgData += intSize;
  msgSize -= intSize;
  e.recvFunc(MsgData(msgData, msgSize), *(ws->getUserData()));
}

void ConnectionClosed(PlayerWS *ws, int, MsgData)
{
  Player &p = *(ws->getUserData());
  printf("Connection closed %d\n", p.color);
  p.g->Disconnect(p);
}

void RunWSServer(int portParam)
{
  port = portParam;
  App app;
  app.ws<Player>("/*", 
   {
    .open = NewConnection,
    .message = Recv,
    .close = ConnectionClosed
   }
  );
  app.listen(port, Listening);
  app.run();
}

std::string msgBuf;

int main(int argc, char* argv[])
{
  printf("params %d %s\n", argc, argv[0]);
  RunWSServer(4567);
  return 0;
}
