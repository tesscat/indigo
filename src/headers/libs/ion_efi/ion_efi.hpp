#ifndef LIBS_IONREADER_IONREADER_HPP
#define LIBS_IONREADER_IONREADER_HPP

#include <stdint.h>
#define _STDINT_H
#include <uefi.h>
#include <vec_efi/vec_efi.hpp>

namespace ion {
    struct Input {
        size_t* pos;
        const char* str;
        const char& operator[](size_t offs) {
            return str[*pos + offs];
        }

        void error(size_t loc, const char* message) {
            printf(message);
            while (1);
        }
        bool is_err = false;
    };

    enum NodeType {
        StrNode    = 0,
        ArrayNode  = 1,
        ObjectNode = 2,
    };

    class Node {
        virtual Node* get(indigo::Vec<const char*>* path, size_t offs) {
            return nullptr;
        };
        friend class StrNode;
        friend class ObjectNode;
        friend class ArrayNode;

    public:
        Node() {};
        Node* getFrom(indigo::Vec<const char*>* path) {
            return get(path, 0);
        };
        virtual inline NodeType getType() {
            return (NodeType)-1;
        };
        virtual ~Node() {};
    };
    class StrNode : public Node {
    private:
        char* data;
        Node* get(indigo::Vec<const char*>* path, size_t offs) override;

    public:
        inline NodeType getType() override {
            return NodeType::StrNode;
        };
        StrNode(Input input);
        char* getStr();
        ~StrNode() override;
    };
    class ArrayNode : public Node {
    private:
        size_t len;
        Node** data;
        Node* get(indigo::Vec<const char*>* path, size_t offs) override;

    public:
        inline NodeType getType() override {
            return NodeType::ArrayNode;
        };
        ArrayNode(Input input);
        Node** getArr();
        size_t getLen();
        ~ArrayNode() override;
    };
    class ObjectNode : public Node {
    private:
        // TODO: replace with hashmap when I have a hashmap
        size_t len;
        char** names;
        Node** data;
        ObjectNode();
        Node* get(indigo::Vec<const char*>* path, size_t offs) override;

        friend ObjectNode* parse(Input input);

    public:
        inline NodeType getType() override {
            return NodeType::ObjectNode;
        };
        ObjectNode(Input input);
        Node* getNode(const char* name);
        char** getNames();
        size_t getLen();
        ~ObjectNode() override;
    };

    using RootNode = class ObjectNode;

    RootNode* parse(Input input);
}

#endif
