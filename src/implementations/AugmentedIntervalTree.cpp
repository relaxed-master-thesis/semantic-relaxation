#include "AugmentedIntervalTree.h"
#include <cstdint>
#include <memory>
namespace bench {
    std::shared_ptr<Interval> AugmentedIntervalTree::insertNode(std::shared_ptr<Interval> tmp, std::shared_ptr<Interval> newNode){
        if(tmp == nullptr){
            tmp = std::make_shared<Interval>(*newNode);
            return tmp;
        }
        if(newNode->end > tmp->max){
            tmp->max = newNode->end;
        }
        if(tmp->compareTo(*newNode) <= 0){
            if(tmp->right == nullptr)
                tmp->right = newNode;
            else
                insertNode(tmp->right, newNode);
        }
        else{
            if(tmp->left == nullptr)
                tmp->left = newNode;
            else
             insertNode(tmp->left, newNode);
        }
        return tmp;
    }

    uint64_t AugmentedIntervalTree::getRank(std::shared_ptr<Interval> tmp, std::shared_ptr<Interval> interval, uint64_t rank){
        if(tmp == nullptr)
            return rank;
        
        //tmp contains interval
        if(tmp->start < interval->start && tmp->end > interval->end){ 
            rank += 1;
        }
        
        //do we need to check the left tree?
        if(tmp->left != nullptr && tmp->left->max >= interval->start){
            rank = this->getRank(tmp->left, interval, rank);
        }
        return this->getRank(tmp->right, interval, rank);
    }

}