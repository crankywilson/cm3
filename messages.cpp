#include "messages.h"

#include "Game.h"
#include <vector>



void Msg1::Recv(Player &p)
{
}

void Msg2::Recv(Player &p)
{
}





/*
struct Msg3
{
  int a;
  int b;

  void Recv(Player &p);

  JS_OBJ(a,b);
};

template <class T>
void Recv (WebSock *ws, const MsgData sv)
{
  T msg;
  JS::ParseContext context(sv.data(), sv.size());
  (void) context.parseTo(msg);
  Player p;
  msg.Recv(p);
}

template <class T>
void RecvBin (WebSock *ws, const MsgData sv)
{
   Player p;
  (*(T*)(sv.data())).Recv(p);
}

void Msg3::Recv(Player& p)
{
  printf("a = %d, b = %d\n", a, b);
}


void dotest()
{
  Msg3 m3;
  m3.a=9;
  m3.b=15;
  auto js = JS::serializeStruct(m3);
  printf("%s\n", js.c_str());
  m3.a = 21;
  MsgData md((const char *)&m3, sizeof(Msg3));
  Recv<Msg3>(nullptr, js);
  RecvBin<Msg3>(nullptr, md);
}

*/
