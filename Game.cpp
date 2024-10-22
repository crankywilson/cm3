#include "Game.h"
#include <map>

using namespace std;

// since the Player is allocated by the WebSocket,
// if the connection goes bad, we need to allocate a new object
// here and update the reference in players vector
map<string, Player> disconnectedPlayers;

Game::Game()
{
}

bool HandleReconnect(Game &g, Player &p, PlayerWS* ws, const string &ip)
{
  if (disconnectedPlayers.find(ip) != disconnectedPlayers.end())
  {
    // do reconnect stuff, send msgs
    disconnectedPlayers.erase(ip);
    return true;
  }

  return false;
}

void Game::Disconnect(Player &p)
{
  p.ws = nullptr;
  Player &newPlayerRef = disconnectedPlayers[p.ip];

  // copy player data to new ref in map
  newPlayerRef = p;

  // now update Game player pointer, since the existing
  // one belonged to the websocket which is going away...
  for (int i=0; i<4; i++)
  {
    if (players[i]->color == p.color)
    {
      players[i] = &newPlayerRef;
      break;
    }
  }
}

void Game::NewConnection(Player &p, PlayerWS* ws, const string &ip)
{
  if (HandleReconnect(*this, p, ws, ip))
    return;

  if (state != SSetup)
  {
    printf("Can't handle unknown connection to active game\n");
    return;
  }

  if (players.size() >= 4)
  {
    printf("Can't handle connection to game with >= 4 players\n");
    return;
  }

  p.starter = players.size() == 0;
  p.ip = ip;
  p.ws = ws;
  for (enum Color c=R; c<=B; c=(Color)((int)c+1))
  {
    p.color = c;
    if (player(c) == nullptr) break;
  }

  players.push_back(&p);
}

void Game::Start()
{
}