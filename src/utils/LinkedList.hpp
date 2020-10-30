//
// Created by sdong on 2019/12/2.
//

#ifndef LIBFCN_SHAREDMQ_HPP
#define LIBFCN_SHAREDMQ_HPP

#include <cstdint>
#include <cstddef>

#include "ObjPool.hpp"

namespace utils{

//#ifdef SYSTYPE_FULL_OS
//    #include <mutex>
//#endif //SYSTYPE_FULL_OS

    template <typename val_T=void*, typename Allocator=DefaultAllocator>
    class LinkedList{
    public:

        /* 节点 */
        struct Node{
            val_T val;
            Node* p_next{nullptr};

            void * operator new(size_t size) noexcept {
                return Allocator::allocate(size);
            }

            void operator delete(void * p) {
                Allocator::deallocate((uint8_t*)p);
            }
        };

        /* 迭代器方法，支持range-based for loop */
        class iterator{
        public:
            iterator(Node* ptr) : ptr(ptr){}
            ~iterator() = default;

            iterator operator++() { ptr = ptr->p_next; return *this; }
            bool operator!=(const iterator & other) const { return ptr != other.ptr; }
            val_T& operator*() const { return ptr->val; }
        private:
            Node* ptr;
        };

        iterator begin() const { return iterator(head_node); }
        iterator end() const { return iterator(nullptr); }

        /* 构造方法 */
        LinkedList() = default;

        ~LinkedList(){
            Node* i_node = head_node;
            if(i_node == nullptr){
                return;
            }

            while(i_node->p_next != nullptr){
                Node* p_node = i_node;
                i_node = i_node->p_next;
                delete p_node;
            }
        }

        val_T head(){
            return head_node->val;
        }

        val_T tail(){
            return tail_node->val;
        }

        uint64_t size(){
            return size_;
        }

        bool empty(){
            return size_ == 0;
        }

#ifdef SYSTYPE_FULL_OS
#if 0
        /* 等待推入数据 */
        void wait(int32_t timeout = -1){
            if(timeout > 0){
                std::unique_lock<std::mutex> updating_lk(update_mutex);
                sched_ctrl_cv.wait_for(
                        updating_lk, std::chrono::microseconds(timeout));
            } else if(timeout < 0){
                std::unique_lock<std::mutex> updating_lk(update_mutex);
                sched_ctrl_cv.wait(updating_lk);
            }
        }

        void notify(){
            sched_ctrl_cv.notify_all();
        }
#endif //0
#endif //SYSTYPE_FULL_OS

        void push(val_T&& val){
            Node* p_node;
            size_ ++;

            if(head_node == nullptr){
                head_node = new Node();
                p_node = head_node;
                p_node->val = val;
                p_node->p_next = nullptr;

                tail_node = head_node;
                return;
            }

            p_node = new Node();
            tail_node->p_next = p_node;
            p_node->val = val;
            p_node->p_next = nullptr;
            tail_node = p_node;
        }

        void push(val_T& val){
            push(std::move(val));
        }

        void pop(){
            if(head_node == nullptr){
                return;
            }

            if(size_ == 1){
                delete head_node;
                head_node = nullptr;
                tail_node = nullptr;

                size_ --;
                return;
            }

            Node* i_node = head_node;
            head_node = head_node->p_next;
            delete i_node;
            size_ --;
        }

        Node* find(bool (*matched)(val_T& val)){
            if(head_node == nullptr){
                return nullptr;
            }

            Node* node_iter = head_node;
            while (node_iter != nullptr) {
                if ((*matched)(node_iter->val)) {
                    return node_iter;
                }

                node_iter = node_iter->p_next;
            }

            return nullptr;
        }

        void remove(Node* node){
            if(head_node == nullptr){
                return;
            }

            Node* node_iter_prev = nullptr;
            Node* node_iter = head_node;

            while (node_iter != nullptr){
                if(node == node_iter){
                    size_ --;

                    if(node_iter_prev != nullptr){
                        /* 如果不是头部，上个节点链接下一个节点 */
                        node_iter_prev->p_next = node_iter->p_next;

                        Node* deleting_node = node_iter;

                        /* 旧指针不动，新指针从下一个节点开始 */
                        node_iter = node_iter_prev->p_next;

                        /* 删除当前节点 */
                        delete deleting_node;
                    }else{
                        /* 如果是头部，头节点后移 */
                        head_node = node_iter->p_next;

                        /* 头节点后移之后为空，说明链表已空 */
                        if(head_node == nullptr){
                            tail_node = nullptr;
                        }

                        delete node_iter;
                        node_iter = head_node;
                    }
                }else{
                    node_iter_prev = node_iter;
                    node_iter = node_iter->p_next;
                }
            }
        }

        void remove_if(bool (*matched)(val_T& val)){
            if(head_node == nullptr){
                return;
            }

            Node* node_iter_prev = nullptr;
            Node* node_iter = head_node;

            while (node_iter != nullptr){
                if((*matched)(node_iter->val)){
                    size_ --;

                    if(node_iter_prev != nullptr){
                        /* 如果不是头部，上个节点链接下一个节点 */
                        node_iter_prev->p_next = node_iter->p_next;

                        Node* deleting_node = node_iter;

                        /* 旧指针不动，新指针从下一个节点开始 */
                        node_iter = node_iter_prev->p_next;

                        /* 删除当前节点 */
                        delete deleting_node;
                    }else{
                        /* 如果是头部，头节点后移 */
                        head_node = node_iter->p_next;

                        /* 头节点后移之后为空，说明链表已空 */
                        if(head_node == nullptr){
                            tail_node = nullptr;
                        }

                        delete node_iter;
                        node_iter = head_node;
                    }
                }else{
                    node_iter_prev = node_iter;
                    node_iter = node_iter->p_next;
                }
            }
        }

    private:
        Node* head_node{nullptr};
        Node* tail_node{nullptr};
        uint64_t size_{0};

//#ifdef SYSTYPE_FULL_OS
//
//        std::condition_variable sched_ctrl_cv;
//        std::mutex update_mutex;
//
//#endif //SYSTYPE_FULL_OS

    };



}
#endif