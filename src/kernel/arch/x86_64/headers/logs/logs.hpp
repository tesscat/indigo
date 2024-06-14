#ifndef KERNEL_LOGS_LOG_HPP
#define KERNEL_LOGS_LOG_HPP

#include "io/iostream.hpp"
#include "libstd/string.hpp"
#include "modules/export.hpp"
#include "util/vec.hpp"

namespace logs {
class Logger : public io::sstream {
    friend void initLogs();
    util::ManualVec<util::ManualVec<char>*> buff;
    util::ManualVec<char>* currBuff();
    bool hasInit;
    void init();
public:
    Logger();
    virtual void write(String& text) override;
    virtual void write(const char* text) override;
    virtual void write(const char c) override;
    void flush();
};
void initLogs();
extern logs::Logger info;
EXPORT void writeToInfo(const char* text);
}

#endif // !KERNEL_LOGS_LOG_HPP
