#include "DedispersionDataGenerator.h"
#include "pelican/utility/ConfigNode.h"
#include "pelican/output/DataBlobFile.h"
#include "SpectrumDataSet.h"
#include "WeightedSpectrumDataSet.h"
#include "DedispersionSpectra.h"
#include "DedispersionModule.h"
#include "LockingPtrContainer.hpp"
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QList>
#include <fstream>
#include <boost/bind.hpp>


namespace pelican {

namespace lofar {


/**
 *@details DedispersionDataGenerator 
 */
DedispersionDataGenerator::DedispersionDataGenerator()
{
    // set default values (typical LOFAR station values)
    nSamples = 6400; // samples per blob
    nSubbands = 32;
    nChannels = 64; // 2048 total channels (32x64)
    startBin = 0; // offset bin (time) to start the dedispersion
    signalWidth = 10; // width of the dedispersion signal

    fch1 = 150;
    foff = -6.0/(double)(nSubbands*nChannels);
    tsamp = 0.00032768; // time sample length (seconds)
}

/**
 *@details
 */
DedispersionDataGenerator::~DedispersionDataGenerator()
{
}

QList<SpectrumDataSetStokes*> DedispersionDataGenerator::generate( int numberOfBlocks , float dm )
{
    QList<SpectrumDataSetStokes*> data;

    for( int i=0; i < numberOfBlocks; ++i ) {
        SpectrumDataSetStokes* stokes = new SpectrumDataSetStokes;
        stokes->resize(nSamples, nSubbands, 1, nChannels);
        data.append(stokes);

        int offset = i * nSamples;
        //stokes->setLofarTimestamp(channeliserOutput->getLofarTimestamp());
        for (unsigned int t = 0; t < nSamples; ++t ) {
            for (unsigned s = 0; s < nSubbands; ++s ) {
                for (unsigned c = 0; c < nChannels; ++c) {
                    int absChannel = s * nChannels + c;
                    int index = (int)( dm * (4148.741601 * ((1.0 / (fch1 + (foff * absChannel)) /
                        (fch1 + (foff * absChannel))) - (1.0 / fch1 / fch1))/tsamp ) ) + startBin;
                    int sampleNumber = index - offset;

                    float* I = stokes->spectrumData(t, s, 0);
                    // add a signal of bandwidth signalWidth
                    if( (int)t >= sampleNumber && 
                                  t < (unsigned)sampleNumber + signalWidth ) {
                        I[c] = 1.0;
                    } else {
                        I[c] = 0.0;
                    }
                }
            }
        }
    }
    return data;
}

DedispersionSpectra* DedispersionDataGenerator::dedispersionData( float dedispersionMeasure ) {
    /// generate stokes data and process it using the dedispersion module
    double dedispersionStep = 0.1;
    unsigned ddSamples = 2*dedispersionMeasure/dedispersionStep;
    QList<SpectrumDataSetStokes*> stokes = generate( 1, dedispersionMeasure );
    ConfigNode config;
    QString configString = QString("<DedispersionModule>"
            " <sampleNumber value=\"%1\" />"
            " <frequencyChannel1 value=\"%2\"/>"
            " <sampleTime value=\"%3\"/>"
            " <channelBandwidth value=\"%4\"/>"
            " <dedispersionSamples value=\"%5\" />"
            " <dedispersionStepSize value=\"%6\" />"
            " <numberOfBuffers value=\"2\" />"
            "</DedispersionModule>")
        .arg( nSamples ) // block size should match the buffer size to ensure we get two calls to the GPU
        .arg( startFrequency())
        .arg( timeOfSample())
        .arg( bandwidthOfSample())
        .arg( ddSamples )
        .arg( dedispersionStep );
    config.setFromString(configString);
    QList<DedispersionSpectra*> outputData;
    outputData << new DedispersionSpectra;
    LockingPtrContainer<DedispersionSpectra> outputBuffer(&outputData);
    DedispersionModule ddm(config);
    WeightedSpectrumDataSet* inputData = new WeightedSpectrumDataSet( stokes[0] );

    // launch the dedispersion module and wait for it to return
    QMutex mutex;
    QMutexLocker locker( &mutex );
    QWaitCondition waiter;
    ddm.onChainCompletion( boost::bind( &DedispersionDataGenerator::wakeUp, this,  &waiter, &mutex) );
    ddm.dedisperse( inputData, &outputBuffer );
    waiter.wait(&mutex);

    return outputData[0];
}

void DedispersionDataGenerator::wakeUp( QWaitCondition* waiter, QMutex* mutex ) {
     QMutexLocker locker(mutex);
     waiter->wakeAll();
}

void DedispersionDataGenerator::writeToFile( const QString& filename, const QList<SpectrumDataSetStokes*>& data ) {
    ConfigNode dummy;
    DataBlobFile writer(dummy);
    writer.addFile(filename ,DataBlobFileType::Homogeneous);
    foreach( const SpectrumDataSetStokes* d, data ) {
        writer.send( QString("input"), d );
    }

}

void DedispersionDataGenerator::deleteData( DedispersionSpectra* data ) {
    foreach( SpectrumDataSetStokes* d, data->inputDataBlobs() ) {
        delete d;
    }
    delete data;
}

void DedispersionDataGenerator::deleteData( QList<SpectrumDataSetStokes*>& data )
{
    foreach( SpectrumDataSetStokes* d, data ) {
        delete d;
    }
}

} // namespace lofar
} // namespace pelican