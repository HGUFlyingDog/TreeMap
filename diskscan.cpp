#include "DiskScan.h"
#include <QDebug>
#include <algorithm>

DiskScan::DiskScan(int startPos, int maxTrack, bool initialDirection)
    : currentPos(startPos), maxTrack(maxTrack), direction(initialDirection) {}

void DiskScan::addRequest(int track) {
    requests.append(track);
}

void DiskScan::execute() {
    QVector<int> left = requests;
    sequence.clear();
    path.clear();
    trackRequestCount.clear();
    std::sort(left.begin(), left.end());

    if (direction) {
        for (int pos = currentPos; pos <= maxTrack; ++pos) processTrack(pos, left);
        for (int pos = maxTrack; pos >= 0; --pos) processTrack(pos, left);
    } else {
        for (int pos = currentPos; pos >= 0; --pos) processTrack(pos, left);
        for (int pos = 0; pos <= maxTrack; ++pos) processTrack(pos, left);
    }

    emit updated();
}

void DiskScan::processTrack(int pos, QVector<int>& left) {
    int count = left.count(pos);
    if (count > 0) {
        sequence.append(pos);
        trackRequestCount[pos] = count;
        if (currentPos != pos) path.append(qMakePair(currentPos, pos));
        currentPos = pos;
        left.removeAll(pos);
    }
}

QVector<int> DiskScan::getSequence() const { return sequence; }
QVector<QPair<int, int>> DiskScan::getPath() const { return path; }
QMap<int, int> DiskScan::getTrackRequestCount() const { return trackRequestCount; }
