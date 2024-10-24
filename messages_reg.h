
int Register(int id, RecvFunc fn, const char *className, bool json, int size=-1);


template <class T>
void RecvJSTempl(WebSock *ws, const MsgData sv)
{
  T msg;
  JS::ParseContext context(sv.data(), sv.size());
  (void) context.parseTo(msg);
  msg.Recv(*(ws->getUserData()->player));
}

template <class T>
void RecvBinTempl(WebSock *ws, const MsgData sv)
{
  (*(T*)(sv.data())).Recv(*(ws->getUserData()->player));
}

#define JS_REG(CLASS, id) \
  static const int msgID; \
  void Recv(Player&); \
}; \
const int CLASS::msgID = { Register(id, RecvJSTempl<CLASS>, #CLASS, true)

#define BIN_REG(CLASS, id) \
  void Recv(Player&); \
}; \
const int CLASS::msgID = { Register(id, RecvBinTempl<CLASS>, #CLASS, false, sizeof(CLASS))


