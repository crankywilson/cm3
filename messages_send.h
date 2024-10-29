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
    LOG("Trying to send unknown message ID %d (must be < %d)\n", msgID, (int)msgs.size());
    return;
  }

  const MsgMapData &e = msgs[msgID];
  if (e.recvFunc == nullptr)
  {
    LOG("Trying to send unregisterred message ID %d\n", msgID);
    return;
  }

  LOGMSG(msgID)(" <- Send to   %c: %s\n", ColorName(ws.getUserData()->player)[0], e.className.c_str());

  if (e.json)
  {
    Token token;
    jsSendContext.serializer.write((char*)&msgID, sizeof(int));
    TypeHandler<T>::from(msg, token, jsSendContext.serializer);
    jsSendContext.flush();

    (void) ws.send(jsSendContextBacking);
  
    LOGJSON("%s\n", jsSendContextBacking.c_str() + sizeof(int));
  }
  else
  {
    ws.send(std::string_view((const char*)&msg, sizeof(T)));
  }
}