#pragma once
#include "lcore/base.hpp"
#include "lcore/traits.hpp"
#include <memory>
#include <cstddef>

LCORE_NAMESPACE_BEGIN

template <typename T>
requires DefaultConstructible<T>
class CircularList {
private:
    class __Node {
    public:
        T value;
        __Node* next;
        __Node* prev;

        __Node(): value(), next(nullptr), prev(nullptr){};
        __Node(const T& value): value(value), next(nullptr), prev(nullptr){};
        __Node(T&& value): value(std::move(value)), next(nullptr), prev(nullptr){};
    };
    

    /// @brief Empty node
    /// In this structure, tail is always the empty node, head is the next node of tail
    __Node* emptynode;
    size_t m_size;
public:
    CircularList(): emptynode(new __Node()), m_size(0){
        emptynode->next = emptynode;
        emptynode->prev = emptynode;
    };
    ~CircularList(){
        auto node = emptynode->next;
        while (node != emptynode){
            auto next = node->next;
            delete node;
            node = next;
        }
        delete emptynode;
    }

    void push_back(T&& value){
        auto node = new __Node(std::forward<T>(value));
        node->prev = emptynode->prev;
        node->next = emptynode;
        emptynode->prev->next = node;
        emptynode->prev = node;
        m_size++;
    }

    void push_front(T&& value){
        auto node = new __Node(std::forward<T>(value));
        node->next = emptynode->next;
        node->prev = emptynode;
        emptynode->next->prev = node;
        emptynode->next = node;
        m_size++;
    }

    void pop_back(){
        if (m_size == 0) return;
        auto node = emptynode->prev;
        emptynode->prev = node->prev;
        node->prev->next = emptynode;
        emptynode->prev = node->prev;
        delete node;
        m_size--;
    }

    void pop_front(){
        if (m_size == 0) return;
        auto node = emptynode->next;
        emptynode->next = node->next;
        node->next->prev = emptynode;
        emptynode->next = node->next;
        delete node;
        m_size--;
    }

    T& front(){
        return emptynode->next->value;
    }

    T& back(){
        return emptynode->prev->value;
    }

    size_t size(){
        return m_size;
    }

    bool empty(){
        return m_size == 0;
    }

    class iterator {
        __Node* node;
    public:
        iterator(__Node* node): node(node){};
        T& operator*(){
            return node->value;
        }
        T* operator->(){
            return &node->value;
        }
        iterator& operator++(){
            node = node->next;
            return *this;
        }
        iterator operator++(int){
            auto temp = *this;
            node = node->next;
            return temp;
        }
        iterator& operator--(){
            node = node->prev;
            return *this;
        }
        iterator operator--(int){
            auto temp = *this;
            node = node->prev;
            return temp;
        }
        bool operator==(const iterator& other){
            return node == other.node;
        }
        bool operator!=(const iterator& other){
            return node != other.node;
        }

        friend class CircularList;
    };

    iterator begin(){
        return iterator(emptynode->next);
    }

    iterator end(){
        return iterator(emptynode);
    }

    class ignore_iterator {
        __Node* node;
        __Node* empty;
    public:
        ignore_iterator(__Node* node, __Node* empty): node(node), empty(empty){
            if (node == empty) node = node->next;
        }
        ignore_iterator& operator++(){
            node = node->next;
            if (node == empty) node = node->next;
            return *this;
        }
        ignore_iterator operator++(int){
            auto temp = *this;
            node = node->next;
            if (node == empty) node = node->next;
            return temp;
        }
        ignore_iterator& operator--(){
            node = node->prev;
            if (node == empty) node = node->prev;
            return *this;
        }
        ignore_iterator operator--(int){
            auto temp = *this;
            node = node->prev;
            if (node == empty) node = node->prev;
            return temp;
        }
        bool operator==(const ignore_iterator& other){
            return node == other.node;
        }
        bool operator!=(const ignore_iterator& other){
            return node != other.node;
        }
        T& operator*(){
            return node->value;
        }
        T* operator->(){
            return &node->value;
        }

        friend class CircularList;
    };

    ignore_iterator ignore_begin(){
        return ignore_iterator(emptynode->next, emptynode);
    }

    iterator erase(iterator iter){
        if (m_size == 0) throw;
        auto node = iter.node;
        auto next = node->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
        m_size--;
        return iterator(next);
    }

    ignore_iterator erase(ignore_iterator iter){
        if (m_size == 0) throw;
        auto node = iter.node;
        auto next = node->next;
        node->prev->next = node->next;
        node->next->prev = node->prev;
        delete node;
        m_size--;
        return ignore_iterator(next, emptynode);
    }
};

LCORE_NAMESPACE_END

