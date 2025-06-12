#ifndef TASK_SCHEDULER_BASE_H
#define TASK_SCHEDULER_BASE_H

namespace psiiot
{
    class ATaskScheduler;
    //---------------------------------------------------------
    enum class TaskResult : uint8_t
    {
        /// Did nothing
        NotRun,
            
        /// Did work, but should continue this
        RunContinue,
            
        /// Did work, don't need to continue
        Run
    };
    //=========================================================
    /**
     * Base task class
     */
    class Task 
    {
    public:
        //---------------------------------------------------------
        virtual TaskResult run(ATaskScheduler* sch) = 0;
    };
    //=========================================================
    /**
     * Task with simple enable/disable function.
     
     * @note The subclass is responsible for exiting if isEnabled() returbs false.
     */
    class EnableableTask : public Task
    {
        protected:
            bool enabled_;

        public:
            EnableableTask(bool en=true) : enabled_(en)
            {}

        bool isEnabled() const { return enabled_; } 
        void setEnabled(bool en) { enabled_ = en; }
        

    };
    //===================================================================
   /**
     * Base Trait class for `TaskScheduler`.
     *
     * This is a mixin class that adds a timer that is reset every time
     * a slice begins, but does not perform any other operations with it.
     *
     * Used as a base for `RunTasksTimed`, and can be mixed in if you
     * want to monitor the execution timing.
     */
    class RunTasksTimerSupport
    {
        protected:
            // Millisecond intimer
            MilliTimer inTimer_;

        public:
            RunTasksTimerSupport()
                : inTimer_(0xffffffff, false)
            {}

            void beginSlice()
            {
                inTimer_.reset();
            }

            bool doneSlice(TaskResult res)
            {
                return false;
            }

            /// millis() when we began the slice
            uint32_t sliceBeginMillis() const { return inTimer_.ticksWhenReset(); }

            bool hasSliceExpired() { return inTimer_.isExpired(); }

            /// How long have we been going?
            inline unsigned long sliceExpired() const {inTimer_.intervalExpired();}

            /// How long have we to go?
            inline unsigned long sliceLeft() const {inTimer_.intervalLeft(); }
                
            static const bool CAN_CONTINUE = false;
                
    };
    //===================================================================
    /**
     * Trait class for `TaskScheduler`.
     *
     * Run everything
     */
    class RunAllTasks : public virtual RunTasksTimerSupport
    {
        public:
            void beginSlice(){}

            bool doneSlice(TaskResult res)
            {
                return false;
            }
            
    };
    //===================================================================
    /**
     * Trait class for `TaskScheduler`.
     *
     * Run first active only
     */
    class RunOneTask : public virtual RunTasksTimerSupport
    {
        public:
            void beginSlice(){}

            bool doneSlice(TaskResult res)
            {
                return res== TaskResult::Run;
            }
            
    };
    //===================================================================
    /**
     * Trait class for `TaskScheduler`.
     *
     * Run a number of tasks
     */
    class RunNTasks : public virtual RunTasksTimerSupport
    {
        protected:
            uint8_t task_count_;
            uint8_t task_limit_;

        public:
            RunNTasks() : task_limit_(255) {}

            void beginSlice()
            {
                task_count_ = task_limit_;
            }

            bool doneSlice(TaskResult res)
            {
                if(res< TaskResult::Run)
                    return false;

                return --task_count_ == 0;
            }

            void setLimit(uint8_t limit)
            {
                task_limit_ = limit;
            }

        uint8_t getMaxExecCount() const { return task_limit_; }

        void setMaxExecCount(uint8_t c)
        {
            task_limit_ = c;
        }
    };
    //===================================================================
    /**
     * Trait class for `TaskScheduler`.
     *
     * Run  tasks up to a time limit
     */
    class RunTasksTimed : public virtual RunTasksTimerSupport
    {
        protected:


        public:
            RunTasksTimed()
            {}


            bool doneSlice(TaskResult res)
            {
                return inTimer_.isExpired();
            }

    };
    //===================================================================
    template<class A, class B>
    class JoinSchedulerTraits : public A, public B
    {
            void beginSlice()
            {
                A::beginSlice();
                B::beginSlice();
            }

            bool doneSlice(TaskResult res)
            {
                return A::doneSlice(res)
                    || B::doneSlice(res)
                    ;
            }
    };
    //===================================================================
    /**
     * Trait class for `TaskScheduler`.
     *
     * Run N tasks up to a time limit
     */
    class  RunNTasksTimed
    : public JoinSchedulerTraits<RunNTasks,RunTasksTimed>
    {
    };

    //===================================================================
    class ATaskScheduler
        : public EnableableTask,
        	public virtual RunTasksTimerSupport
    {

    };

}
#endif
