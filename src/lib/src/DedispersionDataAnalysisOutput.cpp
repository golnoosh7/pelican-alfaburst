#include "DedispersionDataAnalysisOutput.h"
#include "DedispersionDataAnalysis.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include "pelican/utility/ConfigNode.h"
#include "pelican/data/DataBlob.h"
#include <cstring>
#include <iostream>
#include <time.h>
namespace pelican {

namespace ampp {


/**
 *@details DedispersionDataAnalysisOutput 
 */
DedispersionDataAnalysisOutput::DedispersionDataAnalysisOutput( const ConfigNode& configNode )
    : AbstractOutputStream(configNode)
{
    QString filepath = configNode.getOption("file", "name");
    char timestr[22];
    time_t     now = time(0);
    struct tm  tstruct;
    tstruct = *localtime(&now);
    strftime(timestr, sizeof timestr, "D%Y%m%dT%H%M%S", &tstruct );
    QString filename;
    filename = filepath + QString("_") + timestr + QString(".dat");
    if( filename != "" )
    {
        addFile( filename );
    }
    // MJD of 1/1/11 is 55562
    struct tm tm;
    if ( strptime("2011-1-1 0:0:0", "%Y-%m-%d %H:%M:%S", &tm) != NULL ){
        _epoch = mktime(&tm);
    }
    else {
        throw( QString("SigprocStokesWriter: unable to set epoch.") );
    }
    _indexOfDump = 0;
}

/**
 *@details
 */
DedispersionDataAnalysisOutput::~DedispersionDataAnalysisOutput()
{
    foreach( QTextStream* stream, _streams) {
        stream->flush();
        delete stream;
    }
    foreach( QIODevice* device, _devices ) {
        delete device;
    }
}

void DedispersionDataAnalysisOutput::addFile(const QString& filename)
{
    QFile* file = new QFile(filename);
    if( file->open( QIODevice::WriteOnly ) )
    {
         _devices.append( file );
         QTextStream* out = new QTextStream(file);
         _streams.append(out);
         *out << "# Generated by DedispersionDataAnalysisOutput\n"
              << "# Events\n"
              << "# ------\n"
              << "# Time|Dm|Amplitude|BinFactor\n"
              << "# ------\n";
         out->setRealNumberPrecision( 14 );
         out->flush();
    }
    else {
        std::cerr << "DedispersionDataAnalysisOutput: unable to open file for writing: " << filename.toStdString() << std::endl;
        delete file;
    }
}

void DedispersionDataAnalysisOutput::sendStream(const QString& /*streamName*/, const DataBlob* dataBlob)
{
  if( dataBlob->type() == "DedispersionDataAnalysis" ) {
    const DedispersionDataAnalysis* data = static_cast<const DedispersionDataAnalysis*>(dataBlob);
    float rms = data->getRMS();
    //QMutexLocker lock(_mutex);
    foreach( QTextStream* out, _streams ) {
      float SNRmax, DMthis;
      SNRmax = 0.0;
      //        foreach( const DedispersionEvent& e, data->events() ) {
      // Avoid printing the first event, which is only used for the timestampi
      for (unsigned i=1; i<data->eventsFound(); ++i){
        const DedispersionEvent& e = data->events()[i];
        //std::cout << e.getTime() << ", " << _epoch << std::endl;
        //double mjdStamp = (e.getTime()-_epoch)/86400 + 55562.0;
        double mjdStamp = (e.getTime() / 86400) + 40587;
        //                    float SNR = e.amplitude()/rms;
        float SNR = e.mfValue()/(rms * sqrt(e.mfBinning()));
        if (SNR > SNRmax){
          DMthis = e.dm();
          SNRmax = SNR;
        }
        int bf = (int)e.mfBinning();
        *out << left << mjdStamp << ",   " << e.dm() << ", " << SNR << ", " << bf << "\n";
      }
      //double mjdBlock = (data->events()[0].getTime()-_epoch)/86400 + 55562.0;
      double mjdBlock = (data->events()[0].getTime() / 86400) + 40587;
      ++_indexOfDump;
      *out << "# Written buffer :" << _indexOfDump << " | MJDstart: " << mjdBlock <<
        " | Best DM: "<< DMthis << " | Max SNR: " << SNRmax << "  Done\n";
      out->flush();
    }
  }
}
  
} // namespace ampp
} // namespace pelican
