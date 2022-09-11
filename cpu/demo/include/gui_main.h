#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QCoreApplication>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QTextCodec>
#include <QMessageBox>
#include <QDebug>
#include <QMouseEvent>
#include <QCloseEvent>

# include <opencv2/core.hpp>
# include <opencv2/imgcodecs.hpp>
# include <opencv2/highgui.hpp>
# include <opencv2/opencv.hpp>
# include <opencv2/imgproc.hpp>

# include <iostream>
# include <string>
# include <stdio.h>
# include <stdlib.h>

//detection
#include <chrono>
#include <iostream>
#include <memory>
#include <numeric>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "paddle/include/paddle_inference_api.h"
#include "include/object_detector.h"

// sqlite3
#include <sqlite3.h>
#include <qinputdialog.h>
#include "my_sqlite.h"

QT_BEGIN_NAMESPACE
namespace Ui { class GUI_main; }
QT_END_NAMESPACE

class GUI_main : public QMainWindow
{
    Q_OBJECT

public:
    GUI_main(QWidget *parent = nullptr);
    ~GUI_main();
    void DB_init();
    void closeEvent(QCloseEvent *event);
    void fing_date(QString str);
    void insert_date();
    void fing_all_date();
    void get_row_num();
    void Predict_Image(const double threshold, PaddleDetection::ObjectDetector* det);
    void Process(int id_cam, const double threshold, PaddleDetection::ObjectDetector* det);
    void display_img(QString ID);
    QString ID_Worker_Input();

private slots:
    void on_Search_clicked();
    void on_View_clicked();
    void on_State_1_clicked();
    void on_State_2_clicked();
    void on_State_3_clicked();
    void on_State_4_clicked();
    void on_Chage_1_clicked();
    void on_Chage_2_clicked();
    void on_Chage_3_clicked();
    void on_Chage_4_clicked();
    void on_Start_info_clicked();
    void on_Input_textChanged();
    void DB_DellClick(QTableWidgetItem* item);
private:
    Ui::GUI_main *ui;
};
#endif // GUI_MAIN_H
