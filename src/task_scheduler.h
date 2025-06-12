/** @file
 *  @brief A simple task scheduler - based on http://bleaklow.com/2010/07/20/a_very_simple_arduino_task_manager.html
 */
#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_

#ifndef ENABLE_TASK_SCHEDULER_TRACE
#define ENABLE_TASK_SCHEDULER_TRACE 0
#endif

#if ENABLE_TASK_SCHEDULER_TRACE
#define TRACE(x) CONSOLE.write(x)
#define TRACEF(x, ...) CONSOLE.printf(x, __VA_ARGS__ )
#else
#define TRACE(x)
#define TRACEF(x, ...)
#endif

#include "task.h"
namespace psiiot
{
    //===================================================================
    template<uint8_t N>
    class TaskList
    {
    protected:
        TaskList()
        {
            memset(tasks_, 0, sizeof tasks_);
        }

        /// managed tasks
        Task *tasks_[N];


    public:
        unsigned numberOfTasks() const {return N; }
        Task* getTask(int n) const { return tasks_[n]; }
        void setTask(int n, Task* t) { tasks_[n] = t; }
        
//        inline void continueFrom(Task** here)
//        {
            // nop
//        }

        static const bool CAN_CONTINUE = false;    
        static const uint8_t TASK_SLOTS = N;    
    };
    //===================================================================
    /*! @brief ORDER trait

        Runs tasks from the top in index order. This provides a simple
        means of task prioritisation.
     */
    template<uint8_t N>
    class FromFirst : public TaskList<N>
    {
        protected:
        Task** getFirst()
        {
            return this->tasks_;
        }

        Task** getNext(Task** t)
        {
            return ++t;
        }

         inline void continueFrom(Task** here)
         {
             // NOP - should never be called
         }
         
    public:
            static const bool CAN_CONTINUE = false;    
    };
    //===================================================================
    //===================================================================
    template<class BASEORDER>
    class Continuable : public BASEORDER
    {
    protected:
        Task** next_;
        
        Task** getFirst()
        {
            if(next_)
            {
                // if we have continuation set, use it and reset
                // we will call continueFrom() again if we must.
                Task** t = next_;
                next_ = NULL;
                return t;
            }
            
            return BASEORDER::getFirst();
        }
                
        inline void continueFrom(Task** here)
        {
            next_ = here;
        }

        public:
            static const bool CAN_CONTINUE = true;        
    };
    //===================================================================
    /*! @brief ORDER trait class

        Runs tasks cyclicly; this therefore ensures that tasks always have a 
        chance to run.
        
        @note this class is inherently continuable
     */
    template<uint8_t N>
    class RoundRobin : public TaskList<N>
    {
        
        Task** next_;
    protected:
            
        RoundRobin()
        : next_(this->tasks_)
        {
                    
        }

        void Increment()
        {
            if(++next_ >= this->tasks_+N)
                next_ = this->tasks_;
        }

        Task** getFirst()
        {
            Task** n = next_;
            Increment();
            return n;
        }

        inline Task** getNext(Task** t)
        {
            return getFirst();
        }

        inline void continueFrom(Task** here)
        {
            next_ = here;
        }
        
    public:
        static const bool CAN_CONTINUE = true;

    };
    //===================================================================
    /**
     * Scheduler that can execute up to 255 other tasks.
     *
     * It is *also* a task itself, so you can  cascade them
     * if you enjoy complexity.
     *
     * Tasks are scheduled in index order, until the condition
     * supplied by the KIND trait is met.
     *
     * Using a trait class allows us to alter the behaviour of the
     * scheduler in a very runtime-efficient manner, in that we only
     * pay for the functionality we actually use.
     *
     * @tparm N         Number of tasks we support
     * @tparam LIMIT    Run limit algorithm; determines how many slots get run before we pass
     *                  control back. One of RunOneTask, RunAllTasks, RunNTasks (etc)
     * @tparam ORDER    Determine task ordering; currently we have `FromFirst` and `RoundRobin`
     */
    template<
        class LIMIT,
        class ORDER
        >
    class TaskScheduler : public ATaskScheduler, public LIMIT, public ORDER
    {
        //----------------------------------------------------
        int taskIndex(Task **tpp)
        {
            return tpp- this->tasks_;
        }

        
    public:
        static const char* toString(TaskResult r)
        {
            switch(r)
            {
                case TaskResult::NotRun:        return "Nr";
                case TaskResult::Run:           return "Rn";
                case TaskResult::RunContinue:   return "Cn";
            }
        }
    
        /**
         * @param intimeLimit No of ms to spend in a slice.
         * @param maxExec Max number of tasks to run in a slice
         */
        TaskScheduler()
        {}

    //bool canRun() { return KIND::canRun(); }
    //----------------------------------------------------
    /**
     * Do the scheduling.
     *
     * Put a call to this in loop() for the main scheduler(s)
     */
    TaskResult run(ATaskScheduler* /*sch*/ ) override
    {
        if(!enabled_ )
            return TaskResult::NotRun;

        LIMIT::beginSlice();
        Task **tpp = ORDER::getFirst();
        TRACEF("SCH begin @%d\n", taskIndex(tpp) );
        TaskResult res = TaskResult::NotRun;
        for (int t = 0; t < ORDER::TASK_SLOTS; t++)
        {
            Task *tp = *tpp;
            if (tp )
            {
                TaskResult tres = tp->run(this);
                if(tres>res)
                    res = tres;
                TRACEF("SCH %d --> %s\n", taskIndex(tpp), toString(res) );
                if(ORDER::CAN_CONTINUE && res== TaskResult::RunContinue)
                {
                    ORDER::continueFrom(tpp);
                    break;
                }                    
                    
                if(LIMIT::doneSlice(tres))
                    break;
            }
            tpp = ORDER::getNext(tpp);
        }

        return res;
    }
    //----------------------------------------------------
    };
    //====================================================
    /*!
        Scheduler designed for shared resources.
        
        - While the task returns TaskResult::DoneContinue it will continue to be scheduled on successive 
            executions.
        - When finished, it should return TaskResult::Run [which will start next cycle] or TaskResult::NotRun [which will continue to look at lower priority tasks this cycle]
        
        Tasks are round-robin scheduled
     */
    template<uint8_t N>
    class RoundRobinSharedScheduler : public TaskScheduler<RunOneTask,RoundRobin<N> >
    {
        
    };
    //====================================================
    /*!
        Scheduler designed for shared resources.
        
        - While the task returns TaskResult::DoneContinue it will continue to be scheduled on successive
        executions.
        
        - When finished, it should return TaskResult::Run [which will start from the top] or TaskResult::NotRun [which will continue to look at lower priority tasks]
        
        Tasks are scheduled "from the top" 
     */
    template<uint8_t N>
    class FromFirstSharedScheduler : public TaskScheduler<RunOneTask, Continuable<FromFirst<N> > >
    {
        
    };
    //====================================================
    /*!
        Simplest scheduler that just tries to run everything
     */
    template<uint8_t N>
    class TryAllScheduler : public TaskScheduler<RunAllTasks, FromFirst<N> >
    {
        
    };    
    //====================================================
}

#endif