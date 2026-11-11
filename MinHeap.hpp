#ifndef MIN_HEAP_HPP
#define MIN_HEAP_HPP

#include <vector>
#include <stdexcept>
#include <utility>

struct HeapNode{
    int id;
    double dist;
};

class MinHeap{
private:
    std::vector<HeapNode> heap;

    void heapifyUp(size_t idx){
        while (idx > 0){
            size_t parent = (idx-1)/2;
            if(heap[idx].dist < heap[parent].dist){
                std::swap(heap[idx], heap[parent]);
                idx = parent;
            }else{
                break;
            }
        }
    }

    void heapifyDown(size_t idx){
        size_t smallest = idx;
        size_t left = 2*idx+1;
        size_t right = 2*idx+2;

        if(left < heap.size() && heap[left].dist < heap[smallest].dist){
            smallest = left;
        }
        if(right < heap.size() && heap[right].dist < heap[smallest].dist){
            smallest = right;
        }

        if (smallest != idx){
            std::swap(heap[idx], heap[idx]);
            heapifyDown(smallest);
        }
    }


public:
    MinHeap() = default;

    bool isEmpty() const {
        return heap.empty();
    }

    void push(int id, double dist){
        heap.push_back({id, dist});
        heapifyUp(heap.size() - 1);
    }

    HeapNode pop(){
        if(heap.empty()){
            throw std::runtime_error("Heap is Empty");
        }
        HeapNode root = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if(!heap.empty()){
            heapifyDown(0);
        }
        return root;
    }



};

#endif