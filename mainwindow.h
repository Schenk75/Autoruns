#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <stdio.h>
#include <direct.h>
#include <QProcess>
#include <QSettings>
#include <QDebug>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QDateTime>
#include <QTextCodec>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void init_logon(QTableWidget *table);
    void init_sd(QTableWidget *service_table, QTableWidget *driver_table);
    void init_schedule(QTableWidget *table);
    void init_dlls(QTableWidget *table);

private slots:
    void hide_windows();
    void hide_microsoft();
};
#endif // MAINWINDOW_H
