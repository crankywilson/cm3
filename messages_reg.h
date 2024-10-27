
int Register(int id, RecvFunc fn, const char *className, bool json, int size=-1);

Game& GetGame(Player *p);

template <class T>
void RecvJSTempl(WebSock *ws, const MsgData sv)
{
  T msg;
  JS::ParseContext context(sv.data(), sv.size());
  (void) context.parseTo(msg);
  Player* p = ws->getUserData()->player;
  msg.Recv(*p, GetGame(p));
}

template <class T>
void RecvBinTempl(WebSock *ws, const MsgData sv)
{
  // might make sense to check the size here
  if (sv.size() != sizeof(T))
  {
    printf("Received a message of size %d when expecting %d for ID %d\n",
      (int)sv.size(), (int)sizeof(T), T::msgID);
    return;
  }

  Player* p = ws->getUserData()->player;
  (*(T*)(sv.data())).Recv(*p, GetGame(p));
}

#define JS_REG(CLASS, id) \
  static const int msgID; \
  void Recv(Player&, Game&); \
}; \
const int CLASS::msgID = { Register(id, RecvJSTempl<CLASS>, #CLASS, true)

#define BIN_REG(CLASS, id) \
  static const int msgID; \
  void Recv(Player&, Game&); \
}; \
const int CLASS::msgID = { Register(id, RecvBinTempl<CLASS>, #CLASS, false, sizeof(CLASS))

#define JS_REG_NORECV(CLASS, id) \
  static const int msgID; \
  void Recv(Player&, Game&) {} \
}; \
const int CLASS::msgID = { Register(id, RecvJSTempl<CLASS>, #CLASS, true)

#define BIN_REG_NORECV(CLASS, id) \
  static const int msgID; \
  void Recv(Player&, Game&) {} \
}; \
const int CLASS::msgID = { Register(id, RecvBinTempl<CLASS>, #CLASS, false, sizeof(CLASS))


