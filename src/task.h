/** @file
 *  @brief A simple task scheduler - based on http://bleaklow.com/2010/07/20/a_very_simple_arduino_task_manager.html
 */
#ifndef _TASK_H_
#define _TASK_H_




#include "../../PsiCore/src/MilliTimer.h"
#include <stdint.h>
#include "TaskSchedulerBase.h"
namespace psiiot
{

    //=========================================================
    /**
     * Class which implements one-shot and cyclic timing schemes  
     *
     * @note The subclass is responsible for exiting if canRun() returbs false.
     */
    class TimedTask : public EnableableTask 
    {
    protected:
        MilliTimer runTimer_;


        //------------------------------------------------
        /// reset time to `ticks`, and (re)start
        void resetAt(uint32_t ticks)
        {
            enabled_ = true;
            runTimer_.resetAt(ticks);
        }

        //------------------------------------------------
        /*!
            Call this to test if we should run or not
            @param sch      Owning scheduler
         */
        TaskResult canRun(ATaskScheduler* sch)
        {
            if(!enabled_)
            {
                // expired one-shot or stopped cyclic
                return TaskResult::NotRun;
            }

            enabled_ =  !runTimer_.hadExpiredNoReset(
            sch->sliceBeginMillis()
            );

            if(enabled_)
            return TaskResult::NotRun; // timer still running

            if(runTimer_.isCyclic())
            {
                // periodic, running  & done
                runTimer_.resetAt(sch->sliceBeginMillis());
                enabled_ = true; // re-enable cyclic timer
            }

            return TaskResult::Run;
        }        
    
    public:
        inline TimedTask(uint32_t when, bool cyclic, bool en) 
        : EnableableTask(en), runTimer_(when, cyclic)
        { 
        }

        //------------------------------------------------
        
        /// How long have we been waiting?
        inline unsigned long intervalExpired() const {runTimer_.intervalExpired();}

        /// How long have we to go?
        inline unsigned long intervalLeft() const {runTimer_.intervalLeft(); }
        
        inline uint32_t getIntervalBeganMillis() const { return runTimer_.ticksWhenReset();}
        inline uint32_t getInterval() const { return runTimer_.getInterval();}
        inline void setInterval(uint32_t interval) { runTimer_.setInterval(interval);}

        inline bool isCyclic() const { return runTimer_.isCyclic(); }
        inline void setCyclic(bool cy) { runTimer_.setCyclic(cy); }
    };
    //=========================================================
}
#endif