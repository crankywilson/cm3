#ifndef MESSAGES_H
#define MESSAGE_H

#include <json_struct.h>
using namespace JS;

#include "Player.h"

typedef std::string_view MsgData;
typedef void (*MsgHandler)(MsgData, Player&);

const int Register(const int id, MsgHandler fn, const char *className, bool json);

struct MsgMapEntry
{
  MsgHandler recvFunc;
  std::string className;
  bool json;
};

const std::size_t MAXMSGS = 256;
typedef std::vector<MsgMapEntry> MsgMapVec;
extern MsgMapVec *msgMap;  // ( can't rely on order of init, so init in Register() )

#ifdef REGISTERMSGS  // this only gets instantiated in messages.cpp
#define JS_REG(CLASS, id) \
  static const int rtid; \
  static void Recv(MsgData, Player&); \
}; const int CLASS::rtid = { Register(id, CLASS::Recv, #CLASS, true)

#define BIN_REG(CLASS, id) \
  static const int rtid; \
  static void Recv(MsgData, Player&); \
}; const int CLASS::rtid = { Register(id, CLASS::Recv, #CLASS, false)
#else
#define JS_REG(CLASS, id)  static void Recv(MsgData, Player&);
#define BIN_REG(CLASS, id) static void Recv(MsgData, Player&);
#endif

// convenience funcs

template <typename T>
void serializeJS(const T &from_type, std::string& ret_string)
{
  SerializerContext serializeContext(ret_string);
  serializeContext.serializer.setOptions(SerializerOptions::Compact);
  Token token;
  TypeHandler<T>::from(from_type, token, serializeContext.serializer);
  serializeContext.flush();
}

template <typename T>
Error deserializeJS(T &t, const MsgData sv)
{
  ParseContext context(sv.data(), sv.size());
  return context.parseTo(t);
}

// ok actuals messages now

struct Msg1 
{
 std::string name;
 std::string address;
 std::vector<int> ages;

 JS_OBJ(name, address, ages);
 JS_REG(Msg1, 1)
}; 


struct Msg2 
{
 float x;
 float z;

 BIN_REG(Msg2, 2)
}; 

#endif