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
    Node* node = parse(input);
    nodes.Append(node);
    // require a comma
    skipWhitespace(input);
    if (input[0] != ',') {
      input.error(*input.pos, "Expected a `,` inside an array");
    }
    *input.pos += 1;
    skipWhitespace(input);
  }
  nodes.takeMyData = true;
  nodes.Trim();

  data = nodes.data;
  len = nodes.len;
}
}
