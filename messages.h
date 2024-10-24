#pragma once

#include "BasicTypes.h"
struct Player;

struct BinBase {};
template<class T> struct BinMsg : BinBase { int _msgID; BinMsg():_msgID(T::msgID) {} };

// REGISTERMSGS only gets defined in messages_reg.cpp.
// The following macros are defined such that globals are generated
// only in that file whose initailizations cause the message types to
// get registerred to the 'msgs' vector.
#ifdef REGISTERMSGS  
 #include "messages_reg.h"
#else
 #define JS_REG(CLASS, id)  void Recv(Player&);
 #define BIN_REG(CLASS, id) void Recv(Player&); static const int msgID; JS_OBJ(_msgID);
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
