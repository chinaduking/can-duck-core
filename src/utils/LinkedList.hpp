//
// Created by sdong on 2019/11/18.
//

#ifndef LIBFCN_LINKEDLIST_HPP
#define LIBFCN_LINKEDLIST_HPP

#include <cstdint>

namespace utils{

    template <typename val_T>
    class LinkedList{
    public:

        /* 节点 */
        struct Node{
            val_T val;
            Node* p_next;
            Node* p_prev;
        };


        /* 迭代器方法，支持range-based for loop */
        class iterator{
        public:
            iterator(Node* ptr) : ptr(ptr){}
            ~iterator() = default;

            iterator operator++() { ptr = ptr->p_next; return *this; }
            bool operator!=(const iterator & other) const { return ptr != other.ptr; }
            const val_T& operator*() const { return ptr->val; }
        private:
            Node* ptr;
        };

        iterator begin() const { return iterator(head_node); }
        iterator end() const { return iterator(nullptr); }

        /* 构造方法 */
        LinkedList(){
            head_node = nullptr;
            tail_node = nullptr;
            size_ = 0;
        }

        ~LinkedList(){
            Node* i_node = head();
            if(i_node == nullptr){
                return;
            }

            while(i_node->p_next != nullptr){
                Node* p_node = i_node;
                i_node = i_node->p_next;
                delete p_node;
            }
        }
        
        Node* head(){ return head_node; }
        Node* tail(){ return tail_node; }

        uint64_t size(){
            return size_;
        }

        bool empty(){
            return size_ == 0;
        }


        void append(val_T& val){
            Node* p_node;
            size_ ++;

            if(head_node == nullptr){
                head_node = new Node();
                p_node = head_node;
                p_node->val = val;
                p_node->p_next = nullptr;
                p_node->p_prev = nullptr;

                tail_node = p_node;
                return;
            }

            p_node = new Node();
            tail_node->p_next = p_node;
            p_node->val = val;
            p_node->p_next = nullptr;
            p_node->p_prev = tail_node;
            tail_node = p_node;
        }

        Node* find(val_T& val){
            Node* i_node = head_node;
            if(i_node == nullptr){
                return nullptr;
            }

            while(i_node->p_next != nullptr){
                if(i_node->val == val){
                    return i_node;
                }

                i_node = i_node->p_next;
            }

            return nullptr;
        }

        void remove(val_T val){
            Node* i_node = head_node;
            if(i_node == nullptr){
                return;
            }

            while(i_node != nullptr){
                if(i_node->val == val){
                    if(i_node == head_node){
                        head_node = head_node->p_next;
                        head_node->p_prev = nullptr;
                    } else if (i_node == tail_node){
                        tail_node = tail_node->p_prev;
                        tail_node->p_next = nullptr;
                    } else{
                        i_node->p_next->p_prev = i_node->p_prev;
                        i_node->p_prev->p_next = i_node->p_next;
                    }

                    delete i_node;
                    size_ --;
                    break;
                }

                i_node = i_node->p_next;
            }
        }

        void removeIf(bool (*matched)(val_T& val)){
            Node* i_node = head_node;
            if(i_node == nullptr){
                return;
            }

            while(i_node != nullptr){
                if(*matched(i_node->val)){
                    if(i_node == head_node){
                        head_node = head_node->p_next;
                        head_node->p_prev = nullptr;
                    } else if (i_node == tail_node){
                        tail_node = tail_node->p_prev;
                        tail_node->p_next = nullptr;
                    } else{
                        i_node->p_next->p_prev = i_node->p_prev;
                        i_node->p_prev->p_next = i_node->p_next;
                    }

                    delete i_node;
                    size_ --;
                    break;
                }

                i_node = i_node->p_next;
            }
        }

        void popHead(){
            if(head_node == nullptr){
                return;
            }

            if(size_ == 1){
                delete head_node;
                head_node = nullptr;
                tail_node = nullptr;
                return;
            }

            Node* i_node = head_node;
            head_node = head_node->p_next;
            head_node->p_prev = nullptr;
            delete i_node;
            size_ --;
        }

    private:
        Node* head_node;
        Node* tail_node;
        uint64_t size_;
    };

}

#endif //LIBFCN_LINKEDLIST_HPP
