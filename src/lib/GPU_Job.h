#ifndef GPU_JOB_H
#define GPU_JOB_H

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <boost/shared_ptr.hpp>
#include "GPU_MemoryMap.h"

/**
 * @file GPU_Job.h
 */

namespace pelican {

namespace lofar {
class GPU_Kernel;


/**
 * @class GPU_Job
 *  
 * @brief
 *    Specifies a Job to run, its input and output data
 * @details
 * 
 */

class GPU_Job
{
    public:
        typedef enum{ None, Queued, Running, Finished } JobStatus;

    public:
        GPU_Job();
        ~GPU_Job();
        void addKernel( GPU_Kernel* kernel );
        const QList<GPU_Kernel*>& kernels() { return _kernels; };
        inline void setStatus( const JobStatus& status ) { _status = status; };
        void setAsRunning();
        inline JobStatus status() const { return _status; };
        void emitFinished();
        void wait() const;

    private:
        QList<GPU_Kernel*> _kernels;
        // status variables
        bool _processing;
        mutable QMutex _mutex;
        mutable QWaitCondition* _waitCondition;
        JobStatus _status;
};

} // namespace lofar
} // namespace pelican
#endif // GPU_JOB_H
