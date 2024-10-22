#define REGISTERMSGS 1
#include "messages.h"
#include "Player.h"
#include <vector>

MsgMapVec *msgMap;  // ( can't rely on compiler order of init, so init in Register() )

const int Register(const int id, MsgHandler mh, const char *className, bool json)
{
  static MsgMapVec map(MAXMSGS);

  msgMap = &map;

  if (id < 0 || id >= MAXMSGS)
  {
    printf("ID %d is out of range for type '%s'\n", id, className);
    exit(1);
  }

  if (map[id].className != "")
  {
    printf("ID %d is cannot be used for type '%s' as it is used for '%s'\n", id, className, map[id].className.c_str());
    exit(1);
  }

  map[id].className = className;
  map[id].recvFunc = mh;
  map[id].json = json;

  printf("Registerred %s as %d\n", className, id);
  return id;
}

void Msg1::Recv(MsgData d, Player &p)
{
  Msg1 msg;
  deserializeJS(msg, d);
  
  if (msg.name == "Brian")
    printf("Hi\n");
}

void Msg2::Recv(MsgData d, Player &p)
{
}
