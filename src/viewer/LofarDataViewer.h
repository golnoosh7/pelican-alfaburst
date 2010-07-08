#ifndef LOFARDATAVIEWER_H
#define LOFARDATAVIEWER_H
#include <QSet>


#include "viewer/DataViewer.h"

/**
 * @file LofarDataViewer.h
 */

namespace pelican {

namespace lofar {
        class PelicanBlobClient;
/**
 * @class LofarDataViewer
 *  
 * @brief
 *    A lofar specific single stream viewer
 * @details
 * 
 */

class LofarDataViewer : public DataViewer
{
    Q_OBJECT

    public:
        LofarDataViewer(  const ConfigNode& config, QWidget* parent=0 );
        ~LofarDataViewer();
        void run();

    private:
        PelicanBlobClient* _client;
        QString _dataStream;
};

} // namespace lofar
} // namespace pelican
#endif // LOFARDATAVIEWER_H 