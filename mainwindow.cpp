#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());

//    init_logon(ui->tableWidget);
//    init_sd(ui->tableWidget_2, ui->tableWidget_3);
//    init_schedule(ui->tableWidget_4);
    init_dlls(ui->tableWidget_5);

    ui->tabWidget->setCurrentIndex(0);  // 初始进入Logon界面
    connect(ui->action_hide_windows, SIGNAL(toggled(bool)), this, SLOT(hide_windows()));
    connect(ui->action_hide_microsoft, SIGNAL(toggled(bool)), this, SLOT(hide_microsoft()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 初始化logon表格，参数(logon表)
void MainWindow::init_logon(QTableWidget *table){
    QStringList Logon_list;
    Logon_list.append("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    Logon_list.append("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
    Logon_list.append("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    Logon_list.append("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
    Logon_list.append("HKLM\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
    Logon_list.append("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
    Logon_list.append("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer\\Run");
    Logon_list.append("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    Logon_list.append("HKCU\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx");
    Logon_list.append("HKCU\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run");
//    Logon_list.append();

    int i, Logon_num;
    Logon_num = Logon_list.length();  // 注册表路径的数量
    QString temp, root_key, reg_path;
    QTableWidgetItem *item;
    char buf[256];
    _getcwd(buf, sizeof(buf));  // 获取当前绝对路径
//    qDebug("current path: %s", buf);
    int column_width[5] = {231, 200, 300, 410, 170};  // 定义每列的宽度
    int row_num = 0;      // 表格行数
    for (i=0; i<=4; i++)
        table->setColumnWidth(i, column_width[i]);
    table->resizeRowsToContents();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    table->setSelectionMode(QAbstractItemView::SingleSelection);    //只能单选
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);   //禁止修改
    table->verticalHeader()->setVisible(false);   //隐藏列表头
    table->setShowGrid(false);    // 隐藏网格线

    // 设置样式表
    table->setStyleSheet("border:none; selection-color:black; selection-background-color:#E6E6FA;");

    // 添加表项
    for (i=0; i<Logon_num; i++){
        root_key = Logon_list[i].mid(0, 4);
        reg_path = Logon_list[i].mid(5);
//        qDebug() << Logon_list[i];
//        qDebug() << root_key << reg_path;
//        qDebug() << row_num << Logon_list[i];
        table->setRowCount(row_num+1);
        item = new QTableWidgetItem(Logon_list[i].toLocal8Bit().data());
        table->setItem(row_num, 0, item);
        table->setSpan(row_num, 0, 1, 5);
        item->setBackgroundColor(QColor(135,206,250));
        row_num += 1;

        QProcess p;
        QString command = QString(buf) + QString("\\logon.exe");
        QStringList args;
        args.append(root_key);
        args.append(reg_path);
        p.start(command, args);
        p.waitForFinished();
        QString result = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
        result.replace("\r", "");

        // 没有子键的注册表路径不予显示
        if (result == ""){
            table->removeRow(row_num-1);
            row_num -= 1;
            continue;
        }

//        qDebug() << result;
        QStringList result_list = result.split("\n");
//        qDebug() << result_list;

        foreach (result, result_list){
//            qDebug() << result;
            QStringList t = result.split("$#&");
            QString name = t[0].trimmed();
            if (name != ""){
                table->setRowCount(row_num+1);
                QString description = t[1].trimmed();
                if (description == "")
                    description = name;
                QString image_path = t[2].trimmed();
                qDebug("%s === %s", name.toStdString().data(), image_path.toStdString().data());

                // 获取程序签名信息
                QProcess p;
                QString command = QString(buf) + QString("\\sigcheck64.exe");
    //            qDebug("command: %s", command.toLocal8Bit().data());
                QStringList args;
                args.append(image_path);
                p.start(command, args);
                p.waitForFinished();
                temp = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    //            qDebug("%s", result.toLocal8Bit().data());
                temp.replace("\r", "");
                temp.replace("\t", "");
                t = temp.split("\n", QString::SkipEmptyParts);
//                qDebug() << t << "\n";

                // 找到可执行文件，则正常显示信息
                if (t[3] != "No matching files were found."){
                    // 显示启动项名称
                    item = new QTableWidgetItem(name);
                    QFileInfo fi(image_path.toLocal8Bit().data());
                    QFileIconProvider ficon;
                    QIcon icon = ficon.icon(fi);    // 获取图标
                    item->setIcon(icon);
                    table->setItem(row_num, 0, item);
                    // 显示启动项描述
                    item = new QTableWidgetItem(description);
                    table->setItem(row_num, 1, item);
                    // 显示可执行文件目录
                    item = new QTableWidgetItem(image_path);
                    table->setItem(row_num, 3, item);
                    // 显示时间戳(最近修改时间)
                    temp = fi.lastModified().toString("yyyy-MM-dd hh:mm:ss");
                    item = new QTableWidgetItem(temp);
                    table->setItem(row_num, 4, item);
                    // 显示证书信息
                    temp = t.at(6).mid(10);
                    if (t.at(4).contains("Unsigned")){    // 未认证的程序标注
                        temp = QString("(Not Verified)") + temp;
    //                    qDebug() << temp;
                        item = new QTableWidgetItem(temp);
                        table->setItem(row_num, 2, item);
                        for (i=0; i<5; i++){
                            table->item(row_num, i)->setBackgroundColor(QColor(255,182,193));
                        }
                    }
                    else{
                        temp = QString("(Verified)") + temp;
                        item = new QTableWidgetItem(temp);
                        table->setItem(row_num, 2, item);
                    }
                }
                // 未找到可执行文件，则标黄显示
                else{
                    // 显示启动项名称
                    item = new QTableWidgetItem(name);
                    table->setItem(row_num, 0, item);
                    // 显示启动项描述
                    temp = QString("");
                    item = new QTableWidgetItem(temp);
                    table->setItem(row_num, 1, item);
                    // 显示可执行文件目录
                    temp = QString("File not found: ") + image_path;
                    item = new QTableWidgetItem(temp);
                    table->setItem(row_num, 3, item);
                    // 显示时间戳(最近修改时间)
                    temp = QString("");
                    item = new QTableWidgetItem(temp);
                    table->setItem(row_num, 4, item);
                    // 显示证书信息
                    item = new QTableWidgetItem(temp);
                    table->setItem(row_num, 2, item);
                    for (i=0; i<5; i++)
                        table->item(row_num, i)->setBackgroundColor(QColor(255,255,0));
                }
                row_num += 1;
            }
        }
    }

}

// 初始化services和drivers表格，参数(services表, drivers表)
void MainWindow::init_sd(QTableWidget *service_table, QTableWidget *driver_table){
    QString Service_path = "HKLM\\SYSTEM\\CurrentControlSet\\Services";
    int i;
    QString temp;
    QTableWidgetItem *item1, *item2, *item3, *item4, *item0;
    char buf[256];
    _getcwd(buf, sizeof(buf));  // 获取当前绝对路径
//    qDebug("current path: %s", buf);

    int column_width[5] = {231, 200, 300, 410, 170};  // 定义每列的宽度
    int row_num_service = 0;      // services表行数
    int row_num_driver = 0;       // drivers表行数
    for (i=0; i<=4; i++)
        service_table->setColumnWidth(i, column_width[i]);
    service_table->resizeRowsToContents();
    service_table->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    service_table->setSelectionMode(QAbstractItemView::SingleSelection);    //只能单选
    service_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   //禁止修改
    service_table->verticalHeader()->setVisible(false);   //隐藏列表头
    service_table->setShowGrid(false);    // 隐藏网格线
    for (i=0; i<=4; i++)
        driver_table->setColumnWidth(i, column_width[i]);
    driver_table->resizeRowsToContents();
    driver_table->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    driver_table->setSelectionMode(QAbstractItemView::SingleSelection);    //只能单选
    driver_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   //禁止修改
    driver_table->verticalHeader()->setVisible(false);   //隐藏列表头
    driver_table->setShowGrid(false);    // 隐藏网格线
    // 设置样式表
    service_table->setStyleSheet("border:none; selection-color:black; selection-background-color:#E6E6FA;");
    driver_table->setStyleSheet("border:none; selection-color:black; selection-background-color:#E6E6FA;");

    service_table->setRowCount(row_num_service+1);
    driver_table->setRowCount(row_num_driver+1);
    item1 = new QTableWidgetItem(Service_path.toLocal8Bit().data());
    service_table->setItem(row_num_service, 0, item1);
    service_table->setSpan(row_num_service, 0, 1, 5);
    item1->setBackgroundColor(QColor(135,206,250));
    item1 = new QTableWidgetItem(Service_path.toLocal8Bit().data());
    driver_table->setItem(row_num_driver, 0, item1);
    driver_table->setSpan(row_num_driver, 0, 1, 5);
    item1->setBackgroundColor(QColor(135,206,250));
    row_num_driver += 1;
    row_num_service += 1;

    QProcess p;
    QString command = QString(buf) + QString("\\ServiceDriver.exe");
    p.start(command);
    p.waitForFinished();
    QString result = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    result.replace("\r", "");
//    qDebug() << result;
    QStringList result_list = result.split("\n");
//    qDebug() << result_list;

    foreach (result, result_list){
//        qDebug() << result;
        QStringList t = result.split("$#&");
        QString type_ = t[0].trimmed();
        QString name = t[1].trimmed();
        QString description = t[2].trimmed();
        if (description == "")
            description = name;
        QString image_path = t[3].trimmed();
        qDebug("%s === %s", name.toLocal8Bit().data(), image_path.toLocal8Bit().data());

        // 获取程序信息
        QProcess p;
        QString command = QString(buf) + QString("\\sigcheck64.exe");
        QStringList args;
        args.append(image_path);
        p.start(command, args);
        p.waitForFinished();
        QString tmp = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
        tmp.replace("\r", "");
        tmp.replace("\t", "");
        t = tmp.split("\n", QString::SkipEmptyParts);
//        qDebug() << t;
        if (t[3] != "No matching files were found."){
            // 显示启动项名称
            item0 = new QTableWidgetItem(name);
            QFileInfo fi(image_path.toLocal8Bit().data());
            QFileIconProvider ficon;
            QIcon icon = ficon.icon(fi);    // 获取图标
            item0->setIcon(icon);
            // 显示启动项描述
            item1 = new QTableWidgetItem(description);
            // 显示可执行文件目录
            item3 = new QTableWidgetItem(image_path);
            // 显示时间戳(最近修改时间)
            temp = fi.lastModified().toString("yyyy-MM-dd hh:mm:ss");
            item4 = new QTableWidgetItem(temp);
            // 显示证书信息
            temp = t[6].mid(10);
            if (t[4].contains("Unsigned")){
                temp = QString("(Not Verified)") + temp;
                item2 = new QTableWidgetItem(temp);
                if (type_ == "driver"){
                    driver_table->setRowCount(row_num_driver+1);
                    driver_table->setItem(row_num_driver, 0, item0);
                    driver_table->setItem(row_num_driver, 1, item1);
                    driver_table->setItem(row_num_driver, 2, item2);
                    driver_table->setItem(row_num_driver, 3, item3);
                    driver_table->setItem(row_num_driver, 4, item4);
                    for (i=0; i<5; i++)
                        driver_table->item(row_num_driver, i)->setBackgroundColor(QColor(255,182,193));
                }
                else{
                    service_table->setRowCount(row_num_service+1);
                    service_table->setItem(row_num_service, 0, item0);
                    service_table->setItem(row_num_service, 1, item1);
                    service_table->setItem(row_num_service, 2, item2);
                    service_table->setItem(row_num_service, 3, item3);
                    service_table->setItem(row_num_service, 4, item4);
                    for (i=0; i<5; i++)
                        service_table->item(row_num_service, i)->setBackgroundColor(QColor(255,182,193));
                }
            }
            else{
                temp = QString("(Verified)") + temp;
                item2 = new QTableWidgetItem(temp);
                if (type_ == "driver"){
                    driver_table->setRowCount(row_num_driver+1);
                    driver_table->setItem(row_num_driver, 0, item0);
                    driver_table->setItem(row_num_driver, 1, item1);
                    driver_table->setItem(row_num_driver, 2, item2);
                    driver_table->setItem(row_num_driver, 3, item3);
                    driver_table->setItem(row_num_driver, 4, item4);
                }
                else{
                    service_table->setRowCount(row_num_service+1);
                    service_table->setItem(row_num_service, 0, item0);
                    service_table->setItem(row_num_service, 1, item1);
                    service_table->setItem(row_num_service, 2, item2);
                    service_table->setItem(row_num_service, 3, item3);
                    service_table->setItem(row_num_service, 4, item4);
                }
            }
        }
        // 未找到可执行文件，则标黄显示
        else{
            // 显示启动项名称
            item0 = new QTableWidgetItem(name);
            // 显示启动项描述
            temp = QString("");
            item1 = new QTableWidgetItem(temp);
            // 显示可执行文件目录
            temp = QString("File not found: ") + image_path;
            item3 = new QTableWidgetItem(temp);
            // 显示时间戳(最近修改时间)
            temp = QString("");
            item4 = new QTableWidgetItem(temp);
            // 显示证书信息
            item2 = new QTableWidgetItem(temp);
            if (type_ == "driver"){
                driver_table->setRowCount(row_num_driver+1);
                driver_table->setItem(row_num_driver, 0, item0);
                driver_table->setItem(row_num_driver, 1, item1);
                driver_table->setItem(row_num_driver, 2, item2);
                driver_table->setItem(row_num_driver, 3, item3);
                driver_table->setItem(row_num_driver, 4, item4);
                for (i=0; i<5; i++)
                    driver_table->item(row_num_driver, i)->setBackgroundColor(QColor(255,255,0));
            }
            else{
                service_table->setRowCount(row_num_service+1);
                service_table->setItem(row_num_service, 0, item0);
                service_table->setItem(row_num_service, 1, item1);
                service_table->setItem(row_num_service, 2, item2);
                service_table->setItem(row_num_service, 3, item3);
                service_table->setItem(row_num_service, 4, item4);
                for (i=0; i<5; i++)
                    service_table->item(row_num_service, i)->setBackgroundColor(QColor(255,255,0));
            }
        }

        if (type_ == "driver")
            row_num_driver += 1;
        else
            row_num_service += 1;
    }
}

// 初始化scheduled tasks表格，参数(scheduled tasks表)
void MainWindow::init_schedule(QTableWidget *table){
    QString Schedule_path = QString("C:\\Windows\\System32\\Tasks");
    int i;
    QString temp;
    QTableWidgetItem *item;
    char buf[256];
    _getcwd(buf, sizeof(buf));  // 获取当前绝对路径
    int column_width[5] = {231, 200, 300, 410, 170};  // 定义每列的宽度
    int row_num = 0;
    for (i=0; i<=4; i++)
        table->setColumnWidth(i, column_width[i]);
    table->resizeRowsToContents();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    table->setSelectionMode(QAbstractItemView::SingleSelection);    //只能单选
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);   //禁止修改
    table->verticalHeader()->setVisible(false);   //隐藏列表头
    table->setShowGrid(false);    // 隐藏网格线
    // 设置样式表
    table->setStyleSheet("border:none; selection-color:black; selection-background-color:#E6E6FA;");

    table->setRowCount(row_num+1);

    item = new QTableWidgetItem(Schedule_path.toLocal8Bit().data());
    table->setItem(row_num, 0, item);
    table->setSpan(row_num, 0, 1, 5);
    item->setBackgroundColor(QColor(135,206,250));
    row_num += 1;

    // 获取程序信息
    QProcess p;
    QString command = QString(buf) +  QString("\\tasks.exe");
    p.start(command);
    p.waitForFinished();
    QString result = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    result = result.remove("=");
    result = result.remove("?");
    QStringList t = result.split("\n", QString::SkipEmptyParts);
    foreach (QString tmp, t){
//        qDebug("%s", tmp.toStdString().data());
        t = tmp.split("$#&");
//        qDebug("name:%s, description:%s, imagePath:%s", t[0].toStdString().data(), t[1].toStdString().data(), t[2].toStdString().data());

        if (t.length() == 3 && (!t[2].isEmpty())){
            table->setRowCount(row_num+1);  // 行数加一
//            QString image_path = format_image_path(t[2]).trimmed();
            QString image_path = t[2].trimmed();
//            qDebug("%s", image_path.toLocal8Bit().data());
            QString name = t[0].trimmed();
            QString description = t[1].trimmed();
            if (description == "")
                description = name;
            qDebug("%s === %s", name.toLocal8Bit().data(), image_path.toLocal8Bit().data());

            // 获取程序签名信息
            QProcess p;
            QString command = QString(buf) + QString("\\sigcheck64.exe");
//                qDebug("command: %s", command.toLocal8Bit().data());
            QStringList args;
            args.append(image_path);
            p.start(command, args);
            p.waitForFinished();

            QString result = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
//                qDebug("%s", result.toLocal8Bit().data());
            result.replace("\r", "");
            result.replace("\t", "");
//            qDebug("%s", result.toLocal8Bit().data());
            QStringList result_list = result.split("\n", QString::SkipEmptyParts);
//            qDebug() << "list: " << result_list;
            if (result_list[3] != "No matching files were found."){   // 找到可执行文件，则正常显示信息
                // 显示启动项名称
                item = new QTableWidgetItem(name);
                QFileInfo fi(image_path.toLocal8Bit().data());
                QFileIconProvider ficon;
                QIcon icon = ficon.icon(fi);    // 获取图标
                item->setIcon(icon);
                table->setItem(row_num, 0, item);
                // 显示启动项描述
                if (temp.contains("?"))
                    temp = name;
                item = new QTableWidgetItem(description);
                table->setItem(row_num, 1, item);
                // 显示可执行文件目录
                item = new QTableWidgetItem(image_path);
                table->setItem(row_num, 3, item);
                // 显示时间戳(最近修改时间)
                temp = fi.lastModified().toString("yyyy-MM-dd hh:mm:ss");
                item = new QTableWidgetItem(temp);
                table->setItem(row_num, 4, item);
                // 显示证书信息
                temp = result_list.at(6).mid(10);
                if (result_list.at(4).contains("Unsigned")){    // 未认证的程序标注
                    temp = QString("(Not Verified)") + temp;
                    item = new QTableWidgetItem(temp);
                    table->setItem(row_num, 2, item);
                    for (i=0; i<5; i++){
                        table->item(row_num, i)->setBackgroundColor(QColor(255,182,193));
                    }
                }
                else{
                    temp = QString("(Verified)") + temp;
                    item = new QTableWidgetItem(temp);
                    table->setItem(row_num, 2, item);
                }
            }
            // 未找到可执行文件，则标黄显示
            else{
                // 显示启动项名称
                item = new QTableWidgetItem(name);
                table->setItem(row_num, 0, item);
                // 显示启动项描述
                temp = QString("");
                item = new QTableWidgetItem(temp);
                table->setItem(row_num, 1, item);
                // 显示可执行文件目录
                temp = QString("File not found: ") + image_path;
                item = new QTableWidgetItem(temp);
                table->setItem(row_num, 3, item);
                // 显示时间戳(最近修改时间)
                temp = QString("");
                item = new QTableWidgetItem(temp);
                table->setItem(row_num, 4, item);
                // 显示证书信息
                item = new QTableWidgetItem(temp);
                table->setItem(row_num, 2, item);
                for (i=0; i<5; i++)
                    table->item(row_num, i)->setBackgroundColor(QColor(255,255,0));
            }
            row_num += 1;
        }
    }
}

// 初始化KnownDLLs表格，参数(knowndlls表)
void MainWindow::init_dlls(QTableWidget *table){
    QString Dlls_path = "HKLM\\System\\CurrentControlSet\\Control\\Session Manager\\KnownDlls";
    int i;
    QString temp;
    QTableWidgetItem *item;
    char buf[256];
    _getcwd(buf, sizeof(buf));  // 获取当前绝对路径
    int column_width[5] = {231, 200, 300, 410, 170};  // 定义每列的宽度
    int row_num = 0;
    for (i=0; i<=4; i++)
        table->setColumnWidth(i, column_width[i]);
    table->resizeRowsToContents();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    table->setSelectionMode(QAbstractItemView::SingleSelection);    //只能单选
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);   //禁止修改
    table->verticalHeader()->setVisible(false);   //隐藏列表头
    table->setShowGrid(false);    // 隐藏网格线
    // 设置样式表
    table->setStyleSheet("border:none; selection-color:black; selection-background-color:#E6E6FA;");

    table->setRowCount(row_num+1);

    item = new QTableWidgetItem(Dlls_path.toLocal8Bit().data());
    table->setItem(row_num, 0, item);
    table->setSpan(row_num, 0, 1, 5);
    item->setBackgroundColor(QColor(135,206,250));
    row_num += 1;

    // 获取程序信息
    QProcess p;
    QString command = QString(buf) +  QString("\\KnownDll.exe");
    p.start(command);
    p.waitForFinished();
    QString result = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    result = result.remove("=");
    result = result.remove("?");
    QStringList result_list = result.split("\n", QString::SkipEmptyParts);

    foreach (result, result_list){
//        qDebug() << result;
        table->setRowCount(row_num+1);  // 行数加一
        QStringList tmp = result.split("$#&");
        QString name = tmp[0].trimmed();
        QString description = tmp[1].trimmed();
        if (description == "")
            description = name;
        QString image_path = tmp[2].trimmed();
        qDebug("%s === %s", name.toLocal8Bit().data(), image_path.toLocal8Bit().data());

        // 获取程序签名信息
        QProcess p;
        QString command = QString(buf) + QString("\\sigcheck64.exe");
        QStringList args;
        args.append(image_path);
        p.start(command, args);
        p.waitForFinished();

        temp = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
//                qDebug("%s", temp.toLocal8Bit().data());
        temp.replace("\r", "");
        temp.replace("\t", "");
//        qDebug("%s", temp.toLocal8Bit().data());
        tmp = temp.split("\n", QString::SkipEmptyParts);
        qDebug() << "list: " << tmp;
        if (tmp[3] != "No matching files were found."){   // 找到可执行文件，则正常显示信息
            // 显示启动项名称
            item = new QTableWidgetItem(name);
            QFileInfo fi(image_path.toLocal8Bit().data());
            QFileIconProvider ficon;
            QIcon icon = ficon.icon(fi);    // 获取图标
            item->setIcon(icon);
            table->setItem(row_num, 0, item);
            // 显示启动项描述
            item = new QTableWidgetItem(description);
            table->setItem(row_num, 1, item);
            // 显示可执行文件目录
            item = new QTableWidgetItem(image_path);
            table->setItem(row_num, 3, item);
            // 显示时间戳(最近修改时间)
            temp = fi.lastModified().toString("yyyy-MM-dd hh:mm:ss");
            item = new QTableWidgetItem(temp);
            table->setItem(row_num, 4, item);
            // 显示证书信息
            temp = tmp.at(6).mid(10);
            if (tmp.at(4).contains("Unsigned")){    // 未认证的程序标注
                temp = QString("(Not Verified)") + temp;
                item = new QTableWidgetItem(temp);
                table->setItem(row_num, 2, item);
                for (i=0; i<5; i++){
                    table->item(row_num, i)->setBackgroundColor(QColor(255,182,193));
                }
            }
            else{
                temp = QString("(Verified)") + temp;
                item = new QTableWidgetItem(temp);
                table->setItem(row_num, 2, item);
            }
        }
        // 未找到可执行文件，则标黄显示
        else{
            // 显示启动项名称
            item = new QTableWidgetItem(name);
            table->setItem(row_num, 0, item);
            // 显示启动项描述
            temp = QString("");
            item = new QTableWidgetItem(temp);
            table->setItem(row_num, 1, item);
            // 显示可执行文件目录
            temp = QString("File not found: ") + image_path;
            item = new QTableWidgetItem(temp);
            table->setItem(row_num, 3, item);
            // 显示时间戳(最近修改时间)
            temp = QString("");
            item = new QTableWidgetItem(temp);
            table->setItem(row_num, 4, item);
            // 显示证书信息
            item = new QTableWidgetItem(temp);
            table->setItem(row_num, 2, item);
            for (i=0; i<5; i++)
                table->item(row_num, i)->setBackgroundColor(QColor(255,255,0));
        }
        row_num += 1;
    }
}

// 隐藏Windows签名的自启动项
void MainWindow::hide_windows(){
    int i;
//    qDebug() << ui->tableWidget->rowCount() << ui->tableWidget_2->rowCount() << ui->tableWidget_3->rowCount() << ui->tableWidget_4->rowCount() << ui->tableWidget_5->rowCount();
    if (ui->action_hide_windows->isChecked()){
        for (i=0; i<ui->tableWidget->rowCount(); i++){
            if (!ui->tableWidget->item(i, 0)->text().contains("HKLM") && !ui->tableWidget->item(i, 0)->text().contains("HKCU")){
                QString signature = ui->tableWidget->item(i, 2)->text();
                if (signature.contains("(Verified)") && signature.contains("Microsoft Windows"))
                    ui->tableWidget->setRowHidden(i, true);
            }
        }
        for (i=1; i<ui->tableWidget_2->rowCount(); i++){
            QString signature = ui->tableWidget_2->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft Windows"))
                ui->tableWidget_2->setRowHidden(i, true);
        }
        for (i=1; i<ui->tableWidget_3->rowCount(); i++){
            QString signature = ui->tableWidget_3->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft Windows"))
                ui->tableWidget_3->setRowHidden(i, true);
        }
        for (i=1; i<ui->tableWidget_4->rowCount(); i++){
            QString signature = ui->tableWidget_4->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft Windows"))
                ui->tableWidget_4->setRowHidden(i, true);
        }
        for (i=1; i<ui->tableWidget_5->rowCount(); i++){
            QString signature = ui->tableWidget_5->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft Windows"))
                ui->tableWidget_5->setRowHidden(i, true);
        }
    }
    else{
        for (i=0; i<ui->tableWidget->rowCount(); i++)
            ui->tableWidget->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_2->rowCount(); i++)
            ui->tableWidget_2->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_3->rowCount(); i++)
            ui->tableWidget_3->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_4->rowCount(); i++)
            ui->tableWidget_4->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_5->rowCount(); i++)
            ui->tableWidget_5->setRowHidden(i, false);
        ui->action_hide_microsoft->setChecked(false);
    }
}

// 隐藏Microsoft签名的自启动项
void MainWindow::hide_microsoft(){
    int i;
    if (ui->action_hide_microsoft->isChecked()){
        for (i=0; i<ui->tableWidget->rowCount(); i++){
            if (!ui->tableWidget->item(i, 0)->text().contains("HKLM") && !ui->tableWidget->item(i, 0)->text().contains("HKCU")){
                QString signature = ui->tableWidget->item(i, 2)->text();
                if (signature.contains("(Verified)") && signature.contains("Microsoft"))
                    ui->tableWidget->setRowHidden(i, true);
            }
        }
        for (i=1; i<ui->tableWidget_2->rowCount(); i++){
            QString signature = ui->tableWidget_2->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft"))
                ui->tableWidget_2->setRowHidden(i, true);
        }
        for (i=1; i<ui->tableWidget_3->rowCount(); i++){
            QString signature = ui->tableWidget_3->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft"))
                ui->tableWidget_3->setRowHidden(i, true);
        }
        for (i=1; i<ui->tableWidget_4->rowCount(); i++){
            QString signature = ui->tableWidget_4->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft"))
                ui->tableWidget_4->setRowHidden(i, true);
        }
        for (i=1; i<ui->tableWidget_5->rowCount(); i++){
            QString signature = ui->tableWidget_5->item(i, 2)->text();
            if (signature.contains("(Verified)") && signature.contains("Microsoft"))
                ui->tableWidget_5->setRowHidden(i, true);
        }
    }
    else{
        for (i=0; i<ui->tableWidget->rowCount(); i++)
            ui->tableWidget->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_2->rowCount(); i++)
            ui->tableWidget_2->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_3->rowCount(); i++)
            ui->tableWidget_3->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_4->rowCount(); i++)
            ui->tableWidget_4->setRowHidden(i, false);
        for (i=0; i<ui->tableWidget_5->rowCount(); i++)
            ui->tableWidget_5->setRowHidden(i, false);
        ui->action_hide_windows->setChecked(false);
    }
}

