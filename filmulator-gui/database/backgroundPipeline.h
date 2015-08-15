#ifndef BACKGROUNDPIPELINE_H
#define BACKGROUNDPIPELINE_H

#include <QObject>
#include "../core/imagePipeline.h"
#include <QMutex>
#include <QMutexLocker>

/* The background pipeline will handle:
 *
 * * Sanitizing the database.
 * * Generating full size thumbnails for quick previews of enqueued things
 * * Deleting full size thumbnails for the enqueued things
 * * Performing final output from the queue.
 *
 * Things I want it to be able to do:
 *
 * * Pipeline the single-threaded and i/o bound things for max performance
 * * Spit out intermediates (will require modifying the imagePipeline class)
 *
 * It will have 'enqueue' commands to for example check a thumbnail (sanitize db),
 *  and check for the raw file's presence on the disk.

class BackgroundPipeline : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundPipeline(QObject *parent = 0);
    void
};

#endif // BACKGROUNDPIPELINE_H

