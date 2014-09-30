#include "ABOutputStream.h"

#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QTextStream>
#include <iostream>

#include "ABData.h"
#include "pelican/utility/ConfigNode.h"

using namespace pelican;
using namespace pelican::ampp;

// Constructs the output stream.
ABOutputStream::ABOutputStream(const ConfigNode& configNode)
    : AbstractOutputStream(configNode)
{
    // Get the filename from the configuration node, and open it for output.
    QList<QString> fileNames = configNode.getOptionList( "file", "name");
    foreach( const QString& filename, fileNames ) {
        addFile(filename);
    }
}

// Destroys the output stream, deleting all the devices it uses.
ABOutputStream::~ABOutputStream()
{
    foreach (QIODevice* device, _devices) {
        delete device;
    }
}

// Adds a file to the output stream and opens it for writing.
void ABOutputStream::addFile(const QString& filename)
{
    verbose(QString("Creating file %1").arg(filename));
    QFile* file = new QFile(filename);
    if (file->open(QIODevice::WriteOnly)) {
        _devices.append(file);
    }
    else {
        std::cerr << "Cannot open file for writing: "
                << filename.toStdString() << std::endl;
        delete file;
    }
}

// Sends the data blob to the output stream.
void ABOutputStream::sendStream(const QString& /*streamName*/,
        const DataBlob* blob)
{
    // Check it's a data blob type we know how to use.
    if (blob->type() != "ABData") return;

    const float* data = ((const ABData*)blob)->ptr();
    unsigned size = ((const ABData*)blob)->size();
    if (data)
    {
        // Construct the comma separated value string.
        QString csv = QString::number(data[0]);
        for (unsigned i = 1; i < size; ++i) {
            csv += "," + QString::number(data[i]);
        }

        // Output the string to each file
        foreach (QIODevice* device, _devices) {
            QTextStream out(device);
            out << csv << endl;
        }
    }
}
