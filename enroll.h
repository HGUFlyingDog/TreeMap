#ifndef ENROLL_H
#define ENROLL_H

#include <QWidget>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QPushButton>

namespace Ui {
class Enroll;
}

class Enroll : public QWidget
{
    Q_OBJECT

public:
    explicit Enroll(QWidget *parent = nullptr);
    ~Enroll();

    void add_sql(QString account,QString password);

signals:
    void reg(); //登录信号
    void cancel(); //关闭信号
    void enrollStu(QString name); //注册成功信号

public slots:
    void push_button_regiter();

private:
    Ui::Enroll *ui;

    QString account;
    QString password;

    QSqlDatabase reg_db;
};

#endif // ENROLL_H
