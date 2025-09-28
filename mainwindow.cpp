#include "mainwindow.h"
#include "logwidget.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include "showknowledge.h"
#include <QStandardPaths>
#include <QTextStream>
#include <QProgressDialog>
#include <QCloseEvent>
#include <windows.h>
#include "sjf.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QListWidgetItem>
#include <diskwindow.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->setStyleSheet("background-color: #F5F5DC; color: #333333;");
    setWindowState(Qt::WindowMaximized);

    // 取消以下注释
    ui->recommendationList->setStyleSheet(
        "QListWidget {"
        "   background: #f5f5f5;" // 背景浅灰色
        "   border-top: 2px solid #e0e0e0;" //2px灰边框
        "   padding: 8px;" //内缩8px
        "}"
        "QListWidget::item {"
        "   color: #000000;"
        "   background: white;"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 4px;" //白背景搭配灰边框圆角
        "   margin: 3px;" //项之间保留3px距离
        "   padding: 8px;" // 文字与像素保持8px
        "   font-family: 'Microsoft YaHei';" //微软雅黑11号字体
        "   font-size: 11pt;"
        "}"
        "QListWidget::item:hover {"
        "   color: #000000;"  //保持悬停时黑色
        "   background: #f0f8ff;"
        "   border: 1px solid #87cefa;" //背景变成淡蓝色，边框变成浅蓝色
        "}"
        "QListWidget::item:selected {"
        "   color: #000000;"  //保持选中时黑色
        "   background: #fffacd;" //选中时背景变成浅黄色，边框金色
        "   border: 1px solid #ffd700;"
        "}"
        );

    // 确保树视图支持透明背景
    ui->treeView->setStyleSheet("QTreeView { background: transparent; }");

    this->setWindowTitle("操作系统知识点展示平台");
    //setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    connect(ui->replace,&QAction::triggered,this,&MainWindow::relogin);
    // 在 MainWindow 构造函数中连接信号
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::onTreeViewClicked);
    // ShowKnowledge *skl = new ShowKnowledge;
    connect(this,&MainWindow::showtime,this,&MainWindow::show_which_know);
    // LogWidget *lw = new LogWidget;
    connect(ui->openButton,&QPushButton::clicked,this,&MainWindow::openword);

    // 在MainWindow构造函数中添加
    connect(ui->recommendationList, &QListWidget::itemClicked, this, &MainWindow::onRecommendationClicked);

    //getinit();
    get_tree();

    ui->treeView->setModel(&model);

    statusBar()->addPermanentWidget(new QLabel("账户："));
    userLabel = new QLabel("未登录");
    statusBar()->addPermanentWidget(userLabel);

    statusBar()->addPermanentWidget(new QLabel("学习时间："));
    timeLabel = new QLabel("00:00:00");
    statusBar()->addPermanentWidget(timeLabel);

    // 初始化计时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=](){
        totalSeconds++;
        timeLabel->setText(
            QString("%1:%2:%3")
                .arg(totalSeconds/3600, 2, 10, QLatin1Char('0'))
                .arg((totalSeconds%3600)/60, 2, 10, QLatin1Char('0'))
                .arg(totalSeconds%60, 2, 10, QLatin1Char('0'))
            );
    });
    connect(this,&MainWindow::gengxin,[=](){
        userLabel->setText(QString("%1").arg(currentUser));
    });
    timer->start(1000);

    // 在MainWindow构造函数中添加数据库初始化
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "click_connection");
    db.setDatabaseName("UserClickRecords.db");
    if (db.open()) {
        QSqlQuery query(db);
        query.exec(
            "CREATE TABLE IF NOT EXISTS click_records ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "account TEXT NOT NULL, "
            "chapter TEXT NOT NULL, "
            "section TEXT NOT NULL, "
            "knowledge TEXT NOT NULL, "
            "click_time DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "click_count INTEGER DEFAULT 1)"
            );
        // 创建复合索引优化查询
        query.exec("CREATE INDEX IF NOT EXISTS idx_user_knowledge ON click_records (account, chapter, section, knowledge)");
    } else {
        qWarning() << "无法打开点击记录数据库:" << db.lastError();
    }

    QTimer::singleShot(100, this, [this](){
        if(!currentUser.isEmpty()) {
            updateRecommendations();
        } else {
            showPlaceholderRecommendation();
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::relogin()
{
    currentUser.clear();
    showPlaceholderRecommendation();
    this->close();
    LogWidget *lw = new LogWidget;
    lw->show();
}

void MainWindow::get_tree() //这里可以加入从文件中读取
{
    // auto one_capter = new QStandardItem("第一章");
    // auto item = new QStandardItem("调度算法");

    // auto fcfs = new QStandardItem("先来先服务算法");

    // item->appendRow(fcfs);
    // one_capter->appendRow(item);

    // list.push_back(one_capter);
    // model.appendRow(list);

    QString exePath = QCoreApplication::applicationDirPath();
    qDebug() << "Application directory path:" << exePath;

    // 获取目录路径
    QDir dir(exePath);
    dir.cdUp();//返回上级目录
    dir.cdUp();
    dir.cdUp();

    QString dirPath = dir.absolutePath(); // 获取绝对路径

    QString filepath = dirPath+"\\";
    // 根据节点文本跳转到对应页面
    filepath += "capter.json";
    readOSJsonFile(filepath);
    for(auto itChapter = rootObj.constBegin();itChapter!=rootObj.constEnd();itChapter++){
        QString chapterName = itChapter.key();
        QJsonValue chapterValue = itChapter.value();

        if(!chapterValue.isObject()){ //检测章节值是否为对象
            qWarning()<<"章节格式错误: "<<chapterName;
            continue;
        }

        QJsonObject chapterObj = chapterValue.toObject();
        auto capter = new QStandardItem(chapterName);

        for(auto itSection = chapterObj.constBegin();itSection != chapterObj.constEnd();itSection++){
            QString sectionName = itSection.key();
            QJsonValue sectionValue = itSection.value();

            if(!sectionValue.isArray()){//检查小章节是否为数组
                qWarning()<<"小章节格式错误: "<<sectionName;
                continue;
            }

            QJsonArray knowledgeArray = sectionValue.toArray();
            auto section = new QStandardItem(sectionName);

            for(const QJsonValue &knowledegValue : knowledgeArray){
                if(!knowledegValue.isString()){
                    qWarning()<<"知识点格式错误";
                    continue;
                }
                auto knowledge = new QStandardItem(knowledegValue.toString());

                section->appendRow(knowledge);
            }

            capter->appendRow(section);
        }
        model.appendRow(capter);
    }
}

// 在MainWindow::onTreeViewClicked中替换为以下代码
void MainWindow::onTreeViewClicked(const QModelIndex &index) {
    if (!index.isValid()) return;

    // 获取知识点路径的逻辑保持不变
    QStringList path;
    QModelIndex current = index;
    while (current.isValid()) {
        path.prepend(current.data().toString());
        current = current.parent();
    }

    if (path.size() >= 3) {
        QString chapter = path[0];
        QString section = path[1];
        QString knowledge = path[2];

        QSqlDatabase db = QSqlDatabase::database("click_connection");
        if (!db.isOpen()) {
            qWarning() << "数据库未连接";
            return;
        }

        // 使用事务处理方案
        db.transaction();

        // 尝试更新
        QSqlQuery updateQuery(db);
        updateQuery.prepare(
            "UPDATE click_records SET "
            "click_count = click_count + 1, "
            "click_time = datetime('now') " //更新时间戳为当前时间
            "WHERE account = ? AND chapter = ? AND section = ? AND knowledge = ?" //通过参数定义唯一记录，？为占位符
            );
        updateQuery.addBindValue(currentUser);
        updateQuery.addBindValue(chapter);
        updateQuery.addBindValue(section);
        updateQuery.addBindValue(knowledge);
        if (!updateQuery.exec()) {
            qWarning() << "更新失败:" << updateQuery.lastError().text();
        }

        // 插入新记录（如果需要）
        if (updateQuery.numRowsAffected() == 0) {
            QSqlQuery insertQuery(db);
            insertQuery.prepare(
                "INSERT INTO click_records "
                "(account, chapter, section, knowledge, click_count) "
                "VALUES (?, ?, ?, ?, 1)"
                );
            insertQuery.addBindValue(currentUser);
            insertQuery.addBindValue(chapter);
            insertQuery.addBindValue(section);
            insertQuery.addBindValue(knowledge);
            if (!insertQuery.exec()) {
                qWarning() << "插入失败:" << insertQuery.lastError().text();
            }
        }

        if (!db.commit()) {
            qWarning() << "事务提交失败:" << db.lastError().text();
        }

        // 更新推荐列表
        updateRecommendations();
    }

    // 其他逻辑（如打开文档）保持不变


    if (!index.model()->hasChildren(index)) {
        // 获取节点文本
        QString nodeText = index.data(Qt::DisplayRole).toString();

        // 获取并构建文档路径
        QString filepath = buildDocumentPath(nodeText);

        if (!filepath.isEmpty()) {
            qDebug() << "Document path:" << filepath;
            convertAndOpenInBrowser(filepath);
        } else {
            qWarning() << "Failed to build document path for:" << nodeText;
        }
    }
}

void MainWindow::convertAndOpenInBrowser(const QString &docxPath) {
    // 首先检查文件是否存在
    if (!QFileInfo(docxPath).exists()) {
        qDebug() << "文件不存在：" << docxPath;
        word_add = "";
        ui->showText->setPlainText("");
        QMessageBox::warning(this, "错误", "指定的Word文件不存在！");
        return;
    }

    word_add = docxPath;
    qDebug() << "正在处理文件：" << docxPath;

    // 使用更可靠的方式查找 LibreOffice
    QString libreOfficePath = "C:/Program Files/LibreOffice/program/soffice.exe";
    if (!QFileInfo(libreOfficePath).exists()) {
        QMessageBox::warning(this, "错误", "找不到LibreOffice安装！");
        return;
    }

    QProcess process;
    process.setProgram(libreOfficePath);
    process.setArguments({
        "--headless",
        "--convert-to", "html",
        "--outdir", QFileInfo(docxPath).absolutePath(),
        docxPath
    });

    // 启动进程并检查是否成功
    process.start();
    if (!process.waitForStarted()) {
        QMessageBox::warning(this, "错误", "无法启动LibreOffice转换进程！");
        return;
    }

    // 等待完成，更长超时时间
    if (!process.waitForFinished(60000)) {
        QMessageBox::warning(this, "错误", "转换Word文件超时！");
        return;
    }

    // 检查退出状态
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        QMessageBox::warning(this, "错误",
                             QString("转换失败！错误信息：%1").arg(QString(process.readAllStandardError())));
        return;
    }

    // 查找实际生成的HTML文件
    QString expectedHtmlPath = QFileInfo(docxPath).absolutePath() + "/" +
                               QFileInfo(docxPath).baseName() + ".html";

    if (!QFileInfo(expectedHtmlPath).exists()) {
        QMessageBox::warning(this, "错误", "找不到生成的HTML文件！");
        return;
    }

    // 处理HTML内容
    QString content = htmlToText(expectedHtmlPath);
    ui->showText->setPlainText(content);

    // 可以选择删除临时HTML文件
    // QFile::remove(expectedHtmlPath);
}


void MainWindow::on_showButton_clicked()
{
    emit(showtime(show_num));
    //this->hide();
}

void MainWindow::show_which_know(int num)
{
    if(num == 1){
        ShowKnowledge *skl = new ShowKnowledge;
        connect(skl,&ShowKnowledge::close,this,&MainWindow::show);
        skl->show();
        this->hide();
    }else if(num == 2){
        SJF *sjf = new SJF;
        connect(sjf,&SJF::close,this,&MainWindow::show);
        sjf->show();
        this->hide();
    }else if(num == 6){
        DiskWindow *diskwindow = new DiskWindow;
        connect(diskwindow,&DiskWindow::close,this,&MainWindow::show);
        diskwindow->show();
        this->hide();
    } else {
        return;
    }
}

void MainWindow::readOSJsonFile(const QString &filePath)
{
    QFile file(filePath);
    qDebug()<<file.fileName();
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QMessageBox::warning(this,"警告","JSON文件无法打开");
        return;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(),&parseError);
    file.close();

    if(parseError.error != QJsonParseError::NoError){
        QMessageBox::warning(this,"警告","JSON解析错误");
        return;
    }
    QJsonArray tmp = doc.array();
    rootObj = tmp.constBegin()->toObject();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    qDebug()<<"--------------------关闭啦-------------------------";
    if (!currentUser.isEmpty()) {
        qDebug()<<"--------------------开始更新-------------------------";
        // 连接到学习记录数据库
        QSqlDatabase study_db = QSqlDatabase::addDatabase("QSQLITE", "study_close_connection");
        study_db.setDatabaseName("StudyRecords.db");

        if (!study_db.open()) {
            qDebug() << "无法打开数据库:" << study_db.lastError();
            event->accept();
            return;
        }

        // 更新学习记录
        QSqlQuery query(study_db);
        query.prepare(
            "UPDATE UserStudyRecords SET "
            "study_time = study_time + :time, "
            "study_count = study_count + 1 "
            "WHERE account = :user"
            );
        query.bindValue(":time", totalSeconds);
        query.bindValue(":user", currentUser);

        if (!query.exec()) {
            qDebug() << "更新失败:" << query.lastError();
        }

        study_db.close();
    }
    event->accept();
}

QString MainWindow::htmlToText(const QString &filePath) {
    // 读取HTML文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return QString();
    }

    // 使用QTextStream读取内容（自动处理编码）
    QTextStream in(&file);
    //in.setEncoding("UTF-8");  // 根据文件实际编码调整
    QString htmlContent = in.readAll();
    file.close();

    // 创建QTextDocument并加载HTML
    QTextDocument textDoc;
    textDoc.setHtml(htmlContent);

    // 转换为纯文本
    return textDoc.toPlainText();
}

void MainWindow::setCurrentUser(const QString &user) {
    currentUser = user;
    qDebug()<<userLabel->text();
    userLabel->setText(user);
    //statusBar()->addPermanentWidget(userLabel);
    qDebug() << "当前用户设置为:" << currentUser;
    qDebug()<<userLabel->text();

    QTimer::singleShot(0, this, [this](){
        updateRecommendations();
    });

    emit(gengxin());
}

void MainWindow::openword() {
    QString docxPath = word_add;
    QProcess process;
    process.setProgram("C:/Program Files/LibreOffice/program/soffice.exe");
    process.setArguments({
        "--headless", //无图形界面模式
        "--convert-to", "html", //目标格式
        "--outdir", QFileInfo(docxPath).absolutePath(), //输出目录
        docxPath //输入文件
    });
    process.start();

    if (process.waitForFinished(30000)) {
        QString htmlPath = QFileInfo(docxPath).absolutePath() + "/" +
                           QFileInfo(docxPath).baseName() + ".html";
        QString content = htmlToText(htmlPath);
        ui->showText->setPlainText(content);
        // 调用系统浏览器打开 HTML
        bool success = QDesktopServices::openUrl(QUrl::fromLocalFile(htmlPath));
        if (!success) {
            QMessageBox::warning(nullptr, "错误", "无法打开浏览器！");
        }
    } else {
        QMessageBox::warning(nullptr, "错误", "转换 Word 文件超时！");
    }
}


//------------------------画像推荐算法--------------------------------

// // mainwindow.cpp 推荐系统完整实现
// void MainWindow::updateRecommendations()
// {
//     qDebug() << "=== 开始生成推荐 ===";

//     QSqlDatabase db = QSqlDatabase::database("click_connection");
//     if (!db.isOpen()) {
//         qDebug() << "数据库连接失败：" << db.lastError();
//         return;
//     }

//     // 获取近30天加权点击数据
//     QSqlQuery query(db);
//     QString sql = QString(
//                       "SELECT chapter, section, knowledge, "
//                       "SUM(click_count) as clicks, "
//                       "AVG(JULIANDAY('now') - JULIANDAY(click_time)) as recency "
//                       "FROM click_records "
//                       "WHERE account = '%1' "
//                       "GROUP BY chapter, section, knowledge "
//                       "HAVING clicks > 0 "
//                       "ORDER BY (0.6 * EXP(-recency/30.0) + 0.4 * (1 - 1.0/(1+clicks))) DESC "
//                       "LIMIT 5").arg(currentUser);

//     if (!query.exec(sql)) {
//         qDebug() << "推荐查询失败：" << query.lastError();
//         return;
//     }

//     QList<QString> recommendations;
//     while (query.next()) {
//         QString path = QString("%1/%2/%3")
//         .arg(query.value(0).toString().trimmed())
//             .arg(query.value(1).toString().trimmed())
//             .arg(query.value(2).toString().trimmed());
//         recommendations.append(path);
//         qDebug() << "推荐项：" << path;
//     }

//     updateRecommendationDisplay(recommendations);
//     highlightRecommendations(recommendations);
// }

// 新增显示占位内容的方法
void MainWindow::showPlaceholderRecommendation()
{
    ui->recommendationList->clear();
    QListWidgetItem *item = new QListWidgetItem("🔍 登录后查看个性化推荐");
    item->setTextAlignment(Qt::AlignCenter); //文本居中
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable); //禁用选中状态
    item->setForeground(QColor(150, 150, 150)); //文本颜色为灰色
    ui->recommendationList->addItem(item);
}

void MainWindow::updateRecommendationDisplay(const QList<QString>& recommendations)
{
    ui->recommendationList->clear();

    if (recommendations.isEmpty()) {
        QListWidgetItem* placeholder = new QListWidgetItem("💡 继续探索更多知识点，推荐列表将自动生成");
        placeholder->setTextAlignment(Qt::AlignCenter);
        placeholder->setFlags(placeholder->flags() & ~Qt::ItemIsSelectable);
        placeholder->setForeground(QColor(150, 150, 150));
        ui->recommendationList->addItem(placeholder);
        return;
    }

    foreach (const QString& path, recommendations) {
        QStringList parts = path.split('/');
        if (parts.size() == 3) {
            QListWidgetItem* item = new QListWidgetItem(
                QString("⭐ %1 ➔ %2 ➔ %3").arg(parts[0], parts[1], parts[2]));
            item->setData(Qt::UserRole, path);
            ui->recommendationList->addItem(item);
        }
    }
}

void MainWindow::highlightRecommendations(const QList<QString>& recommendations)
{
    clearAllHighlights();
    QStandardItem* root = model.invisibleRootItem();

    foreach (const QString& path, recommendations) {
        QStringList parts = path.split('/');
        if (parts.size() == 3) {
            for (int i = 0; i < root->rowCount(); ++i) {
                QStandardItem* chapterItem = root->child(i);
                QString cleanChapter = chapterItem->text()
                                           .replace(QRegularExpression("[ 　第章]"), "");

                if (cleanChapter == parts[0].trimmed()) {
                    traverseAndHighlight(chapterItem, parts[0], parts[1], parts[2]);
                }
            }
        }
    }
}

void MainWindow::traverseAndHighlight(QStandardItem* parent,
                                      const QString& targetChapter,
                                      const QString& targetSection,
                                      const QString& targetKnowledge)
{
    for (int i = 0; i < parent->rowCount(); ++i) {
        QStandardItem* chapterItem = parent->child(i);
        QString currentChapter = chapterItem->text(); // 直接使用原始名称

        if (currentChapter == targetChapter) {
            ui->treeView->expand(chapterItem->index());

            for (int j = 0; j < chapterItem->rowCount(); ++j) {
                QStandardItem* sectionItem = chapterItem->child(j);
                QString currentSection = sectionItem->text(); // 直接使用原始名称

                if (currentSection == targetSection) {
                    ui->treeView->expand(sectionItem->index());

                    for (int k = 0; k < sectionItem->rowCount(); ++k) {
                        QStandardItem* knowledgeItem = sectionItem->child(k);
                        QString currentKnowledge = knowledgeItem->text(); // 直接使用原始名称

                        if (currentKnowledge == targetKnowledge) {
                            knowledgeItem->setBackground(QColor(255, 255, 220)); //浅黄色（米色）
                            animateHighlight(knowledgeItem);
                            return;
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::updateRecommendations()
{
    qDebug() << "=== 开始生成推荐 ===";

    if(currentUser.isEmpty()) {
        showPlaceholderRecommendation();
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("click_connection");
    if (!db.isOpen()) {
        qDebug() << "数据库连接失败：" << db.lastError();
        return;
    }

    // 修正时间计算，使用datetime('now')获取当前时间
    QString sql = QString(
                      "SELECT chapter, section, knowledge, "
                      "SUM(click_count) as clicks, " //统计点击次数
                      "AVG(JULIANDAY(datetime('now')) - JULIANDAY(click_time)) as recency " //计算平均间隔天数，数值越大表示时间越久远
                      "FROM click_records "
                      "WHERE account = '%1' " //仅查询当前用户的数据
                      "GROUP BY chapter, section, knowledge " //按知识点分组聚合点击知识点
                      "HAVING clicks > 0 " //排除0点击知识点
                      "ORDER BY (0.6 * EXP(-recency/30.0) + 0.4 * (1 - 1.0/(1+clicks))) DESC " //时间衰减因子，30天内点击权重高，超过则快速下降；指数衰减；点击次数越多，值越接近1（饱和曲线避免少数高点击垄断）；0.6的近期点击，时效性优先，0.4的历史兴趣，稳定性。
                      "LIMIT 5").arg(currentUser); //限制五条推荐。

    QSqlQuery query(db);
    if (!query.exec(sql)) {
        qDebug() << "推荐查询失败：" << query.lastError();
        return;
    }

    QList<QString> recommendations;
    while (query.next()) { // 遍历查询结果
        QString chapter = query.value(0).toString().trimmed(); //去除空格
        QString section = query.value(1).toString().trimmed();
        QString knowledge = query.value(2).toString().trimmed();
        QString path = QString("%1/%2/%3").arg(chapter, section, knowledge); //拼接路径
        recommendations.append(path); //加入推荐项
        qDebug() << "推荐项：" << chapter << section << knowledge;
    }

    updateRecommendationDisplay(recommendations);
    highlightRecommendations(recommendations);

    if(recommendations.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("✨ 试试点击任意知识点开始学习旅程");
        item->setTextAlignment(Qt::AlignCenter);
        item->setForeground(QColor(180, 180, 180));
        ui->recommendationList->addItem(item);
    }
}
void MainWindow::animateHighlight(QStandardItem* item)
{
    // 清理旧动画
    if (activeAnimations.contains(item)) {
        QTimer* oldTimer = activeAnimations.take(item);
        oldTimer->stop();
        oldTimer->deleteLater();
    }

    // 动画参数
    int alpha = 0; //透明度增量
    bool increasing = true; //控制增减方向
    const QColor baseColor(255, 255, 200); //浅黄色

    QTimer* timer = new QTimer(this);
    timer->setInterval(50); // 20 FPS

    connect(timer, &QTimer::timeout, [=]() mutable {
        // 呼吸灯效果计算
        alpha += increasing ? 8 : -8; //步长为8，控制方向
        if (alpha >= 120 || alpha <= 0) increasing = !increasing; //反转方向

        QColor dynamicColor = QColor(
            baseColor.red() - alpha/2, //红递减
            baseColor.green() - alpha/2, //绿递增
            baseColor.blue() //蓝不变
            );
        item->setBackground(dynamicColor);
    });

    // 自动停止
    QTimer::singleShot(3000, [=]() {
        timer->stop();
        item->setBackground(baseColor); //恢复初始颜色
        activeAnimations.remove(item); //容器内移除记录
        timer->deleteLater(); //安全删除定时器
    });

    activeAnimations.insert(item, timer); //记录并启动动画
    timer->start();
}

void MainWindow::clearAllHighlights()
{
    // 递归清除所有颜色
    std::function<void(QStandardItem*)> clearColor = [&](QStandardItem* item) {
        item->setBackground(QBrush());
        for (int i = 0; i < item->rowCount(); ++i) {
            clearColor(item->child(i));
        }
    };
    clearColor(model.invisibleRootItem());
}

void MainWindow::onRecommendationClicked(QListWidgetItem *item) {
    if (!item) return;

    // 获取存储的路径数据
    QString path = item->data(Qt::UserRole).toString();
    QStringList parts = path.split('/');
    if (parts.size() != 3) {
        qDebug() << "无效的推荐项路径:" << path;
        return;
    }

    QString chapter = parts[0];
    QString section = parts[1];
    QString knowledge = parts[2];

    // 在树状模型中查找对应的知识点节点
    QModelIndex foundIndex;
    QModelIndex rootIndex = model.invisibleRootItem()->index();

    // 遍历所有章节
    for (int i = 0; i < model.rowCount(rootIndex); ++i) {
        QModelIndex chapIdx = model.index(i, 0, rootIndex);
        if (chapIdx.data().toString() == chapter) {
            // 遍历当前章节的所有小节
            for (int j = 0; j < model.rowCount(chapIdx); ++j) {
                QModelIndex secIdx = model.index(j, 0, chapIdx);
                if (secIdx.data().toString() == section) {
                    // 遍历当前小节的所有知识点
                    for (int k = 0; k < model.rowCount(secIdx); ++k) {
                        QModelIndex knowIdx = model.index(k, 0, secIdx);
                        if (knowIdx.data().toString() == knowledge) {
                            foundIndex = knowIdx;
                            break;
                        }
                    }
                    if (foundIndex.isValid()) break;
                }
            }
            if (foundIndex.isValid()) break;
        }
    }

    if (foundIndex.isValid()) {
        // 触发树状视图的点击处理
        ui->treeView->setCurrentIndex(foundIndex);
        ui->treeView->scrollTo(foundIndex, QAbstractItemView::EnsureVisible);
        onTreeViewClicked(foundIndex); // 直接调用点击处理函数
    } else {
        qDebug() << "未找到对应的知识点节点:" << chapter << section << knowledge;
    }
}

// void MainWindow::getinit()
// {
//     DiskWindow *dk = new DiskWindow;
//     ShowKnowledge *skl = new ShowKnowledge;
//     SJF *sjf = new SJF;
//     connect(dk,&DiskWindow::close,this,&MainWindow::show);
//     connect(skl,&ShowKnowledge::close,this,&MainWindow::show);
//     connect(sjf,&SJF::close,this,&MainWindow::show);
// }

// 辅助函数：构建文档路径
QString MainWindow::buildDocumentPath(const QString &nodeText)
{
    // 获取应用程序目录并向上三级
    QString exePath = QCoreApplication::applicationDirPath();
    QDir dir(exePath);
    for (int i = 0; i < 3; ++i) {
        if (!dir.cdUp()) {
            qWarning() << "Failed to cdUp from:" << dir.absolutePath();
            return QString();
        }
    }

    // 定义菜单项结构
    struct MenuItem {
        QString name;
        QString subdir;
        int showNum;
    };

    // 菜单项配置
    static const QVector<MenuItem> menuItems = {
        {"先来先服务算法", "作业调度", 1},
        {"短作业优先算法", "作业调度", 2},
        {"目录结构", "文件管理", 3},
        {"索引分配", "文件管理", 4},
        {"文件控制块", "文件管理", 5},
        {"SCAN算法", "文件管理", 6}
    };

    // 查找匹配的菜单项
    for (const auto &item : menuItems) {
        if (nodeText == item.name) {
            show_num = item.showNum;
            return QDir::cleanPath(dir.absolutePath() +
                                   "/info/" +
                                   item.subdir +
                                   "/" +
                                   nodeText + ".docx");
        }
    }

    // 默认情况
    show_num = 7;
    return QDir::cleanPath(dir.absolutePath() +
                           "/info/进程的描述与控制/" +
                           nodeText + ".docx");
}
