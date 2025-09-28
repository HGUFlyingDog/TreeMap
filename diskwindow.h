#ifndef DISKWINDOW_H
#define DISKWINDOW_H

#include <QWidget>
#include <QGraphicsScene>
#include "DiskScan.h"
#include <QPointer>
#include <QCloseEvent>

namespace Ui {
class DiskWindow;
}

class DiskWindow : public QWidget {
    Q_OBJECT

public:
    explicit DiskWindow(QWidget *parent = nullptr);
    ~DiskWindow();

signals:
    void close();

private slots:
    void addRequest();
    void startScan();
    void animateScan();

private:
    void setupConnections();
    void addVisualRequest(int track);
    void updateAxis();
    void calculateDisplayRange();

    Ui::DiskWindow *ui;
    QGraphicsScene *scene;
    QVector<int> currentRequests;
    QPointer<DiskScan> scanner;
    QScopedPointer<QTimer> animationTimer;

    int currentStep;
    int currentSeqIndex;
    int maxTrack = 100;
    int displayMaxTrack = 100;
    QGraphicsPolygonItem* head;
    QPointF trackToPoint(int track);
    void closeEvent(QCloseEvent *event);
};

#endif // DISKWINDOW_H
