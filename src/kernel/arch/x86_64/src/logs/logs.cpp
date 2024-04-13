#include "graphics/psf.hpp"
#include "graphics/screen.hpp"
#include "memory/heap.hpp"
#include "multi/cpu.hpp"
#include <logs/logs.hpp>

namespace logs {
Logger info;

Logger::Logger() {
    init();
}

void initLogs() {
    info.init();
}

util::Vec<char>& Logger::currBuff() {
    uint64_t idx = multi::getCpuIdx();
    return buff[idx];
}
void Logger::write(String& text) {
    for (uint64_t i = 0; i < text.len; i++) {
        currBuff().Append(text[i]);
        if (text[i] == '\n') flush();
    }
}
void Logger::write(const char* text) {
    const char* t = text;
    while(*t) {
        currBuff().Append(*t);
        if (*t == '\n') flush();
        t++;
    }
}
void Logger::write(const char c) {
    currBuff().Append(c);
    if (c == '\n') flush();
}
void Logger::flush() {
    graphics::psf::consoleLock.lock();
    currBuff().Append('\0');
    graphics::psf::print(currBuff().data);
    currBuff().len = 0;
    currBuff().ApproxTrim();
    graphics::psf::consoleLock.release();
}
void Logger::init() {
    if (hasInit) return;
    hasInit = true;
    buff.init();
    for (unsigned int i = 0; i < kargs->n_cpus; i++) {
        util::Vec<char> p = util::Vec<char>();
        buff.Append(p);
    }
}
}
