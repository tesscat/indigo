#include "args/args.hpp"
#include <util/util.hpp>

namespace args {
ArgsNode Args::getAsTypeOrDefault(const char* path, ArgsNodeTypes type, ArgsNode def) {
    UNUSED(path); UNUSED(type);
    return def;
}
}
