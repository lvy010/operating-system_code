#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include <atomic>
#include <memory>
#include <optional>

template<typename T>
class LockFreeQueue {
private:
    struct Node {
        T data;
        std::atomic<Node*> next{nullptr};
        
        Node(const T& value) : data(value) {}
    };
    
    std::atomic<Node*> head{nullptr};
    std::atomic<Node*> tail{nullptr};

public:
    LockFreeQueue() {
        Node* dummy = new Node(T{});
        head.store(dummy);
        tail.store(dummy);
    }
    
    ~LockFreeQueue() {
        Node* current = head.load();
        while (current) {
            Node* next = current->next.load();
            delete current;
            current = next;
        }
    }
    
    void push(const T& value) {
        Node* new_node = new Node(value);
        Node* old_tail = tail.load();
        
        while (true) {
            Node* expected_tail = old_tail;
            if (tail.compare_exchange_weak(expected_tail, new_node)) {
                old_tail->next.store(new_node);
                break;
            }
            old_tail = tail.load();
        }
    }
    
    std::optional<T> pop() {
        Node* old_head = head.load();
        
        while (old_head) {
            Node* old_tail = tail.load();
            Node* next = old_head->next.load();
            
            if (old_head == old_tail) {
                if (!next) {
                    return std::nullopt; // 队列为空
                }
                tail.compare_exchange_weak(old_tail, next);
            } else {
                if (head.compare_exchange_weak(old_head, next)) {
                    T value = next->data;
                    delete old_head;
                    return value;
                }
            }
            old_head = head.load();
        }
        
        return std::nullopt;
    }
    
    bool empty() const {
        return head.load() == tail.load();
    }
};

#endif // LOCK_FREE_QUEUE_H 