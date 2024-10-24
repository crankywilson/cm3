#pragma once

#include "BasicTypes.h"
struct Player;

template<class T> struct BinMsg { int _msgID; BinMsg():_msgID(T::msgID) {} };

// REGISTERMSGS only gets instantiated in messages_reg.cpp
// where global are definied to initialize the messages
#ifdef REGISTERMSGS  
 #include "messages_reg.h"
#else
 #define JS_REG(CLASS, id)  void Recv(Player&);
 #define BIN_REG(CLASS, id) void Recv(Player&); static const int msgID;
#endif


struct Msg1 
{
  string name;
  string address;
  vector<int> ages;

 JS_OBJ(name, address, ages);
 JS_REG(Msg1, 1)
}; 

struct Msg2 : BinMsg<Msg2>
{
  int i2;
  float x;
  float z;

 BIN_REG(Msg2, 2)
}; 
