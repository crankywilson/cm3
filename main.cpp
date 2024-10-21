#include <stdio.h>
#include "uWebSockets.h"
int port = 4567;

using namespace uWS;

void Listening(us_listen_socket_t *param)
{
  printf("Listening on port %d socket ptr %p\n", port, param);
}

struct Player
{
};

typedef WebSocket<false, true, Player> PlayerWS;
typedef std::string_view MsgData;

inline Player& GetPlayer(PlayerWS *ws) { return *(ws->getUserData()); }

void NewConnection(PlayerWS *ws)
{
  Player& p = GetPlayer(ws);
  printf("New connection\n");
  ws->send("OK, connected");
}

void Recv(PlayerWS *ws, MsgData msg, OpCode opCode)
{
  printf("Received %d bytes\n", (int)msg.size());
}

void ConnectionClosed(PlayerWS *ws, int, MsgData)
{
  printf("Connection closed\n");
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

int main(int argc, char* argv[])
{
  printf("params %d %s\n", argc, argv[0]);
  RunWSServer(4567);
  return 0;
}
