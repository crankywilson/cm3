#pragma once

#include "BasicTypes.h"
struct Player;

#ifdef REGISTERMSGS  // this only gets instantiated in messages_reg.cpp
#include "messages_reg.h"
#else
#define JS_REG(CLASS, id)  void Recv(Player&);
#define BIN_REG(CLASS, id) void Recv(Player&);
#endif

#define BIN_MSGID(CLASS) static const int msgID; const int _msgID = CLASS::msgID

struct Msg1 
{
  string name;
  string address;
  vector<int> ages;

 JS_OBJ(name, address, ages);
 JS_REG(Msg1, 1)
}; 

struct Msg2 
{
 BIN_MSGID(Msg2);  // this has to come first for bin msgs (id field)
  float x;
  float z;
 BIN_REG(Msg2, 2)
}; 
