#include <vec_efi/vec_efi.hpp>
#define _STDINT_H
#include <uefi.h>
#include <ion_efi/parse.hpp>
#include <ion_efi/ion_efi.hpp>

namespace ion {
ObjectNode::ObjectNode() {}
ObjectNode::ObjectNode(Input input) {
  // assume input[0] is a {
  *input.pos += 1;

  indigo::Vec<char*> keys;
  indigo::Vec<Node*> nodes;

  skipWhitespace(input);
  while (input[0] != '}' && input[0] != '\0') {
    char* key = takeName(input);
    skipWhitespace(input);
    if (input[0] != '=') {
      input.is_err = true;
      input.error(*input.pos, "Expected '=' in a key-value pairing");
    }
    *input.pos += 1;
    skipWhitespace(input);
    Node* value = parseValue(input);
    keys.Append(key);
    nodes.Append(value);
    skipWhitespace(input);
  }

  if (input[0] != '}') {
    input.is_err = true;
    input.error(*input.pos, "Expected a } to finish an object");
  } else {
    *input.pos += 1;
  }
  
  len = keys.len;
  keys.Trim();
  keys.takeMyData = true;
  nodes.Trim();
  nodes.takeMyData = true;
  names = keys.data;
  data = nodes.data;
}

Node* ObjectNode::getNode(const char* name) {
  for (int i = 0; i < len; i++) {
    if (strcmp(name, names[i]) == 0) {
      return data[i];
    }
  }
  return nullptr;
}

char** ObjectNode::getNames() {
  return names;
}

size_t ObjectNode::getLen() {
  return len;
}

ObjectNode::~ObjectNode() {
  for (int i = 0; i < len; i++) {
    delete names[i];
    delete data[i];
  }
  delete names;
  delete data;
}

Node* ObjectNode::get(indigo::Vec<const char*>* path, size_t offs) {
  if (offs > path->len) return nullptr;
  if (offs == path->len) return this;
  return getNode(path->data[offs])->get(path, offs + 1);
}
}
