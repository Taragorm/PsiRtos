#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <AtomicBlock.h>

namespace psiiot
{

/**
 * Lightweight circular buffer class
 */
template<
        class T, 
        unsigned N, 
        typename ATOMIC = UnsafeBlock
        >
class CircularBuffer
{
    T arr_[N];
    T* head_;
    T* tail_;
    unsigned count_;

    T* advance( T* p) 
    {
        ++p;
        if(p==arr_ + N)
            return arr_;
        return p;
    }

    //----------------------------------------
    inline bool _isEmpty() const
    {
        return count_ == 0;
    }
    //----------------------------------------
    inline bool _isFull() const
    {
        return count_ == N;
    }

public:
    //----------------------------------------
    CircularBuffer()
    {
        head_ = arr_;
        tail_ = arr_;
        count_ = 0;
    }
    //----------------------------------------
    bool isEmpty() const
    {
        ATOMIC block;
        return _isEmpty();
    }
    //----------------------------------------
    bool isFull() const
    {
        ATOMIC block;
        return _isFull();
    }
    //----------------------------------------
    unsigned count() const
    {
        ATOMIC block;
        return count_;
    }
    //----------------------------------------
    unsigned available() const
    {
        ATOMIC block;
        return N-count_;
    }
    //----------------------------------------
    T* head() 
    { 
        ATOMIC block;
        return head_; 
    }
    //----------------------------------------
    T* tail() 
    { 
        ATOMIC block;
        return tail_; 
    }
    //----------------------------------------
    void clear()
    {
        ATOMIC block;        
        head_ = arr_;
        tail_ = arr_;
        count_ = 0;    
    }
    //----------------------------------------
    /**
     * Get a pointer to the next Head element,
     * but don't actually advance the head pointer.
     */
    T* peekHeadElement()
    {
        ATOMIC block;
        
        if(_isFull())
            return NULL;
            
        return head_;
    }
    //----------------------------------------
    /**
     * Advance the head pointer one
     */
    T* advanceHead()
    {
        ATOMIC block;
        if(!_isFull())
        {
            head_ = advance(head_);        
            ++count_;
        }
        return head_;
    }
    //----------------------------------------
    T* peekTailElement()
    {
        ATOMIC block;
        
        if(_isEmpty())
            return NULL;
            
        return tail_;
    }
    //----------------------------------------
    T* advanceTail()
    {
        ATOMIC block;
        if(!_isEmpty())
        {
            tail_ = advance(tail_);
            --count_;
        }
        return tail_;
    }
    //----------------------------------------
    /*!
        Pop tail element to `ret` if present.
        @param ret[out] popped element; unchanged if empty
        @return `true` if popped, `false` if empty
     */
    bool popTailUnsafe(T& ret)
    {
        if(!_isEmpty())
        {
            ret = *tail_;
            tail_ = advance(tail_);
            --count_;
            return true;
        }
        else
            return false;
        
    }
    //----------------------------------------
    bool pushHeadUnsafe(const T& e)
    {
        if(!_isFull())
        {
            *head_ = e;
            head_ = advance(head_);
            ++count_;
            return true;
        }
        else
            return false;        
    }
    //----------------------------------------
    //----------------------------------------
    /*!
        Pop tail element to `ret` if present.
        @param ret[out] popped element; unchanged if empty
        @return `true` if popped, `false` if empty
     */
    inline bool popTail(T& ret)
    {
        ATOMIC block;
        return popTailUnsafe(ret);
    }
    //----------------------------------------
    inline bool pushHead(const T& e)
    {
        ATOMIC block;
        return pushHeadUnsafe(e);
    }
    //----------------------------------------

};
//==============================================================

} //namespace

#endif