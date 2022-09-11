#include "gui_main.h"
#include "ui_gui_main.h"


DEFINE_string(model_dir, "../picodet_l_640_coco_lcnet", "Path of inference model");
DEFINE_string(image_file, "", "Path of input image");
DEFINE_string(image_dir,
              "",
              "Dir of input image, `image_file` has a higher priority.");
DEFINE_int32(batch_size, 1, "batch_size");
DEFINE_string(
    video_file,
    "",
    "Path of input video, `video_file` or `camera_id` has a highest priority.");
DEFINE_int32(camera_id, -1, "Device id of camera to predict");
DEFINE_bool(
    use_gpu,
    false,
    "Deprecated, please use `--device` to set the device you want to run.");
DEFINE_string(device,
              "CPU",
              "Choose the device you want to run, it can be: CPU/GPU/XPU, "
              "default is CPU.");
DEFINE_double(threshold, 0.5, "Threshold of score.");
DEFINE_string(output_dir, "../output", "Directory of output visualization files.");
DEFINE_string(run_mode,
              "paddle",
              "Mode of running(paddle/trt_fp32/trt_fp16/trt_int8)");
DEFINE_int32(gpu_id, 0, "Device id of GPU to execute");
DEFINE_bool(run_benchmark,
            false,
            "Whether to predict a image_file repeatedly for benchmark");
DEFINE_bool(use_mkldnn, false, "Whether use mkldnn with CPU");
DEFINE_int32(cpu_threads, 1, "Num of threads with CPU");
DEFINE_int32(trt_min_shape, 1, "Min shape of TRT DynamicShapeI");
DEFINE_int32(trt_max_shape, 1280, "Max shape of TRT DynamicShapeI");
DEFINE_int32(trt_opt_shape, 640, "Opt shape of TRT DynamicShapeI");
DEFINE_bool(trt_calib_mode,
            false,
            "If the model is produced by TRT offline quantitative calibration, "
            "trt_calib_mode need to set True");

using namespace cv;//找不到就是ET114.pro中opencv路径问题
using namespace std;
namespace{
    static int Date_Row = 5;
    static int Date_Column = 13;
    bool flag_stop = 0;
    bool release_flag = 0;
    int Channel_0 = 2;
    int Channel_1 = 0;
    int Channel_2 = 6;
    int Channel_3 = 4;
    cv::Mat frameA;
    cv::Mat frameB;
    cv::Mat frameC;
    cv::Mat frameD;
    cv::VideoCapture capA(Channel_0);
    cv::VideoCapture capB(Channel_1);
    cv::VideoCapture capC(Channel_2);
    cv::VideoCapture capD(Channel_3);
    cv::Mat vis_img;

    // 数据库相关
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char sql[1024] = {'\0'};
    int date_info[11];
    int id_count = 0;
    int ID_SUM = 0;
    int COUNT_SUM = 0;
    QString ID_Cam0, ID_Cam1, ID_Cam2, ID_Cam3;
    QStringList strParamValue;

    //工人
    QString ID_Worker_0, ID_Worker_1, ID_Worker_2, ID_Worker_3;

    //模型
    PaddleDetection::ObjectDetector det(FLAGS_model_dir,
                                            FLAGS_device,
                                            FLAGS_use_mkldnn,
                                            FLAGS_cpu_threads,
                                            FLAGS_run_mode,
                                            FLAGS_batch_size,
                                            FLAGS_gpu_id,
                                            FLAGS_trt_min_shape,
                                            FLAGS_trt_max_shape,
                                            FLAGS_trt_opt_shape,
                                            FLAGS_trt_calib_mode);

}

extern int item_value[9], id_num;
extern bool find_flag, count_flag;

// paddle inference
using paddle_infer::Config;
using paddle_infer::Predictor;
using paddle_infer::CreatePredictor;

QImage Mat2QImage(cv::Mat mtx);
void PrintBenchmarkLog(std::vector<double> det_time, int img_num);
void PredictImage(const std::vector<std::string> all_img_paths,
                  const int batch_size,
                  const double threshold,
                  const bool run_benchmark,
                  PaddleDetection::ObjectDetector* det,
                  const std::string& output_dir = "output");
void Predict_Image(const double threshold, PaddleDetection::ObjectDetector* det);
void date_info_init();


GUI_main::GUI_main(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GUI_main)
{
    ui->setupUi(this);

    DB_init();

    ui->Info->setText("Here are some Information.");


    rc = sqlite3_open("../database/et114.db", &db);

    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);
    }else{
        fprintf(stderr, "Opened database successfully\n");
    }


    date_info_init();
    get_row_num();

    //module input
    Config config;
    config.SetModel("../picodet_l_640_coco_lcnet/model.pdmodel","../picodet_l_640_coco_lcnet/model.pdiparams");

    config.EnableMKLDNN();
    // Open the memory optim.
    config.EnableMemoryOptim();

    std::shared_ptr<Predictor> predictor = CreatePredictor(config);



    // // 获取输入 Tensor
    auto input_names = predictor->GetInputNames();
    auto input_tensor = predictor->GetInputHandle(input_names[0]);

    std::cout << input_names.size() << std::endl;
    std::cout << input_names[0] << std::endl;
    std::cout << input_names[1] << std::endl;




}

GUI_main::~GUI_main()
{
    delete ui;
}


void GUI_main::DB_init()
{
    //数据库链接信息
    ui->Datebase->setRowCount(Date_Row);
    ui->Datebase->setColumnCount(Date_Column);
    QStringList strList;
    strList << tr("序号") << tr("支撑板") << tr("横梁") << tr("工人") << tr("电机") << tr("升降柱") << tr("配件包") << tr("工人") << tr("桌脚") << tr("传动组") << tr("说明书") << tr("工人") << tr("工人");
    ui->Datebase->setHorizontalHeaderLabels(strList);

    for(int nRow = 0; nRow < Date_Row; nRow++){
        ui->Datebase->setRowHeight(nRow,10);
        for(int nColumn = 0; nColumn < Date_Column; nColumn++){
            if (nRow == 0) ui->Datebase->setColumnWidth(nColumn,67);
            QTableWidgetItem *pItem = new QTableWidgetItem();
                pItem->setFlags(Qt::ItemIsEnabled);              //设置编辑状态为浅色不可编辑
                ui->Datebase->setItem(nRow,nColumn,pItem);
        }
    }
    connect(ui->Datebase,SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(DB_DellClick(QTableWidgetItem*)));//链接点击事件
}

//点击事件获取
void GUI_main::DB_DellClick(QTableWidgetItem* item)
{   //获取行号以及序列实现查看相应图片
    qDebug()<<"click!:::"<<item->row()<<","<<item->column()<<":::"<<item->text();
    qDebug()<<"序号::"<<ui->Datebase->item(item->row(),0)->text();
    fing_date(ui->Datebase->item(item->row(),0)->text());
    //TODO数据库搜索对应序号信息并显示图片
}


void GUI_main::on_State_1_clicked()
{
    if(capA.isOpened()){
        Process(0,0.5,&det);
    }else{
        QMessageBox::information(this,
                                 tr("提示"),
                                 tr("请检查摄像头0是否规范启动！"));
    }
}
void GUI_main::on_State_2_clicked()
{
    if(capB.isOpened()){
        Process(1,0.5,&det);
    }else{
        QMessageBox::information(this,
                                 tr("提示"),
                                 tr("请检查摄像头1是否规范启动！"));
    }
}
void GUI_main::on_State_3_clicked()
{
    if(capC.isOpened()){
        Process(2,0.5,&det);
    }else{
        QMessageBox::information(this,
                                 tr("提示"),
                                 tr("请检查摄像头2是否规范启动！"));
    }
}
void GUI_main::on_State_4_clicked()
{
    if(capD.isOpened()){
        Process(3,0.5,&det);
    }else{
        QMessageBox::information(this,
                                 tr("提示"),
                                 tr("请检查摄像头3是否规范启动！"));
    }
}
QString GUI_main::ID_Worker_Input(){
    bool ok;
    QString text;
    while(1){
        text = QInputDialog::getText(this, tr("员工信息"),tr("请输入员工编号"), QLineEdit::Normal,0, &ok);
        if(ok && !text.isEmpty()){
            return text;
            break;
        }else{
            text.clear();
            QMessageBox::information(this,
                                     tr("员工信息"),
                                     tr("请输入有效信息！！！"));
        }
    }
    insert_date();
}
void GUI_main::on_Chage_1_clicked()
{
    ID_Worker_0 =ID_Worker_Input();
    qDebug()<<"员工0信息"<<ID_Worker_0;
}
void GUI_main::on_Chage_2_clicked()
{
    ID_Worker_1 = ID_Worker_Input();
    qDebug()<<"员工1信息"<<ID_Worker_1;
}
void GUI_main::on_Chage_3_clicked()
{
    ID_Worker_2 = ID_Worker_Input();
    qDebug()<<"员工2信息"<<ID_Worker_2;
}
void GUI_main::on_Chage_4_clicked()
{
    ID_Worker_3 = ID_Worker_Input();
    qDebug()<<"员工3信息"<<ID_Worker_3;
}
void GUI_main::on_Start_info_clicked()
{
    //摄像头

    bool ID_ok,ID_sum_ok;
    QString ID_text, SUM_text;
    while (1) {
        ID_text = QInputDialog::getText(this, tr("登录信息"),tr("请输入产品起始编码(如：LG00001)"), QLineEdit::Normal,0, &ID_ok);
        SUM_text = QInputDialog::getText(this, tr("登录信息"),tr("请输入产品批次数"), QLineEdit::Normal,0, &ID_sum_ok);
        if(ID_ok && ID_sum_ok && !ID_text.isEmpty() && !SUM_text.isEmpty()){
            ID_Cam0 = ID_text;
            ID_Cam1 = ID_Cam0;
            ID_Cam2 = ID_Cam0;
            ID_Cam3 = ID_Cam0;
            ID_SUM = SUM_text.toInt();
            break;
        }else{
            ID_text.clear();
            SUM_text.clear();
            QMessageBox::information(this,
                                     tr("登录信息"),
                                     tr("请输入有效信息！！！"));
        }
    }

    bool CapA_opened = 0;
    bool CapB_opened = 0;
    bool CapC_opened = 0;
    bool CapD_opened = 0;

    flag_stop = 0;

    if(release_flag){
        capA.open(Channel_0);
        capA.set(CAP_PROP_FRAME_WIDTH, 640);
        capA.set(CAP_PROP_FRAME_HEIGHT, 480);
        capB.open(Channel_1);
        capB.set(CAP_PROP_FRAME_WIDTH, 640);
        capB.set(CAP_PROP_FRAME_HEIGHT, 480);
        capC.open(Channel_2);
        capC.set(CAP_PROP_FRAME_WIDTH, 640);
        capC.set(CAP_PROP_FRAME_HEIGHT, 480);
        capD.open(Channel_3);
        capD.set(CAP_PROP_FRAME_WIDTH, 640);
        capD.set(CAP_PROP_FRAME_HEIGHT, 480);
        release_flag = 0;
    }

    if(!capA.isOpened()) {
        QMessageBox::information(this,
                                 tr("Failed to open camera 0"),
                                 tr("Failed to open camera 0 !!!"));
    }else{
        CapA_opened = 1;
    }

    if(!capB.isOpened()) {
        QMessageBox::information(this,
                                 tr("Failed to open camera 1"),
                                 tr("Failed to open camera 1 !!!"));
    }else{
        CapB_opened = 1;
    }

    if(!capC.isOpened()) {
        QMessageBox::information(this,
                                 tr("Failed to open camera 2"),
                                 tr("Failed to open camera 2 !!!"));
    }else{
        CapC_opened = 1;
    }

    if(!capD.isOpened()) {
        QMessageBox::information(this,
                                 tr("Failed to open camera 3"),
                                 tr("Failed to open camera 3 !!!"));
    }else{
        CapD_opened = 1;
    }

    while(1){
        if(CapA_opened)
        {
            capA >> frameA;
            QImage img0 = Mat2QImage(frameA);
            ui->Cam0_2->setPixmap(QPixmap::fromImage(img0.scaled(ui->Cam0_2->width(), ui->Cam0_2->height(), Qt::KeepAspectRatio)));
        }

        if(CapB_opened)
        {
            capB >> frameB;
            QImage img1 = Mat2QImage(frameB);
            ui->Cam1_2->setPixmap(QPixmap::fromImage(img1.scaled(ui->Cam1_2->width(), ui->Cam1_2->height(), Qt::KeepAspectRatio)));
        }

        if(CapC_opened)
        {
            capC >> frameC;
            QImage img2 = Mat2QImage(frameC);
            ui->Cam2_2->setPixmap(QPixmap::fromImage(img2.scaled(ui->Cam2_2->width(), ui->Cam2_2->height(), Qt::KeepAspectRatio)));
        }

        if(CapD_opened)
        {
            capD >> frameD;
            QImage img3 = Mat2QImage(frameD);
            ui->Cam3_2->setPixmap(QPixmap::fromImage(img3.scaled(ui->Cam3_2->width(), ui->Cam3_2->height(), Qt::KeepAspectRatio)));
        }
        cv::waitKey(1);
        if(flag_stop){
            capA.release();
            capB.release();
            capC.release();
            capD.release();
            release_flag = 1;
            break;
        }
    }
}

void GUI_main::on_Input_textChanged()
{
    qDebug()<<"输入序号::"<<ui->Input->toPlainText();
}


void GUI_main::on_View_clicked()
{
}

void GUI_main::on_Search_clicked()
{
    qDebug()<<"搜索模式";
    fing_date(ui->Input->toPlainText());
    qDebug()<<"搜索信息：："<<ui->Input->toPlainText();
}
QImage Mat2QImage(cv::Mat mtx)
{
    switch (mtx.type())
    {
    case CV_8UC1:
        {
            QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols, QImage::Format_Grayscale8);
            return img;
        }break;
    case CV_8UC3:
        {
            QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 3, QImage::Format_RGB888);
            return img.rgbSwapped();
        }break;
    case CV_8UC4:
        {
            QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 4, QImage::Format_ARGB32);
            return img;
        }break;
    default:
        {
            QImage img;
            return img;
        }break;
    }
}
void GUI_main::closeEvent(QCloseEvent *event){
    qDebug()<<"Shutdown";
    flag_stop = 1;
    sqlite3_close(db);
    event->accept();
}

void GUI_main::display_img(QString ID){
    cv::Mat img0,img1,img2,img3;
    QString temp,outputdir;
    outputdir = "/home/bj/workspace/opensource/ScholarShip/Paddle-X86/cpu/demo/output/"+ ID;
    qDebug()<<"图片地址::"<<outputdir;
    std::string res;

    if(capA.isOpened()){
        res = (outputdir+"_0.jpg").toStdString();
        img0 = cv::imread(res);
        QImage cam0 = Mat2QImage(img0);
        ui->Cam0->setPixmap(QPixmap::fromImage(cam0.scaled(ui->Cam0->width(), ui->Cam0->height(), Qt::KeepAspectRatio)));
    }

    if(capB.isOpened()){
        res = (outputdir+"_1.jpg").toStdString();
        img1 = cv::imread(res);
        QImage cam1 = Mat2QImage(img1);
        ui->Cam1->setPixmap(QPixmap::fromImage(cam1.scaled(ui->Cam1->width(), ui->Cam1->height(), Qt::KeepAspectRatio)));
    }

    if(capC.isOpened()){
        res = (outputdir+"_2.jpg").toStdString();
        img2 = cv::imread(res);
        QImage cam2 = Mat2QImage(img2);
        ui->Cam2->setPixmap(QPixmap::fromImage(cam2.scaled(ui->Cam2->width(), ui->Cam2->height(), Qt::KeepAspectRatio)));
    }

    if(capD.isOpened()){
        res = (outputdir+"_3.jpg").toStdString();
        img3 = cv::imread(res);
        QImage cam3 = Mat2QImage(img3);
        ui->Cam3->setPixmap(QPixmap::fromImage(cam3.scaled(ui->Cam3->width(), ui->Cam3->height(), Qt::KeepAspectRatio)));
    }
}

//数据库

void date_info_init(){
    for(int i = 0; i<11; i++){
        date_info[i]=0;
    }
}
void GUI_main::fing_all_date(){
    const char* data = "ALL DATE ITEMS:\n";
    sprintf(sql, "SELECT * from COMPANY;");
    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, date_callback, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK ){
       fprintf(stderr, "SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
    }else{
       fprintf(stdout, "Operation done successfully\n");
    }
}
void GUI_main::fing_date(QString str){
    extern string D_ID_Worker_0, D_ID_Worker_1, D_ID_Worker_2, D_ID_Worker_3, D_ID;
    const char* data = "Find the DATE:\n";
    sprintf(sql, "Select * from COMPANY where id='%s';", str.toStdString().c_str());
    rc = sqlite3_exec(db, sql, find_callback, (void*)data, &zErrMsg);
     if(SQLITE_OK != rc){
         fprintf(stderr, "find item:%s\n", sqlite3_errmsg(db));
         fprintf(stderr, "find item:%s\n", zErrMsg);
     }else if(find_flag){
         find_flag = 0;
         for(int nColumn = 0; nColumn < Date_Column; nColumn++){
             QString info;
             QTableWidgetItem *pItem;
             if(nColumn == 0)  pItem = new QTableWidgetItem(D_ID.c_str());
             if(nColumn == 1 ){
                 if(item_value[1] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 2 ){
                 if(item_value[2] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 3 ) pItem = new QTableWidgetItem(D_ID_Worker_0.c_str());
             if(nColumn == 4 ){
                 if(item_value[3] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 5 ){
                 if(item_value[4] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 6 ){
                 if(item_value[5] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 7 ) pItem = new QTableWidgetItem(D_ID_Worker_1.c_str());
             if(nColumn == 8 ){
                 if(item_value[6] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 9 ){
                 if(item_value[7] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 10 ){
                 if(item_value[8] == 0) info = "NO";
                 else info = "YES";
                 pItem = new QTableWidgetItem(info);
             }
             if(nColumn == 11 ) pItem = new QTableWidgetItem(D_ID_Worker_2.c_str());
             if(nColumn == 12 ) pItem = new QTableWidgetItem(D_ID_Worker_3.c_str());
             pItem->setFlags(Qt::ItemIsEnabled);              //设置编辑状态为浅色不可编辑
             ui->Datebase->setItem(0,nColumn,pItem);
         }
         for(int nRow = 1; nRow < Date_Row; nRow++){
             ui->Datebase->setRowHeight(nRow,10);
             for(int nColumn = 0; nColumn < Date_Column; nColumn++){
                 if (nRow == 0) ui->Datebase->setColumnWidth(nColumn,67);
                 if(nRow*Date_Column+nColumn < strParamValue.count()){
                     QTableWidgetItem *pItem = new QTableWidgetItem();
                     pItem->setFlags(Qt::ItemIsEnabled);              //设置编辑状态为浅色不可编辑
                     ui->Datebase->setItem(nRow,nColumn,pItem);
                 }
             }
         }
         fprintf(stderr, "Find table successfully\n");
         display_img(D_ID.c_str());
         ui->Info->setText("！请仔细核对装配情况！");
     }else{
         fprintf(stderr, "NO Such table Item\n");
         for(int nColumn = 0; nColumn < Date_Column; nColumn++){
             QTableWidgetItem *pItem;
             pItem = new QTableWidgetItem();
             pItem->setFlags(Qt::ItemIsEnabled);              //设置编辑状态为浅色不可编辑
             ui->Datebase->setItem(0,nColumn,pItem);
         }
         ui->Cam0->clear();
         ui->Cam1->clear();
         ui->Cam2->clear();
         ui->Cam3->clear();
         ui->Info->setText("！无此编号产品装配完成！");
    }

}
void GUI_main::insert_date(){
    QString info;
    for(int i=12;i>=0;i--){
        //for test
        if(i == 0)  strParamValue.insert(0,ID_Cam3);
        if(i == 1 ){
            if(date_info[1] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 2 ){
            if(date_info[2] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 3 ) strParamValue.insert(0,ID_Worker_0);
        if(i == 4 ){
            if(date_info[3] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 5 ){
            if(date_info[4] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 6 ){
            if(date_info[5] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 7 ) strParamValue.insert(0,ID_Worker_1);
        if(i == 8 ){
            if(date_info[6] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 9 ){
            if(date_info[7] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 10 ){
            if(date_info[8] == 0) info = "NO";
            else info = "YES";
            strParamValue.insert(0,info);
        }
        if(i == 11 ) strParamValue.insert(0,ID_Worker_2);
        if(i == 12 ) strParamValue.insert(0,ID_Worker_3);
    }
    for(int nRow = 0; nRow < Date_Row; nRow++){
        ui->Datebase->setRowHeight(nRow,10);
        for(int nColumn = 0; nColumn < Date_Column; nColumn++){
            if (nRow == 0) ui->Datebase->setColumnWidth(nColumn,67);
            if(nRow*Date_Column+nColumn < strParamValue.count()){
                QTableWidgetItem *pItem = new QTableWidgetItem(strParamValue.at(nRow*Date_Column+nColumn));
                pItem->setFlags(Qt::ItemIsEnabled);              //设置编辑状态为浅色不可编辑
                ui->Datebase->setItem(nRow,nColumn,pItem);
            }
        }
    }
    //test
    sprintf(sql, "insert into COMPANY (id, support_plate, beam, work_id_0, motor, lifting_columns, accessory_package, work_id_1, table_leg, transmission, instructions, work_id_2, work_id_3) values ('%s', %d, %d, '%s', %d, %d, %d, '%s', %d, %d, %d, '%s', '%s');",ID_Cam3.toStdString().c_str() , date_info[1], date_info[2], ID_Worker_0.toStdString().c_str(), date_info[3], date_info[4], date_info[5], ID_Worker_1.toStdString().c_str(), date_info[6], date_info[7], date_info[8], ID_Worker_2.toStdString().c_str(), ID_Worker_3.toStdString().c_str());
    qDebug()<<sql;
    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, NULL, NULL, &zErrMsg);
    if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }else{
      fprintf(stdout, "Operation done successfully\n");
    }
}
void GUI_main::get_row_num(){
    const char* data = "Row Count of Table:\n";
    sprintf(sql, "Select COUNT(*) from COMPANY;");
    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, id_count_callback, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    }else{
      while(!count_flag){};
      count_flag = 0;
      id_count = id_num;
      fprintf(stdout, "Operation done successfully\n");
    }
}

//检测



void GUI_main::Process(int id_cam, const double threshold, PaddleDetection::ObjectDetector* det){
    std::string info;
    std::vector<cv::Mat> batch_imgs;

    switch(id_cam){
    case 0:
      batch_imgs.insert(batch_imgs.end(), frameA);
      info = "Information of frameA:::";
      break;
    case 1:
      batch_imgs.insert(batch_imgs.end(), frameB);
      info = "Information of frameB:::";
      break;
    case 2:
      batch_imgs.insert(batch_imgs.end(), frameC);
      info = "Information of frameC:::";
      break;
    case 3:
      batch_imgs.insert(batch_imgs.end(), frameD);
      info = "Information of frameD:::";
      break;
    }

    // Store all detected result
    std::vector<PaddleDetection::ObjectResult> result;
    std::vector<int> bbox_num;
    std::vector<double> det_times;
    bool is_rbox = false;
    det->Predict(batch_imgs, threshold, 0, 1, &result, &bbox_num, &det_times);
    // get labels and colormap
    auto labels = det->GetLabelList();
    auto colormap = PaddleDetection::GenerateColorMap(labels.size());

    cv::Mat im = batch_imgs[0];
    std::vector<PaddleDetection::ObjectResult> im_result;
    int detect_num = 0;

    for (int j = 0; j < bbox_num[0]; j++) {
        PaddleDetection::ObjectResult item = result[j];
        if (item.confidence < threshold || item.class_id == -1) {
          continue;
        }
        detect_num += 1;
        im_result.push_back(item);
        if (item.rect.size() > 6) {
          is_rbox = true;
          printf("class=%d confidence=%.4f rect=[%d %d %d %d %d %d %d %d]\n",
                 item.class_id,
                 item.confidence,
                 item.rect[0],
                 item.rect[1],
                 item.rect[2],
                 item.rect[3],
                 item.rect[4],
                 item.rect[5],
                 item.rect[6],
                 item.rect[7]);
        } else {
          printf("class=%d confidence=%.4f rect=[%d %d %d %d]\n",
                 item.class_id,
                 item.confidence,
                 item.rect[0],
                 item.rect[1],
                 item.rect[2],
                 item.rect[3]);
        }
        if(item.class_id == 41 ||item.class_id==76||item.class_id==73||item.class_id==64||item.class_id==47||item.class_id==46||item.class_id==45||item.class_id==44){
            switch(item.class_id){
            case 41://第一层
                date_info[1] = 1;
                break;
            case 76:
                date_info[2] = 1;
                break;
            case 73://第二层
                date_info[3] = 1;
                break;
            case 64:
                date_info[4] = 1;
                break;
            case 47:
                date_info[5] = 1;
                break;
            case 46://第三层
                date_info[6] = 1;
                break;
            case 45:
                date_info[7] = 1;
                break;
            case 44:
                date_info[8] = 1;
                break;

            }
        }

    }
    date_info[0]=id_count++;
    std::cout << info << " The number of detected box: " << detect_num << std::endl;
    // Visualization result
    vis_img = PaddleDetection::VisualizeResult(
      im, im_result, labels, colormap, is_rbox);
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95);

    //中英混编累加
    QString outputdir = "/home/bj/workspace/opensource/ScholarShip/Paddle-X86/cpu/demo/output/";
    QString temp, S_str2, N_str1, N_str2;
    QRegExp rx("(\\d)");  // 匹配数字
    QRegExp rd("(\\c)");  // 匹配数字
    int pos = 0;
    if(id_cam == 0){
        S_str2 = ID_Cam0;
        while ((pos = rx.indexIn(ID_Cam0, pos)) != -1) {
            N_str1 = rx.cap(1);
            pos += rx.matchedLength();
            N_str2.append(N_str1);
        }
        qDebug()<<"字符串部分::"<<S_str2.remove(N_str2)<<"数字部分::"<<N_str2;
        outputdir = outputdir + ID_Cam0 + "_0.jpg";
        std::string res = outputdir.toStdString();
        cout<<"\nOutPutDir:::::"<<res<<endl;
        cv::imwrite(res, vis_img);
        ui->Info->setText("装配完成总量："+temp.number(id_count));
        ID_Cam0 = S_str2.remove(N_str2);
        for(int i = 0; i<(N_str2.size()-temp.number(N_str2.toInt()+1).size());i++){
            ID_Cam0 += '0';
        }
        ID_Cam0 += temp.number(N_str2.toInt()+1);
    }
    if(id_cam == 1){
        S_str2 = ID_Cam1;
        while ((pos = rx.indexIn(ID_Cam1, pos)) != -1) {
            N_str1 = rx.cap(1);
            pos += rx.matchedLength();
            N_str2.append(N_str1);
        }
        outputdir = outputdir + ID_Cam1 + "_1.jpg";
        std::string res = outputdir.toStdString();
        cout<<"\nOutPutDir:::::"<<res<<endl;
        cv::imwrite(res, vis_img);
        ui->Info->setText("装配完成总量："+temp.number(id_count));
        ID_Cam1 = S_str2.remove(N_str2);
        for(int i = 0; i<(N_str2.size()-temp.number(N_str2.toInt()+1).size());i++){
            ID_Cam1 += '0';
        }
        ID_Cam1 += temp.number(N_str2.toInt()+1);
    }
    if(id_cam == 2){
        S_str2 = ID_Cam2;
        while ((pos = rx.indexIn(ID_Cam2, pos)) != -1) {
            N_str1 = rx.cap(1);
            pos += rx.matchedLength();
            N_str2.append(N_str1);
        }
        outputdir = outputdir + ID_Cam2 + "_2.jpg";
        std::string res = outputdir.toStdString();
        cout<<"\nOutPutDir:::::"<<res<<endl;
        cv::imwrite(res, vis_img);

        ui->Info->setText("装配完成总量："+temp.number(id_count));
        ID_Cam2 = S_str2.remove(N_str2);
        for(int i = 0; i<(N_str2.size()-temp.number(N_str2.toInt()+1).size());i++){
            ID_Cam2 += '0';
        }
        ID_Cam2 += temp.number(N_str2.toInt()+1);
    }

    //id_cam 3
    if(id_cam == 3){
        S_str2 = ID_Cam3;
        while ((pos = rx.indexIn(ID_Cam3, pos)) != -1) {
            N_str1 = rx.cap(1);
            pos += rx.matchedLength();
            N_str2.append(N_str1);
        }
        outputdir = outputdir + ID_Cam3 +"_3.jpg";
        std::string res = outputdir.toStdString();
        cout<<"\nOutPutDir:::::"<<res<<endl;
        cv::imwrite(res, vis_img);

        insert_date();

        ui->Info->setText("装配完成总量："+temp.number(id_count));
        ID_Cam3 = S_str2.remove(N_str2);
        for(int i = 0; i<(N_str2.size()-temp.number(N_str2.toInt()+1).size());i++){
            ID_Cam3 += '0';
        }
        ID_Cam3 += temp.number(N_str2.toInt()+1);

        //for test
        date_info_init();
        COUNT_SUM++;
        if(COUNT_SUM>=ID_SUM){
            QMessageBox::information(this,
                                     tr("提示"),
                                     tr("当前批次以全部装配完毕！"));
        }

    }




}

