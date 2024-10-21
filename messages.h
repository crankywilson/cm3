#include <json_struct.h>

typedef std::string_view MsgData;

const int Register(const int id, void(*fn)(MsgData), const char *className, bool json);

#define JS_REG(CLASS, id) \
  static const int rtid; \
  static void Recv(MsgData); \
}; const int CLASS::rtid = { Register(id, CLASS::Recv, #CLASS, true)

#define BIN_REG(CLASS, id) \
  static const int rtid; \
  static void Recv(MsgData); \
}; const int CLASS::rtid = { Register(id, CLASS::Recv, #CLASS, false)


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
