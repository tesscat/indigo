#include "args/args.hpp"
namespace args {
ArgsNode Args::getAsTypeOrDefault(const char* path, ArgsNodeTypes type, ArgsNode def) {
    return def;
}
}
