#ifndef TEST_TASK_H
#define TEST_TASK_H

#include "task.h"

#ifndef CONSOLE
#define CONSOLE Serial
#endif

namespace psiiot
{
    
class TestTask : public Task
{
    char id_;
    const int * seq_;
    const int * work_;
    bool run_;
    unsigned switchAt_;
    


    void computeSwitchPoint()
    {
        switchAt_ = ticks_ + *work_;            
    }
    
public:
    static unsigned ticks_;
        
    //--------------------------------------------------------
    TestTask(char id, const int* seq)
    : id_(id), run_(false), seq_(seq), work_(seq)
    {
        computeSwitchPoint();      
    }
    //--------------------------------------------------------
    TaskResult run(ATaskScheduler* sch)
    {
        if(switchAt_==0)
            return TaskResult::NotRun;
        
        TaskResult ret;
        
        if(ticks_ >= switchAt_) 
        {
            bool wasrun = run_;
            run_ = !run_;
            ++work_;
            if(*work_==0)
                work_ = seq_;
            
            computeSwitchPoint();                     
                
            ret = wasrun ? TaskResult::Run : TaskResult::NotRun;
        }
        else
        {
            ret= run_ ? TaskResult::RunContinue : TaskResult::NotRun;
        }
        
        CONSOLE.printf("%c%d ", id_, (int)ret );
        
        return ret;            
    }        
            
};
    
    
}

#endif
