#include "logwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 在显示登录窗口时（例如main.cpp）
    LogWidget *login = new LogWidget;
    MainWindow *mainWindow = new MainWindow;

    // 连接登录成功信号
    QObject::connect(login, &LogWidget::log_stu, mainWindow, &MainWindow::setCurrentUser);

    // 连接注册成功信号（在注册窗口显示时）
    Enroll *enroll = new Enroll;
    QObject::connect(enroll, &Enroll::enrollStu, mainWindow, &MainWindow::setCurrentUser);
    login->show();
    return a.exec();
}
