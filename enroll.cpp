#include "enroll.h"
#include "ui_enroll.h"

Enroll::Enroll(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Enroll)
{
    ui->setupUi(this);
    this->setStyleSheet("background-color: #F5F5DC; color: #333333;");
    connect(ui->enrollButton,SIGNAL(clicked()),this,SLOT(push_button_regiter()));
    connect(ui->backButton,&QPushButton::clicked,[this]{
        this->close();
        emit(cancel());
    });
    setWindowState(Qt::WindowMaximized);
}

Enroll::~Enroll()
{
    delete ui;
}


void Enroll::push_button_regiter(){
    account = ui->userLine->text();
    password = ui->passLine->text();
    if(password != ui->repassLine->text()){
        QMessageBox::warning(this,"提示","两次输入的密码不相同");
        return;
    }

    if(QSqlDatabase::contains("qt_sql_default_connection")){
        reg_db = QSqlDatabase::database("qt_sql_default_connection");
    }else{
        reg_db = QSqlDatabase::addDatabase("QSQLITE");
        reg_db.setDatabaseName("Account.db");
    }

    reg_db.open();

    QString str = QString("select * from Account where user = '%1'").arg(account);
    QSqlQuery sqlQuery;
    sqlQuery = QSqlQuery(reg_db);

    if(!sqlQuery.exec(str)){
        QMessageBox::warning(this,"提示","账号已被注册");
    }else{
        add_sql(account,password);

        // 连接到学习记录数据库
        QSqlDatabase study_db = QSqlDatabase::addDatabase("QSQLITE", "study_connection");
        study_db.setDatabaseName("StudyRecords.db");
        if (!study_db.open()) {
            QMessageBox::warning(this, "错误", "无法打开学习记录数据库: " + study_db.lastError().text());
            study_db.close();
            emit(cancel());
            this->close();
            return;
        }

        QSqlQuery study_query(study_db);
        // 创建表（如果不存在）
        QString createTable = "CREATE TABLE IF NOT EXISTS UserStudyRecords ("
                              "account TEXT PRIMARY KEY, "
                              "study_time INTEGER DEFAULT 0, "
                              "study_count INTEGER DEFAULT 0)";
        if (!study_query.exec(createTable)) {
            QMessageBox::warning(this, "错误", "创建学习记录表失败: " + study_query.lastError().text());
            study_db.close();
            emit(cancel());
            this->close();
            return;
        }

        // 插入初始学习记录
        study_query.prepare("INSERT INTO UserStudyRecords (account, study_time, study_count) "
                            "VALUES (:account, 0, 0)");
        study_query.bindValue(":account", account);
        if (!study_query.exec()) {
            QMessageBox::warning(this, "错误", "初始化学习记录失败: " + study_query.lastError().text());
            study_db.close();
            emit(cancel());
            this->close();
            return;
        }

        study_db.close();

        QMessageBox::warning(this,"提示","注册成功！");
        emit(cancel());
        emit(enrollStu(account));
        this->close();
    }

    reg_db.close();
}
void Enroll::add_sql(QString account,QString password)
{
    //reg_db.open();
    QSqlQuery sqlQuery;
    sqlQuery = QSqlQuery(reg_db);

    QString str = QString("insert into Account values('%1','%2')").arg(account).arg(password);
    qDebug()<<str;

    // if(!reg_db.open()){
    //     qDebug()<<"在add里打开数据库失败！！！";
    // }else{
    //     qDebug()<<"在add中打开数据库成功！！！";
    // }
    //qDebug()<<"要运行的代码是:"<<sqlQuery.exec(str);
    if(!sqlQuery.exec(str)){
        qDebug()<<"插入数据失败"<<sqlQuery.lastError();
    }else{
        qDebug()<<"插入数据成功";
    }
    reg_db.close();
}
