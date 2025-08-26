#include <ion_efi/ion_efi.hpp>
#include <ion_efi/parse.hpp>
#include <uefi.h>

namespace ion {
    Node* parseValue(Input input) {
        skipWhitespace(input);
        if (input[0] == '{') {
            return new class ObjectNode(input);
        } else if (input[0] == '[') {
            return new class ArrayNode(input);
        } else if (input[0] == '"') {
            return new class StrNode(input);
        } else {
            input.is_err = true;
            printf("char: `%c`\n", input[0]);
            input.error(*input.pos, "Unexpected character");
            return nullptr;
        }
    }

    class ObjectNode* parse(Input input) {
        class ObjectNode* node = new class ObjectNode();

        indigo::Vec<char*> keys;
        indigo::Vec<Node*> nodes;

        skipWhitespace(input);
        while (input[0] != '\0') {
            // TODO: deduplicate across ObjectNode ctor
            char* key = takeName(input);
            skipWhitespace(input);
            if (input[0] != '=') {
                // TODO: fix
                if (input[0] == '\0') break;
                input.is_err = true;
                printf("got `%c`\n", input[0]);
                input.error(
                    *input.pos, "Expected '=' in a TL key-value pairing"
                );
            }
            *input.pos += 1;
            skipWhitespace(input);
            Node* value = parseValue(input);
            keys.Append(key);
            nodes.Append(value);
            skipWhitespace(input);
        }

        node->len = keys.len;
        keys.Trim();
        keys.takeMyData = true;
        nodes.Trim();
        nodes.takeMyData = true;
        node->names      = keys.data;
        node->data       = nodes.data;

        return node;
    }
}
