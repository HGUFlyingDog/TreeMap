#include "DiskWindow.h"
#include "ui_DiskWindow.h"
#include <QGraphicsLineItem>
#include <QTimer>
#include <QMessageBox>
#include <QPolygonF>
#include <cmath>

DiskWindow::DiskWindow(QWidget *parent)
    : QWidget(parent), ui(new Ui::DiskWindow), scene(new QGraphicsScene(this)),
    currentStep(0), currentSeqIndex(0) {
    ui->setupUi(this);
    ui->graphicsView->setScene(scene);
    //this->setStyleSheet("background-color: #F5F5DC; color: #333333;");
    setWindowState(Qt::WindowMaximized);
    scene->setSceneRect(0, 0, 800, 400);
    updateAxis();
    setupConnections();
}

void DiskWindow::setupConnections() {
    connect(ui->addBtn, &QPushButton::clicked, this, &DiskWindow::addRequest);
    connect(ui->startBtn, &QPushButton::clicked, this, &DiskWindow::startScan);
}


void DiskWindow::calculateDisplayRange() {
    maxTrack = 100;
    foreach(int track, currentRequests) {
        if(track > maxTrack) maxTrack = track;
    }
    displayMaxTrack = std::max(100, ((maxTrack/50)+1)*50); // 显示范围为50的整数倍
}

void DiskWindow::updateAxis() {
    scene->clear();
    calculateDisplayRange();

    // 水平磁道轴（保留左右边距）
    QGraphicsLineItem *axis = new QGraphicsLineItem(50, 200, 750, 200);
    axis->setPen(QPen(Qt::black, 2));
    scene->addItem(axis);

    // 动态刻度计算
    const int majorInterval = (displayMaxTrack > 200) ? 50 :
                                  (displayMaxTrack > 100) ? 20 : 10;

    // 绘制主要刻度
    for(int track = 0; track <= displayMaxTrack; track += majorInterval) {
        double ratio = static_cast<double>(track)/displayMaxTrack;
        int x = 50 + ratio * 700;

        // 刻度线
        QGraphicsLineItem *tick = new QGraphicsLineItem(x, 190, x, 210);
        scene->addItem(tick);

        // 刻度文本
        QGraphicsTextItem *text = scene->addText(QString::number(track));
        text->setPos(x - text->boundingRect().width()/2, 210);
    }

    // 重新绘制已有请求
    foreach(int track, currentRequests) {
        addVisualRequest(track);
    }
}

QPointF DiskWindow::trackToPoint(int track) {
    double ratio = static_cast<double>(track)/displayMaxTrack;
    int x = 50 + ratio * 700;
    return QPointF(x, 170); // 磁道上方的位置
}

void DiskWindow::addRequest() {
    QString input = ui->trackInput->text().trimmed().replace("，", ",");
    QStringList tracksStr = input.split(',', Qt::SkipEmptyParts);
    bool hasError = false;

    foreach (const QString &s, tracksStr) {
        bool ok;
        int track = s.toInt(&ok);
        if(ok && track >=0) {
            currentRequests.append(track);
            addVisualRequest(track);
        } else {
            hasError = true;
        }
    }

    if(hasError) {
        QMessageBox::warning(this, "错误", "包含无效磁道号（必须为非负整数）");
    }
    updateAxis();
    ui->trackInput->clear();
}

void DiskWindow::startScan() {
    if(animationTimer) animationTimer->stop();
    if(scanner) disconnect(scanner.data(), nullptr, this, nullptr);

    calculateDisplayRange();
    scanner = new DiskScan(0, displayMaxTrack, ui->directionCombo->currentIndex() == 0);

    // 初始化磁头
    QPolygonF triangle;
    triangle << QPointF(0, 0) << QPointF(-10, 30) << QPointF(10, 30);
    head = new QGraphicsPolygonItem(triangle);
    head->setPos(trackToPoint(0));
    head->setBrush(Qt::blue);
    scene->addItem(head);

    animationTimer.reset(new QTimer());
    currentStep = 0;
    currentSeqIndex = 0;

    connect(scanner.data(), &DiskScan::updated, this, [this]() {
        if(scanner) animationTimer->start(500);
    }, Qt::QueuedConnection);

    connect(animationTimer.data(), &QTimer::timeout, this, &DiskWindow::animateScan);

    foreach(int track, currentRequests) {
        scanner->addRequest(track);
    }

    QMetaObject::invokeMethod(scanner.data(), [this]() {
        if(scanner) scanner->execute();
    }, Qt::QueuedConnection);
}

void DiskWindow::animateScan() {
    if(!scanner || currentStep >= scanner->getPath().size()) {
        animationTimer->stop();
        ui->logOutput->append("所有请求处理完成！");
        return;
    }

    auto path = scanner->getPath();
    auto move = path.at(currentStep);
    QPointF oldPos = trackToPoint(move.first);
    QPointF newPos = trackToPoint(move.second);

    // 绘制移动路径
    QGraphicsLineItem *line = new QGraphicsLineItem(oldPos.x(), oldPos.y(), newPos.x(), newPos.y());
    line->setPen(QPen(Qt::darkGreen, 2, Qt::DotLine));
    scene->addItem(line);

    // 移动磁头
    head->setPos(newPos);

    // 更新日志
    if(currentSeqIndex < scanner->getSequence().size()) {
        int currentTrack = scanner->getSequence().at(currentSeqIndex);
        if(move.second == currentTrack) {
            int count = scanner->getTrackRequestCount().value(currentTrack);
            ui->logOutput->append(QString("磁道%1（共%2个请求）已处理")
                                      .arg(currentTrack).arg(count));
            currentSeqIndex++;
        }
    }

    currentStep++;
    scene->update();
}

void DiskWindow::addVisualRequest(int track) {
    QGraphicsEllipseItem *point = new QGraphicsEllipseItem(-5, -5, 10, 10);
    point->setPos(trackToPoint(track));
    point->setBrush(Qt::red);
    scene->addItem(point);
}

DiskWindow::~DiskWindow() {
    delete ui;
    if(animationTimer) animationTimer->stop();
}

void DiskWindow::closeEvent(QCloseEvent *event) {
    // 显示父窗口
    emit(close());
    event->accept();
}
