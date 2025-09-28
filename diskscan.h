#ifndef DISKSCAN_H
#define DISKSCAN_H

#include <QObject>
#include <QVector>
#include <QPair>
#include <QMap>

class DiskScan : public QObject {
    Q_OBJECT
public:
    explicit DiskScan(int startPos, int maxTrack, bool initialDirection);

    void addRequest(int track);
    void execute();
    QVector<int> getSequence() const;
    QVector<QPair<int, int>> getPath() const;
    QMap<int, int> getTrackRequestCount() const;
    QVector<int> getOriginalRequests() const { return requests; }

signals:
    void updated();

private:
    void processTrack(int pos, QVector<int>& left);

    int currentPos;
    int maxTrack;
    bool direction;
    QVector<int> requests;
    QVector<int> sequence;
    QVector<QPair<int, int>> path;
    QMap<int, int> trackRequestCount;
};

#endif // DISKSCAN_H
