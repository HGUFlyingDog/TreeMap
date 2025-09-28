#include "sjf.h"
#include "ui_sjf.h"
#include <QRandomGenerator>
#include <queue>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QHeaderView>
#include <QMessageBox>
#include <algorithm>

// //初始化作业数据
// Job::Job(int id,int arrival,int burst) :
//     id(id),arrivalTime(arrival),burstTime(burst),
//     startTime(0),endTime(0),fgRect(nullptr),
//     arrivalArrow(nullptr),startArrow(nullptr),endLine(nullptr),
//     arrivalText(nullptr),startText(nullptr),endText(nullptr) {}

// Job::~Job(){
//     delete fgRect;//防止野指针
//     delete arrivalArrow;
//     delete startArrow;
//     delete endLine;
//     delete arrivalText;
//     delete startText;
//     delete endText;
// }

SJF::SJF(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SJF)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    //this->setStyleSheet("background-color: #F5F5DC; color: #333333;");
    is_continue = 1;
    //初始化图形场景
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);//启用抗锯齿，避免边缘出现锯齿现象

    //定时器设置，每500ms触发一次
    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&SJF::updateProgress);

    //初始化时间标签
    timerLabel = new QGraphicsTextItem("0.0");
    timerLabel->setDefaultTextColor(Qt::red);
    //scene->addItem(timerLabel);

    //初始化模式选择
    modeGroup = new QButtonGroup(this);
    randomMode = ui->randomMode;
    manualMode = ui->manualMode;
    modeGroup->addButton(randomMode);
    modeGroup->addButton(manualMode);
    randomMode->setChecked(true);//默认为随机

    //作业表格设置
    jobTable = new QTableWidget(this);
    jobTable->setColumnCount(2);//两列,到达时间和运行时间
    jobTable->setHorizontalHeaderLabels({"到达时间","运行时间"});
    jobTable->verticalHeader()->hide();//隐藏行号
    jobTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//自动拉伸列宽
    jobTable->setEditTriggers(QAbstractItemView::AllEditTriggers);//允许编辑
    jobTable->setSelectionMode(QAbstractItemView::NoSelection);//禁用选择表格
    jobTable->hide();//最初隐藏表格

    //布局调整
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->centralWidget->layout());
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(randomMode);
    modeLayout->addWidget(manualMode);
    mainLayout->insertLayout(0,modeLayout);//插入主布局顶部
    mainLayout->insertWidget(1,jobTable);//插入表格

    // 添加状态文本框初始化
    statusText = ui->statusText;
    statusText->setStyleSheet("font-family: Consolas; font-size: 12pt;");

    //信号连接
    //作业数量变化时更新表格
    connect(ui->spinBox,QOverload<int>::of(&QSpinBox::valueChanged),this,&SJF::updateJobTable);
    //模式切换事件
    connect(modeGroup,QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),this,&SJF::on_modeChanged);
}

SJF::~SJF()
{
    delete ui;
    delete scene;
}

void SJF::on_startButton_clicked()
{
    int n = ui->spinBox->value();//获取作业数量
    generateJobs(n);//生成作业数据
    createJobItems();//创建图形元素

    currentTime = 0; //重置模拟时间
    timerLabel->setPlainText("0.0");
    if(timer->isActive()) timer->stop(); //确保定时器停止
    timer->start(500);//每500ms变化一次
}

void SJF::on_pauseButton_clicked()
{
    timer->stop();
}

void SJF::on_continueButton_clicked()
{
    if(!timer->isActive() && currentTime < maxEndTime)
        timer->start(500); //继续
}

//更新模式
void SJF::on_modeChanged()
{
    bool manual = manualMode->isChecked();
    jobTable->setVisible(manual);
    updateJobTable(ui->spinBox->value());
}

void SJF::updateJobTable(int count)
{
    //仅仅在手动模式下才能运行
    if(!manualMode->isChecked()) return;

    jobTable->setRowCount(count);
    for(int i = 0;i<count;i++)
    {
        //到达时间列
        if(!jobTable->cellWidget(i,0)){//未初始化
            QSpinBox *arrivalSpin = new QSpinBox();
            arrivalSpin->setMinimum(0);
            arrivalSpin->setMaximum(1000);
            jobTable->setCellWidget(i,0,arrivalSpin);//将定义的控件放到当前位置
        }
        //运行时间列
        if(!jobTable->cellWidget(i, 1)) {
            QSpinBox* burstSpin = new QSpinBox();
            burstSpin->setMinimum(1);
            burstSpin->setMaximum(1000);
            jobTable->setCellWidget(i, 1, burstSpin);
        }
    }
}


void SJF::createJobItems()
{
    //清空所有图形项
    scene->clear();
    // scene = new QGraphicsScene(this);
    // ui->graphicsView->setScene(scene);
    // ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    // scene->addItem(timeLabel);

    // 绘制时间轴
    //计算时间轴Y左边，用作业数量*作业高度+时间轴高度
    qreal timeAxisY = jobs.size() * rowHeight + timeAxisHeight;
    //创建矩形，水平线，从0到最大结束时间
    QGraphicsLineItem* timeAxis = scene->addLine(0, timeAxisY,maxEndTime * scaleFactor, timeAxisY);
    //设置黑色2px粗线条
    timeAxis->setPen(QPen(Qt::black, 2));

    // 时间刻度
    for(qreal t = 0; t <= maxEndTime; t += timeInterval) {
        qreal x = t * scaleFactor;//计算时间点对应的坐标
        //绘制垂直刻度线，时间轴上5px长度的标记
        scene->addLine(x, timeAxisY - 5, x, timeAxisY + 5, QPen(Qt::black));

        //添加时间坐标文本
        QGraphicsTextItem* text = scene->addText(QString::number(t));
        text->setPos(x - 10, timeAxisY + 10);//调整标签位置：居中偏左
    }

    // 创建作业项可视化
    for(int i = 0; i < jobs.size(); ++i) {
        Job &job = jobs[i]; //获取当前作业引用
        qreal y = i * rowHeight;//计算当前作业的y坐标

        // 行背景
        QColor bgColor = (i % 2 == 0) ? QColor(245, 245, 245) : QColor(255, 255, 255);//设置为浅灰/白色交叉
        QGraphicsRectItem* bg = scene->addRect(0, y, maxEndTime * scaleFactor, rowHeight);//创建矩形
        bg->setBrush(bgColor);
        bg->setZValue(-1);//设置图层在底层

        // 作业标签
        QGraphicsTextItem* label = scene->addText(QString("J%1").arg(job.id));
        label->setPos(-60, y + 15);//在左侧外部区域

        // 到达时间箭头
        createArrow(job.arrivalTime * scaleFactor, y - 25, Qt::darkGreen, job, true);//设置箭头在上方，绿色
        job.arrivalText->setPlainText(QString("AT:%1").arg(job.arrivalTime));

        // 开始时间箭头
        createArrow(job.startTime * scaleFactor, y + rowHeight + 5, Qt::blue, job, false);//正下方
        job.startText->setPlainText(QString("ST:%1").arg(job.startTime));

        // 结束时间标记
        qreal endX = job.endTime * scaleFactor;
        job.endLine = scene->addLine(endX, y, endX, y + rowHeight);//创建垂直线
        job.endLine->setPen(QPen(Qt::red, 2));//红2px
        job.endText = scene->addText(QString("ET:%1").arg(job.endTime));
        job.endText->setDefaultTextColor(Qt::red);
        job.endText->setPos(endX + 5, y + rowHeight/2 - 10);

        // 进度条
        job.fgRect = scene->addRect(0, y, 0, rowHeight);//初始宽度为0，
        job.fgRect->setBrush(QColor(100, 200, 100));//绿色填充
    }

    // 时间指针
    pointer = scene->addLine(0, -20, 0, jobs.size() * rowHeight + timeAxisHeight, //顶部延伸20px，底部到最低，
                             QPen(Qt::red, 2));
    pointer->setZValue(1);//最顶层
}


void SJF::generateJobs(int n)
{
    //清空所有作业列表
    jobs.clear();

    //手动处理
    if(manualMode->isChecked()) {
        // 保持原始输入顺序
        for(int i = 0; i < n; ++i) {
            QSpinBox* arrival = qobject_cast<QSpinBox*>(jobTable->cellWidget(i, 0));
            QSpinBox* burst = qobject_cast<QSpinBox*>(jobTable->cellWidget(i, 1));

            if(!arrival || !burst) {
                QMessageBox::warning(this, "错误", "请输入所有作业参数");
                return;
            }
            jobs.append(Job(i, arrival->value(), burst->value()));//创建作业并且加入列表
        }

        // // 创建调度顺序副本（按到达时间排序）
        // QList<Job*> scheduleOrder;
        // for(Job& job : jobs) {//将原始作业加入排序序列
        //     scheduleOrder.append(&job);
        // }
        // std::sort(scheduleOrder.begin(), scheduleOrder.end(), //按照到达时间升序排序
        //           [](const Job* a, const Job* b) {
        //               return a->arrivalTime < b->arrivalTime;
        //           });

        // // 计算调度时间
        // qreal prevEnd = 0; //计算前一个作业的结束时间
        // for(Job* job : scheduleOrder) {
        //     job->startTime = qMax(prevEnd, (qreal)job->arrivalTime); //开始时间取前一个作业的结束时间和当前作业的到达时间
        //     job->endTime = job->startTime + job->burstTime;
        //     prevEnd = job->endTime;
        // }
        // maxEndTime = prevEnd;//最终停止时间

        QList<Job*> sortedByArrival;
        for (Job& job : jobs) {
            sortedByArrival.append(&job);
        }
        std::sort(sortedByArrival.begin(), sortedByArrival.end(),
                  [](const Job* a, const Job* b) { return a->arrivalTime < b->arrivalTime; });

        auto cmp = [](Job* a, Job* b) { return a->burstTime > b->burstTime; };
        std::priority_queue<Job*, std::vector<Job*>, decltype(cmp)> pq(cmp);

        qreal currentTime = 0;
        int index = 0;
        while (index < sortedByArrival.size() || !pq.empty()) {
            // 将到达的作业加入队列
            while (index < sortedByArrival.size() && sortedByArrival[index]->arrivalTime <= currentTime) {
                pq.push(sortedByArrival[index]);
                index++;
            }

            if (pq.empty()) {
                if (index < sortedByArrival.size()) {
                    currentTime = sortedByArrival[index]->arrivalTime;
                }
            } else {
                Job* currentJob = pq.top();
                pq.pop();
                currentJob->startTime = currentTime;
                currentJob->endTime = currentJob->startTime + currentJob->burstTime;
                currentTime = currentJob->endTime;
            }
        }
        maxEndTime = currentTime;

    } else {
        // 生成随机作业并调度（SJF）
        for(int i = 0; i < n; ++i) {
            int arrivalTime = QRandomGenerator::global()->bounded(0, n);
            int burstTime = QRandomGenerator::global()->bounded(1, 10);
            jobs.append(Job(i, arrivalTime, burstTime)); // 正确存入 jobs
        }

        // 按到达时间排序
        QList<Job*> sortedByArrival;
        for (Job& job : jobs) {
            sortedByArrival.append(&job);
        }
        std::sort(sortedByArrival.begin(), sortedByArrival.end(),
                  [](const Job* a, const Job* b) { return a->arrivalTime < b->arrivalTime; });

        // 定义优先队列（最短作业优先）
        auto cmp = [](Job* a, Job* b) { return a->burstTime > b->burstTime; };
        std::priority_queue<Job*, std::vector<Job*>, decltype(cmp)> pq(cmp);

        // 调度逻辑
        qreal currentTime = 0;
        int index = 0;
        while (index < sortedByArrival.size() || !pq.empty()) {
            // 将到达的作业加入队列
            while (index < sortedByArrival.size() && sortedByArrival[index]->arrivalTime <= currentTime) {
                pq.push(sortedByArrival[index]);
                index++;
            }

            if (pq.empty()) {
                // 队列为空，跳转到下一个到达时间
                if (index < sortedByArrival.size()) {
                    currentTime = sortedByArrival[index]->arrivalTime;
                }
            } else {
                // 执行最短作业
                Job* currentJob = pq.top();
                pq.pop();
                currentJob->startTime = currentTime;
                currentJob->endTime = currentJob->startTime + currentJob->burstTime;
                currentTime = currentJob->endTime;
            }
        }
        maxEndTime = currentTime;
    }
}



void SJF::createArrow(qreal x, qreal y, Qt::GlobalColor color, Job& job, bool isArrival)
{
    const qreal arrowHeight = 20; //箭头高度

    // 箭头线
    //x水平位置，y顶部起始位置，y+arrowHeight底部结束位置
    QGraphicsLineItem* arrow = scene->addLine(x, y, x, y + arrowHeight);
    arrow->setPen(QPen(color, 1.5));

    // 箭头头部
    QPolygonF head;
    head << QPointF(x-4, y+arrowHeight-8)//左下方4px
         << QPointF(x, y+arrowHeight)//箭头顶点
         << QPointF(x+4, y+arrowHeight-8);//右下方
    scene->addPolygon(head, QPen(color), QBrush(color));//图形，描边，填充

    // 文本标签
    QGraphicsTextItem *text = scene->addText("");//说明文本
    text->setDefaultTextColor(color);
    text->setPos(x + 5, isArrival ? y + arrowHeight + 2 : y - 20);

    if(isArrival) {
        job.arrivalArrow = arrow;
        job.arrivalText = text;
    } else {
        job.startArrow = arrow;
        job.startText = text;
    }
}



void SJF::updateProgress()
{
    const qreal epsilon = 0.01;//精度容差
    currentTime += 0.5;//每次进度条推进时间

    // 更新指针->移动的红线
    qreal pointerX = currentTime * scaleFactor; //当前时间*范围大小
    //pointer->setLine(pointerX, -20, pointerX, scene->height());
    pointer->setLine(pointerX,-20,pointerX,jobs.size() * rowHeight + timeAxisHeight+20);//指针的起始和结束
    //更新时间标签文本，保留一位小数
    timerLabel->setPlainText(QString::number(currentTime, 'f', 1));
    //设置位置
    timerLabel->setPos(pointerX - 15, -30);

    bool allFinished = true; //标记作业是否全部完成
    for(Job& job : jobs) {
        //已完成状态判断
        if(currentTime >= job.endTime - epsilon) {
            // 已完成，设置进度条为完整长度
            job.fgRect->setRect(job.startTime * scaleFactor, job.id * rowHeight,
                                job.burstTime * scaleFactor, rowHeight);
            allFinished = true;
        }
        else if(currentTime >= job.startTime - epsilon) {
            // 执行中,计算比例进度，已执行时间/总时间
            qreal progress = (currentTime - job.startTime) / job.burstTime;
            //进度条约束在0-1
            progress = qBound(0.0, progress, 1.0);
            //设置动态增长进度条  x起始位置不变，y位置，当前进度宽度
            job.fgRect->setRect(job.startTime * scaleFactor, job.id * rowHeight,
                                progress * job.burstTime * scaleFactor, rowHeight);
            allFinished = false;
        }
        else {
            // 未开始，0，y，0
            job.fgRect->setRect(0, job.id * rowHeight, 0, rowHeight);
            allFinished = false;
        }
    }


    QString status;
    for(const Job& job : jobs) {
        QString state;

        if(currentTime >= job.endTime - epsilon) {
            state = "已完成";
        }
        else if(currentTime >= job.startTime - epsilon) {
            state = "正在执行";
        }
        else if(currentTime >= job.arrivalTime - epsilon) {
            state = "已到达（等待）";
        }
        else {
            state = "未到达";
        }

        status += QString("作业%1：%2（AT:%3 BT:%4）\n")
                      .arg(job.id+1)
                      .arg(state)
                      .arg(job.arrivalTime)
                      .arg(job.burstTime);
    }

    statusText->setPlainText(status);


    //当时间超过最晚结束时间，计时器停止，隐藏时间指针，隐藏时间标签
    if(currentTime >= maxEndTime - epsilon) {
        timer->stop();
        pointer->hide();
        timerLabel->hide();
    }
}
void SJF::closeEvent(QCloseEvent *event) {
    // 显示父窗口
    emit(close());
    event->accept();
}
