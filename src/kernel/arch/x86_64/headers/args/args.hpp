#ifndef KERNEL_ARGS_ARGS_HPP
#define KERNEL_ARGS_ARGS_HPP

namespace args {
enum ArgsNodeTypes {
    String,
    Int,
    Map,
    Array,
    Bool
};
struct ArgsNode {
    ArgsNodeTypes type;
    void* data;
};
class Args {
public:
    void init();
    // TODO: suitable strings
    void fromCommandLine();
    
    ArgsNode getAsTypeOrDefault(const char* path, ArgsNodeTypes type, ArgsNode def);
};
}

#endif // !KERNEL_ARGS_ARGS_HPP
