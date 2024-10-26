#pragma once
#include "BasicTypes.h"
#include "uWebSockets.h"
#include "messages.h"
#include <type_traits>

template<class T>
void Send(WebSock& ws, const T& msg)
{
  using namespace JS;
  int msgID = T::msgID;
  if (msgID < 0 || msgID >= msgs.size())
  {
    printf("Trying to send unknown message ID %d (must be < %d)\n", msgID, (int)msgs.size());
    return;
  }

  const MsgMapData &e = msgs[msgID];
  if (e.recvFunc == nullptr)
  {
    printf("Trying to send unregisterred message ID %d\n", msgID);
    return;
  }

  if (e.json)
  {
    jsSendContextBacking.clear();
    Token token;
    TypeHandler<T>::from(msg, token, jsSendContext.serializer);
    jsSendContext.flush();
    (void) ws.send(
      std::string_view((const char*)&msgID, sizeof(int)), 
      uWS::BINARY, false, false);
    (void) ws.send(jsSendContextBacking);
  }
  else
  {
    ws.send(std::string_view((const char*)&msg, sizeof(T)));
  }
}