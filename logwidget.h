#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include "enroll.h"
#include "mainwindow.h"

namespace Ui {
class LogWidget;
}

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget *parent = nullptr);
    ~LogWidget();

    void create_sql(); //创建数据库
    void init_sql(); //初始化数据库

    //数据库的操作
    QString query_sql(QString account); //查询数据库
    void delete_sql(QString account); //删除表项
    void modify_sql(QString account,QString password); //修改数据
    void add_sql(QString account,QString password); //添加数据

signals:
    //void login(QString account);//登录主界面信号 信号可以携带参数，并且把参数传递给槽函数
    void login();
    void close_window();//关闭登陆界面信号
    void regsiter_peo();//注册
    void password_hide();//隐藏密码
    void log_stu(QString str);

public slots:
    void btn_register_clicked();//注册按下
    void btn_log_clicked();//按下登录后触发的事件
    void cancel_clicked();//取消
    void passwordButton_clicked();//隐藏密码

private slots:
    void on_deleteButton_clicked();


private:
    Ui::LogWidget *ui;

    QString m_username;
    QString m_password;

    QSqlDatabase login_db;
    Enroll *reg;
    MainWindow *mwd;
    //注册

    int count = 0;
};

#endif // LOGWIDGET_H
