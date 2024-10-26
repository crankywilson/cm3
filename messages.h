#pragma once

#include "BasicTypes.h"
struct Player;

template<class T> struct BinMsg { int _msgID; BinMsg():_msgID(T::msgID) {} };
template<class T> struct JSMsg {};
struct CliToServ {};
struct ServToCli {};
struct BiDir     {};

inline void NR(int msgID)
{
  printf("Got unexpected msg ID #%d which has no Recv implemented", msgID);
}

// REGISTERMSGS only gets defined in messages_reg.cpp.
// The following macros are defined such that globals are generated
// only in that file whose initailizations cause the message types to
// get registerred to the 'msgs' vector.
#ifdef REGISTERMSGS  
 #include "messages_reg.h"
#else
 #define JS_REG(CLASS, id)  void Recv(Player&);
 #define BIN_REG(CLASS, id) void Recv(Player&); static const int msgID; JS_OBJ(_msgID);
 #define JS_REG_NORECV(CLASS, id)  void Recv(Player&) {}
 #define BIN_REG_NORECV(CLASS, id) void Recv(Player&) {} static const int msgID; JS_OBJ(_msgID);
#endif

#define List vector

class Msg1 : JSMsg<Msg1>, BiDir
{ 
  public: string    name;
  public: string    address;
  public: List<int> ages;

 JS_OBJ(name, address, ages);
 JS_REG(Msg1, 1)
};

struct Msg4 : BinMsg<Msg4>, BiDir               
{
  int ifield;
  float x;
  float z;

 BIN_REG(Msg4, 4)
}; 
