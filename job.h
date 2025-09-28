#ifndef JOB_H
#define JOB_H

#include "qgraphicsitem.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QButtonGroup>
#include <QTableWidget>
#include <QRadioButton>
#include <QPlainTextEdit>

struct Job{
    int id;
    int arrivalTime;
    int burstTime; //运行时间
    qreal startTime; //实际开始时间
    qreal endTime; //实际结束时间

    //图形项
    QGraphicsRectItem *fgRect;//进度条前景
    QGraphicsLineItem *arrivalArrow;//到达时间箭头
    QGraphicsLineItem *startArrow;//开始
    QGraphicsLineItem *endLine;//结束标记线
    QGraphicsTextItem *arrivalText;//到达时间文本
    QGraphicsTextItem *startText;//开始时间文本
    QGraphicsTextItem *endText;//结束时间文本

    Job(int id,int arrival,int burst);//构造函数
    ~Job(){
        delete fgRect;//防止野指针
        delete arrivalArrow;
        delete startArrow;
        delete endLine;
        delete arrivalText;
        delete startText;
        delete endText;
    };//析构函数，释放资源
};

//初始化作业数据
inline Job::Job(int id,int arrival,int burst) :
    id(id),arrivalTime(arrival),burstTime(burst),
    startTime(0),endTime(0),fgRect(nullptr),
    arrivalArrow(nullptr),startArrow(nullptr),endLine(nullptr),
    arrivalText(nullptr),startText(nullptr),endText(nullptr) {}

#endif // JOB_H
