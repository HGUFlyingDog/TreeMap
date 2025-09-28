#ifndef SHOWKNOWLEDGE_H
#define SHOWKNOWLEDGE_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QButtonGroup>
#include <QTableWidget>
#include <QRadioButton>
#include <QPlainTextEdit>
#include "job.h"

namespace Ui {
class ShowKnowledge;
}

class ShowKnowledge : public QMainWindow
{
    Q_OBJECT

public:
    explicit ShowKnowledge(QWidget *parent = nullptr);
    ~ShowKnowledge();

signals:
    void close();

private slots:
    void on_startButton_clicked();
    void on_pauseButton_clicked();
    void on_continueButton_clicked();
    void on_modeChanged(); //模式切换
    void updateJobTable(int count); //更新作业表格

private:
    Ui::ShowKnowledge *ui;

    QGraphicsScene *scene; //图形场景容器
    QTimer *timer;//定时器驱动
    QList<Job> jobs;//作业集合列表
    qreal currentTime;//当前模拟时间
    qreal maxEndTime; //所有作业的最大结束时间
    QGraphicsLineItem *pointer;//时间指针
    QGraphicsTextItem *timerLabel;//时间标签

    QButtonGroup *modeGroup;//模式选择
    QRadioButton *randomMode;//随机模式
    QRadioButton *manualMode;//手动模式
    QTableWidget *jobTable;//作业参数输入表格

    QPlainTextEdit *statusText;//展示作业当前状态

    const qreal scaleFactor = 50; //时间轴缩放比例
    const qreal rowHeight = 80; //每行作业高度
    const qreal timeAxisHeight = 40;//时间轴区域高度
    const int timeInterval = 1;//时间刻度间隔

    void generateJobs(int n);//生成作业数据
    void createJobItems();//创建图形元素
    //创建箭头标志
    void createArrow(qreal x,qreal y,Qt::GlobalColor color,Job &job,bool isArrival);
    void updateProgress();//更新进度动画

    int is_continue;

    void closeEvent(QCloseEvent *event);

};

#endif // SHOWKNOWLEDGE_H
