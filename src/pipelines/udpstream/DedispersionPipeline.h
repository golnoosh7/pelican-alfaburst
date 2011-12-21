#ifndef DEDISPERSIONPIPELINE_H
#define DEDISPERSIONPIPELINE_H

#include <QtCore/QList>
#include "pelican/core/AbstractPipeline.h"
#include "pelican/utility/LockingCircularBuffer.hpp"
#include "PPFChanneliser.h"
#include "StokesGenerator.h"
#include "RFI_Clipper.h"
#include "StokesIntegrator.h"
#include "AdapterTimeSeriesDataSet.h"
#include "TimeSeriesDataSet.h"
#include "SpectrumDataSet.h"
#include "SigprocStokesWriter.h"
#include "DedispersionModule.h"
#include "DedispersedTimeSeries.h"
#include "DedispersionAnalyser.h"


/**
 * @file DedispersionPipeline.h
 */

namespace pelican {

namespace lofar {

/**
 * @class DedispersionPipeline
 *  
 * @brief
 *     A dedispersion pipeline for streaming TimeSeries bemaformed Data
 * @details
 * 
 */

class DedispersionPipeline : public AbstractPipeline
{
    public:
        DedispersionPipeline( const QString& streamIdentifier );
        ~DedispersionPipeline();

        /// Initialises the pipeline.
        void init();

        /// Runs the pipeline.
        void run(QHash<QString, DataBlob*>& remoteData);

        /// called internally to free up DataBlobs after they are finished with
        void updateBufferLock();

    private:
        QString _streamIdentifier;

        /// Module pointers
        PPFChanneliser* _ppfChanneliser;
        StokesGenerator* _stokesGenerator;
        StokesIntegrator* _stokesIntegrator;
        RFI_Clipper* _rfiClipper;
        DedispersionModule* _dedispersionModule;
        DedispersionAnalyser* _dedispersionAnalyser;

        /// Local data blobs
        SpectrumDataSetC32* spectra;
        QList<SpectrumDataSetC32*> _spectraBuffer;
        QList<DedispersedTimeSeries<float>*> _dedispersedData;
        LockingCircularBuffer<DedispersedTimeSeries<float>* >* _dedispersedDataBuffer;
        DedispersedTimeSeries<float>* _currentDedispersedData;
        TimeSeriesDataSetC32* timeSeries;
        QList<SpectrumDataSetStokes*> _stokesData;
        LockingCircularBuffer<SpectrumDataSetStokes*>* _stokesBuffer;
        SpectrumDataSetStokes* intStokes;
        WeightedSpectrumDataSet* weightedIntStokes;

};

} // namespace lofar
} // namespace pelican
#endif // DEDISPERSIONPIPELINE_H 
