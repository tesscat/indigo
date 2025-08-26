#ifndef KERNEL_MEMORY_FILE_HPP
#define KERNEL_MEMORY_FILE_HPP

#include "interfaces/native/fs.hpp"
#include "interfaces/native/rw.hpp"
namespace memory {
    class MemoryReader : public interfaces::Reader {
        uint8_t* addr;
        uint64_t _size;

    public:
        virtual uint8_t* read(uint64_t offs, uint64_t len) override;
        virtual uint64_t size() override;
        virtual ~MemoryReader() override;

        MemoryReader(uint8_t* addr, uint64_t size);
    };

    class MemoryWriter : public interfaces::Writer {
        // uint8_t* addr;
        // uint64_t size;
    public:
        virtual ~MemoryWriter() override;

        MemoryWriter(uint8_t* addr, uint64_t size);
    };

    class MemoryFile : public interfaces::File {
        uint8_t* addr;
        uint64_t _size;
        bool _readable, _writeable;

    public:
        virtual ~MemoryFile() override;

        virtual bool readable() override;
        virtual MemoryReader* getReader() override;
        virtual bool writeable() override;
        virtual MemoryWriter* getWriter() override;

        virtual uint64_t size() override;

        MemoryFile(
            uint8_t* addr,
            uint64_t size,
            bool readable  = true,
            bool writeable = true
        );
    };
}

#endif // !KERNEL_MEMORY_FILE_HPP
