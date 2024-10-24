#define REGISTERMSGS 1

#include <uWebSockets.h>
using namespace uWS;

#include "messages.h"

vector<MsgMapData> msgs;

vector<MsgMapData>* preInitMsgs;

int Register(int id, RecvFunc fn, const char *className, bool json, int size)
{
  static bool preInitMsgsAllocated = false;

  if (!preInitMsgsAllocated)
  {
    preInitMsgs = new vector<MsgMapData>;
    preInitMsgs->reserve(256);
    preInitMsgsAllocated = true;
  }

  vector<MsgMapData>& msgs = *preInitMsgs;
  
  if (msgs.size() <= id)
    msgs.resize(id + 1);
  
  MsgMapData &entry = msgs[id];

  if (entry.className != "")
  {
    printf("Couldn't register '%s' as ID %d used by '%s'\n",
      className, id, entry.className.c_str() );
    exit(1);
  }

  entry.className = className;
  entry.json = json;
  entry.msgSize = size;
  entry.recvFunc = fn;

  printf("Registerred '%s' as ID %d\n", className, id);

  return id;
}

void PopulateMsgs()
{
  msgs = *preInitMsgs;
}