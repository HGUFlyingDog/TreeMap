#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QTreeView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTextStream>
#include <QFile>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QTextEdit>
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void get_tree();
    void onTreeViewClicked(const QModelIndex &index);
    void readOSJsonFile(const QString &filePath);
    QString getShortPath(const QString& longPath);
    void closeEvent(QCloseEvent *event) override;
    void convertAndOpenInBrowser(const QString &docxPath);
    QString htmlToText(const QString &filePath);
    void updateRecommendations();
    QList<QString> getTopRecommendations(const QMap<QString, float>& scores);
    void updateRecommendationDisplay(const QList<QString>& recommendations);
    void highlightRecommendations(const QList<QString>& recommendations);
    void clearAllHighlights();


    void setCurrentUser(const QString &user);

signals:
    void log();
    void showtime(int num);
    void gengxin();

private slots:
    void relogin();

    void on_showButton_clicked();
    void show_which_know(int num);
    void openword();
    void animateHighlight(QStandardItem* item);
    void onRecommendationClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QStandardItemModel model;
    QList<QStandardItem *> list;
    QString show_String;
    QJsonObject rootObj;

    QLabel* timeLabel;
    QLabel* userLabel;
    QTimer* timer;
    qint64 totalSeconds = 0;
    QString currentUser;

    QString word_add;

    int show_num;
    QMap<QStandardItem*, QTimer*> activeAnimations;
    void traverseAndHighlight(QStandardItem* parent,const QString& targetChapter,const QString& targetSection,const QString& targetKnowledge);

    bool recommendationInitialized = false; // 推荐系统初始化标志

    void showPlaceholderRecommendation();
    void getfile(QStringList &path);
    void getinit();
};
#endif // MAINWINDOW_H
