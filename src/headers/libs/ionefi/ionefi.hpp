#ifndef LIBS_IONREADER_IONREADER_HPP
#define LIBS_IONREADER_IONREADER_HPP

namespace ion {
// class Node {
// public:
//   char* path;
//   virtual ~Node() =0;
// };
class StrNode/*  : public Node */ {
private:
  char* data;
public:
  StrNode(const char* input);
  char* getStr();
  // ~StrNode();
};
// class ArrayNode : public Node {
// private:
//   size_t len;
//   Node* data[];
// public:
//   ArrayNode(const char* input);
//   Node** getArr();
//   ~ArrayNode();
// };
// class ObjectNode : public Node {
// private:
//   // TODO: replace with hashmap when I have a hashmap
//   char** names;
//   Node* data;
// public:
//   ObjectNode(const char* input);
//   Node* getNode(const char* name);
//   char** getNames();
// };
//
// Node* parse(char* input);
}

#endif
