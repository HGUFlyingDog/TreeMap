#include "logwidget.h"
#include "ui_logwidget.h"

LogWidget::LogWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogWidget)
{
    ui->setupUi(this);
    //this->setStyleSheet("background-color: #F5F5DC; color: #333333;");
    this->setWindowTitle("登录界面");
    setWindowState(Qt::WindowMaximized);

    init_sql(); // 先初始化相关数据库
    //QSqlQuery sqlQuery;

    // QString clear_str = "drop table Account";
    // sqlQuery.prepare(clear_str);
    // if(!sqlQuery.exec()){
    //     qDebug()<<"delete_table true";
    // }else{
    //     qDebug()<<"table cleared";
    // }

    //ui->loginButton->setFocus();
    //ui->loginButton->setDefault(true);


    connect(ui->passLineEdit,SIGNAL(returnPressed()),this,SLOT(btn_log_clicked())); //enter进入
    connect(ui->loginButton,SIGNAL(clicked()),this,SLOT(btn_log_clicked()));
    connect(ui->cancelButton,SIGNAL(clicked()),this,SLOT(cancel_clicked()));
    connect(ui->enrollButton,SIGNAL(clicked()),this,SLOT(btn_register_clicked()));
    connect(ui->displayRadioButton,SIGNAL(clicked()),this,SLOT(passwordButton_clicked()));

    connect(this,SIGNAL(close_window()),this,SLOT(close()));

    ui->passLineEdit->setEchoMode(QLineEdit::Password); //输入时隐藏密码
    // m_username = "taffy";
    // m_password = "guanzhumiao";

    login_db.close();//关闭数据库

    //this->show();
}

LogWidget::~LogWidget()
{
    delete ui;
}


void LogWidget::btn_log_clicked()
{
    QString username = ui->accountLineEdit->text();
    QString password = ui->passLineEdit->text();

    qDebug()<<username<<" "<<password<<"\n";


    if(password == query_sql(username)){
        //发送登录信号
        //emit(login(username));
        // emit(login());
        //登陆完直接关闭窗口
        emit(close_window());
        emit(log_stu(username));
        mwd = new MainWindow;
        mwd->show();
        mwd->setCurrentUser(username);
    }else{
        QMessageBox::warning(this,"警告","账号密码不匹配！");
    }
}

void LogWidget::cancel_clicked()
{
    //直接关闭窗口
    emit(close_window());
}

void LogWidget::btn_register_clicked() //注册
{
    // //init_sql(); // 先初始化相关数据库
    // add_sql("dongxuelian","hanjian");
    // // this->close();
    // // MainWindow mainwindow = new MainWindow;
    // // mainwindow.show();
    // emit(regsiter_peo());

    this->hide();
    Enroll *regite = new Enroll;
    connect(regite,SIGNAL(cancel()),this,SLOT(show()));
    regite->setAttribute(Qt::WA_DeleteOnClose); //关闭即销毁
    regite->show();
}

void LogWidget::passwordButton_clicked()
{
    count = (count + 1) % 2;
    if(count % 2) ui->passLineEdit->setEchoMode(QLineEdit::Normal);
    else ui->passLineEdit->setEchoMode(QLineEdit::Password);
}


void LogWidget::on_deleteButton_clicked()
{
    QString account = ui->accountLineEdit->text();
    delete_sql(account);
}


//-------------------------------------------------------------------------------- 数据库操作

void LogWidget::init_sql()
{
    if(QSqlDatabase::contains("qt_sql_default_connection")){ //建立数据库，qt_...默认连接名
        login_db = QSqlDatabase::database("qt_sql_default_connection");
    }else{
        login_db = QSqlDatabase::addDatabase("QSQLITE");
        login_db.setDatabaseName("Account.db");
        login_db.setUserName("jianghao");
        login_db.setPassword("20030811");
    }

    // if(!login_db.open()){
    //     qDebug()<<"ERROR: FAILED TO CONNECT DATABASE."<<login_db.lastError();
    // }

    // login_db = QSqlDatabase::addDatabase("QSQLITE"); //链接数据库
    // login_db.setDatabaseName("Account.db"); //打开名字为account的数据库

    //create_sql();
    if(!login_db.open()){
        qDebug()<<"打开数据库失败 "<<login_db.lastError().text();
        //create_sql();
    }else{
        qDebug()<<"打开数据库成功";
        create_sql();
    }
}

void LogWidget::create_sql()
{
    login_db.open();
    QSqlQuery sqlQuery;
    sqlQuery = QSqlQuery(login_db);
    // login_db.open();
    // sqlQuery = QSqlQuery(login_db);
    QString str = "";
    str = "create table Account ("
          "user VERCHAR(40) NOT NULL primary key,"
          "password VERCHAR(40) NOT NULL)";
    if(!sqlQuery.exec(str)){
        qDebug()<<"创建失败"<<sqlQuery.lastError();
    }else {
        qDebug()<<"创建成功";
    }
    login_db.close();
}

QString LogWidget::query_sql(QString account)
{
    QString password = "23213";
    //-----
    QString str;
    QSqlQuery sqlQuery;
    login_db.open();
    sqlQuery = QSqlQuery(login_db);
    str = QString("select * from Account where user = '%1'").arg(account);

    if(!sqlQuery.exec(str)){
        qDebug()<<"查询失败"<<sqlQuery.lastError();
        //查询失败操作
    }else if(sqlQuery.first()){
        qDebug()<<"查询成功";
        password = sqlQuery.value(0).toString();
        qDebug()<<password;
        password = sqlQuery.value(1).toString();
        qDebug()<<password;
    }else{
        qDebug()<<"不存在该账户";
    }
    //-----

    // qDebug()<<"\n\n现在要对表内内容进行打印\n\n";

    // while(sqlQuery.next()){
    //     qDebug()<<"打印： "<<sqlQuery.value(0).toString()<<" "<<sqlQuery.value(1).toString();
    //     // if(account == sqlQuery.value(0).toString()){
    //     //     password = sqlQuery.value(1).toString();
    //     //     break;
    //     // }
    // }

    // qDebug()<<"-------------------";
    login_db.close();
    return password;
    // while(sqlQuery.next()){
    //     QString s1 = sqlQuery.value(0).toString();
    //     QString s2 = sqlQuery.value(1).toString();
    //     qDebug()<<s1<<" "<<s2;
    //     break;
    // }

}

void LogWidget::delete_sql(QString account)
{
    login_db.open();
    QSqlQuery sqlQuery;
    sqlQuery = QSqlQuery(login_db);
    QString str = QString("delete from Account where user = '%1'").arg(account);
    if(!sqlQuery.exec(str)){
        qDebug()<<"删除失败";
    }else{
        qDebug()<<"删除成功";
    }
    login_db.close();
}

void LogWidget::add_sql(QString account,QString password)
{
    login_db.open();
    QSqlQuery sqlQuery;
    sqlQuery = QSqlQuery(login_db);

    //查询
    if(account == sqlQuery.value(0).toString()) {
        qDebug()<<"当前插入信息已存在";
    }

    QString str = QString("insert into Account values('%1','%2')").arg(account).arg(password);
    qDebug()<<str;

    if(!login_db.open()){
        qDebug()<<"在add里打开数据库失败！！！";
    }else{
        qDebug()<<"在add中打开数据库成功！！！";
    }
    //qDebug()<<"要运行的代码是:"<<sqlQuery.exec(str);
    if(!sqlQuery.exec(str)){
        qDebug()<<"插入数据失败"<<sqlQuery.lastError();
    }else{
        qDebug()<<"插入数据成功";
    }
    login_db.close();
}

// void LogWidget::add_sql(QString account,QString password)
// {
//     sqlQuery.prepare("insert into user (account,password) values (:account,:password");
//     sqlQuery.bindValue(":account",account);
//     sqlQuery.bindValue(":password",password);
//     if(!sqlQuery.exec()){
//         qDebug()<<"添加数据失败"<<sqlQuery.lastQuery();
//         return;
//     }else{
//         qDebug()<<"添加数据成功!";
//     }
// }

void LogWidget::modify_sql(QString account,QString password)
{
    login_db.open();
    QSqlQuery sqlQuery;
    sqlQuery = QSqlQuery(login_db);
    QString str = QString("update account set password = '%1' where user = '%2'").arg(password).arg(account);
    qDebug()<<str;
    if(!sqlQuery.exec(str)){
        qDebug()<<"修改数据失败";
    }else{
        qDebug()<<"修改数据成功";
    }
    login_db.close();
}
