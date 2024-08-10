#include "memory/heap.hpp"
#include <memory/file.hpp>

namespace memory {
uint8_t* MemoryReader::read(uint64_t offs, uint64_t len) {
    // TODO: throw here
    if (len+offs > _size) return nullptr;
    uint8_t* res = (uint8_t*)kmalloc(len);
    memcpy(res, addr+offs, len);
    return res;
}
uint64_t MemoryReader::size() {
    return _size;
}
MemoryReader::~MemoryReader() {}
MemoryReader::MemoryReader(uint8_t* _addr, uint64_t __size) : addr{_addr}, _size{__size} {}


MemoryWriter::~MemoryWriter() {}
MemoryWriter::MemoryWriter(uint8_t* _addr, uint64_t _size) : addr{_addr}, size{_size} {}


MemoryFile::MemoryFile(uint8_t* _addr, uint64_t __size, bool __readable, bool __writeable) : 
    addr{_addr}, _size{__size}, _readable{__readable}, _writeable{__writeable} {}

uint64_t MemoryFile::size() {return _size;}

bool MemoryFile::readable() {return _readable;}
bool MemoryFile::writeable() {return _writeable;}

MemoryReader* MemoryFile::getReader() {
    if (_readable) return new MemoryReader(addr, _size);
    return nullptr;
}


MemoryWriter* MemoryFile::getWriter() {
    if (_writeable) return new MemoryWriter(addr, _size);
    return nullptr;
}

// TODO: ownership semantics
MemoryFile::~MemoryFile() {}

}
