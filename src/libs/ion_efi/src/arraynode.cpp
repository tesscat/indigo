#include <vec_efi/vec_efi.hpp>
#include <ion_efi/ion_efi.hpp>
#include <ion_efi/parse.hpp>
#include <uefi.h>

namespace ion {
ArrayNode::ArrayNode(Input input) {
  // assume input[0] is '['
  *input.pos += 1;
  skipWhitespace(input);

  indigo::Vec<Node*> nodes;

  while (input[0] != ']' && input[0] != '\0') {
    Node* node = parseValue(input);
    nodes.Append(node);
    skipWhitespace(input);
    if (input[0] == ']')
      break;
    // require a comma
    if (input[0] != ',') {
      printf("char: `%c`\n", input[0]);
      input.error(*input.pos, "Expected a `,` inside an array");
    }
    *input.pos += 1;
    skipWhitespace(input);
  }
  if (input[0] != ']') {
    input.is_err = true;
    input.error(*input.pos, "Expected ] to finish an array");
  } else {
    *input.pos += 1;
  }
  nodes.takeMyData = true;
  nodes.Trim();

  data = nodes.data;
  len = nodes.len;
}

Node** ArrayNode::getArr() {
  return data;
}
size_t ArrayNode::getLen() {
  return len;
}

ArrayNode::~ArrayNode() {
  for (int i = 0; i < len; i++) {
    delete data[i];
  }

  delete data;
}

Node* ArrayNode::get(indigo::Vec<const char*>* path, size_t offs) {
  if (offs > path->len) return nullptr;
  if (offs == path->len) return this;
  uint64_t i = strtol(path->data[offs], nullptr, 16);
  if (i >= len) return nullptr;
  return data[i]->get(path, offs + 1);
}
}
