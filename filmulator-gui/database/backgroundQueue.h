#ifndef BACKGROUNDQUEUE_H
#define BACKGROUNDQUEUE_H

#include <QObject>
#include "../core/imagePipeline.h"
#include <QMutex>
#include <QMutexLocker>
#include <QtSql/QSqlQuery>
#include <QList>
#include <QString>

/* The background queue will handle:
 *
 * * Moving things around in the database (enqueueing multiple things, batch application of settings)
 * Enqueueing and dequeueing things will bypass the priority queue thing.
 * That needs to happen immediately, and can be more efficient if applied as one change to the db.
 *
 * * Sanitizing the database.
 * * Generating full size thumbnails for quick previews of enqueued things
 * * Deleting full size thumbnails for the enqueued things that are dequeued
 * * Performing final output from the queue.
 *
 * Things I want it to be able to do:
 *
 * * Pipeline the single-threaded and i/o bound things for max performance
 * * Spit out intermediates (will require modifying the imagePipeline class)
 *
 * It will have 'enqueue' commands to for example check a thumbnail (sanitize db),
 *  and check for the raw file's presence on the disk.
 *
 * There will be more than one queue.
 * One for fast things: database-only operations; these will preempt other queues.
 *  It will have to have a QSqlQuery that it passes into other things; it first
 *  executes BEGIN TRANSACTION and then repeatedly does the stuff, then does COMMIT.
 * One for high priority: copying
 * *



struct QueueEntry {
    QString searchID,

};

class BackgroundQueue : public QObject
{
    Q_OBJECT

public:
    explicit BackgroundQueue(QObject *parent = 0);

protected:
    QList< sanitizeQueue;
    std::deque previewQueue;
};

#endif // BACKGROUNDQUEUE_H

