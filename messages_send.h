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
  if (e.className == "")
  {
    LOG("Trying to send unregisterred message ID %d\n", msgID);
    return;
  }

  LOGMSG(msgID, " <- Send to   %c: %s\n", ColorName(ws.getUserData()->player)[0], e.className.c_str());

  if (e.json)
  {
    Token token;
    jsSendContextBacking.resize(4096);
    JS::SerializerContext jsSendContext(jsSendContextBacking);
    jsSendContext.serializer.setOptions(JS::SerializerOptions::Compact);
    jsSendContext.serializer.write((char*)&msgID, sizeof(int));
    TypeHandler<T>::from(msg, token, jsSendContext.serializer);
    jsSendContext.flush();

    const char *msgPtr = jsSendContextBacking.c_str();
    const char *json = msgPtr + sizeof(int);

    string_view msg(msgPtr, strlen(json) + sizeof(int));
    (void) ws.send(msg);
  
    LOGJSON(json, msg.size()-sizeof(int));
  }
  else
  {
    ws.send(std::string_view((const char*)&msg, sizeof(T)));

  if (msgID == CurrentAuctionState::msgID && LOG_LEVEL > LOG_NOTHING)
  {
    CurrentAuctionState& cas = *(CurrentAuctionState*)&msg;
    printf("R: %d, Y: %d, lowask: %d, highbid: %d\n", 
      cas.R, cas.Y, cas.lowestAsk, cas.highestBid);
  }
  else if (msgID == ModelPos::msgID || msgID == ModelRot::msgID) {}
  else
    LOGBIN(((const char*)&msg), sizeof(T));
  }
}