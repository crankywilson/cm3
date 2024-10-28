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

  printf(" -> Send to   %c: %s\n", ColorName(ws.getUserData()->player)[0], e.className.c_str());

  if (e.json)
  {
    Token token;
    TypeHandler<T>::from(msg, token, jsSendContext.serializer);
    jsSendContext.flush();

    string msgWithID = "    " + jsSendContextBacking;
    *(int*)(&(msgWithID[0])) = msgID;

    (void) ws.send(msgWithID);
  
    printf("%s\n", jsSendContextBacking.c_str());

  }
  else
  {
    ws.send(std::string_view((const char*)&msg, sizeof(T)));
  }
}