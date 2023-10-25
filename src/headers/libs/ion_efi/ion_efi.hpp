#ifndef LIBS_IONREADER_IONREADER_HPP
#define LIBS_IONREADER_IONREADER_HPP

#include <vec_efi/vec_efi.hpp>

namespace ion {
struct Input {
  size_t* pos;
  const char* str;
  const char& operator [](size_t offs) {return str[*pos + offs];}

  void(error)(size_t loc, const char* message);
};

class Node {
public:
  char* path;
  virtual ~Node() =0;
};
class StrNode : public Node {
private:
  char* data;
public:
  StrNode(Input input);
  char* getStr();
  ~StrNode();
};
class ArrayNode : public Node {
private:
  size_t len;
  Node** data;
public:
  ArrayNode(Input input);
  indigo::Vec<Node*>* getArr();
  ~ArrayNode();
};
class ObjectNode : public Node {
private:
  // TODO: replace with hashmap when I have a hashmap
  char** names;
  Node* data;
public:
  ObjectNode(Input input);
  Node* getNode(const char* name);
  char** getNames();
};

Node* parse(Input input);
}

#endif
