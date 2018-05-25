//
//  CarPlateRecgnize.cpp
//  CarPlateRecognize
//
//  Created by liuxiang on 2017/7/28.
//  Copyright © 2017年 liuxiang. All rights reserved.
//


#include "common.h"



char CarPlateRecgize::CHARS[]  = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','J','K','L','M','N','P','Q','R','S','T','U','V','W','X','Y','Z'};

string CarPlateRecgize::ZHCHARS[]  = {"川","鄂","赣","甘","贵","桂","黑","沪","冀","津","京","吉","辽","鲁","蒙","闽","宁","青","琼","陕","苏","晋","皖","湘","新","豫","渝","粤","云","藏","浙"};


CarPlateRecgize::CarPlateRecgize(const char* svm_path,const char* ann_path, const char*ann_ch_path){
    sobel_plate_location = new CarSobelPlateLocation;
    color_plate_location = new CarColorPlateLocation;
    ann_size = Size(ANN_COLS,ANN_ROWS);
    svm = SVM::load(svm_path);
    ann = ANN_MLP::load(ann_path);
    ann_zh = ANN_MLP::load(ann_ch_path);
    svm_hog = new HOGDescriptor(Size(128, 64), Size(16, 16), Size(8, 8), Size(8, 8), 3);
    ann_hog = new HOGDescriptor(Size(32, 32), Size(16, 16), Size(8, 8), Size(8, 8), 3);
    
}

CarPlateRecgize::~CarPlateRecgize(){
    svm->clear();
    svm.release();
    ann->clear();
    ann.release();
    ann_zh->clear();
    ann_zh.release();
    if(sobel_plate_location){
        delete sobel_plate_location;
        sobel_plate_location = 0;
    }
    if(color_plate_location){
        delete color_plate_location;
        color_plate_location = 0;
    }
    if (svm_hog){
        delete svm_hog;
        svm_hog = 0;
    }
    if (ann_hog){
        delete ann_hog;
        ann_hog = 0;
    }
}


// #include <android/log.h>


// #define TAG "car_plate"

// #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)

void CarPlateRecgize::getHOGFeatures(HOGDescriptor *hog,Mat image, Mat& features) {


    vector<float> descriptor;

    Mat trainImg = Mat(hog->winSize, CV_32S);
    resize(image, trainImg, hog->winSize);

    //计算
    hog->compute(trainImg, descriptor, Size(8, 8));
    
    Mat mat_featrue(descriptor);
    mat_featrue.copyTo(features);
    mat_featrue.release();
    trainImg.release();
}




void CarPlateRecgize::plateLocation(Mat src,Mat& dst){
    
    vector< Mat > plates;
    
    vector< Mat > sobel_Plates;
    //使用sobel 定位车牌
    sobel_plate_location->plateLocate(src, sobel_Plates);
    plates.insert(plates.end(), sobel_Plates.begin(),sobel_Plates.end());
    
    
    vector< Mat > color_Plates;
    //使用颜色定位
    color_plate_location->plateLocate(src,color_Plates);
    plates.insert(plates.end(), color_Plates.begin(),color_Plates.end());
    

#if 0
    int i = 0;
#endif
    float minScore = 2;
    int index = -1;
    for (int i = 0;i< plates.size();++i) {
        auto plate = plates[i];
        Mat p;
        //下载的模型
        Mat src;
        cvtColor(plate, src, CV_BGR2GRAY);
        threshold(src, src, 0, 255, THRESH_BINARY+THRESH_OTSU);
        //自己训练的模型
        getHOGFeatures(svm_hog,src,p);
        Mat sample = p.reshape(1,1);
        sample.convertTo(p, CV_32FC1);
        float score = svm->predict(sample, noArray(), StatModel::Flags::RAW_OUTPUT);
        if (score < minScore){
            minScore = score;
            index = i;
        }
        src.release();
        p.release();
        sample.release();
        // LOGI("候选:%f",score);
#if 0
        char path[50];
        sprintf(path,"/sdcard/car/test%d.jpg",i);
        imwrite(path, plate.plate_mat);
        ++i;
#endif
        
//        waitKey();

    }
//    gray.release();
    // //根据score排序
    if (index >= 0) {
        dst = plates[index].clone();
    }
    // LOGI("选定:%f",sorted[0].score);
    for (auto p :plates) {
        p.release();
    }
}




//移除车牌固定点 固定点所在的行肯定色值上变化不大(基本上全是黑的) 所以如果出现这样的行 那么可以试着抹黑这一行
void CarPlateRecgize::clearFixPoint(Mat &img) {
    const int maxChange = 10;
    vector<int > vec;
    for (int i = 0; i < img.rows; i++) {
        int change = 0;
        for (int j = 0; j < img.cols - 1; j++) {
            //如果不一样则+1
            if (img.at<char>(i, j) != img.at<char>(i, j + 1)) change++;
        }
        vec.push_back(change);
    }
    for (int i = 0; i < img.rows; i++) {
        int change = vec[i];
        //跳跃数大于maxChange 的行数抹黑
        if (change <= maxChange) {
            for (int j = 0; j < img.cols; j++) {
                img.at<char>(i, j) = 0;
            }
        }
    }
    
}


int CarPlateRecgize::verifyCharSizes(Mat src) {
    float aspect = .9f;
    float charAspect = (float)src.cols / (float)src.rows;
    float error = 0.7f;
    float minHeight = 10.f;
    float maxHeight = 33.f;
    float minAspect = 0.05f;
    float maxAspect = aspect + aspect * error;
    if (charAspect > minAspect && charAspect < maxAspect &&
        src.rows >= minHeight && src.rows < maxHeight)
        return 1;
   return 0;
}


//得到中文后面的 轮廓下标
int CarPlateRecgize::getCityIndex( vector<Rect> vec_rect) {
    int specIndex = 0;
    for (int i = 0; i < vec_rect.size(); i++) {
        Rect mr = vec_rect[i];
        int midx = mr.x + mr.width / 2;
        //车牌平均分为7份 如果当前下标对应的矩形右边大于1/7 且小于2/7
        if (midx < WIDTH / 7 * 2 && midx > WIDTH / 7) {
            specIndex = i;
        }
    }
    return specIndex;
}

void CarPlateRecgize::getChineseRect(Rect src,Rect &dst ) {
    //宽度增加一点 数值是个大概就行 可以查看截出来的中文字符调整
    float width = src.width  * 1.15f;
    int x = src.x;
    
    //x多减去一点点(中文和后面间隔稍微宽一点点)
    int newx = x - int(width * 1.15f);
    dst.x = newx > 0 ? newx : 0;
    dst.y = src.y;
    dst.width = int(width);
    dst.height = src.height;
}

string CarPlateRecgize::plateRecognize(Mat src,Mat& plate){
    plateLocation(src,plate);
    if (plate.empty()) {
        return "未找到车牌";
    }
//    imwrite("/sdcard/car/test7.jpg", plate);
    Mat plate_gray;
    Mat plate_threshold;
    cvtColor(plate, plate_gray, CV_BGR2GRAY);
//    imshow("myraw",plate);

    threshold(plate_gray, plate_threshold, 0, 255, THRESH_OTSU + THRESH_BINARY);
//    plate_threshold = plate_gray.clone();
//    imshow("ppp",plate_threshold);
//    imshow("my", plate_threshold);
//    waitKey();
//    waitKey();
    clearFixPoint(plate_threshold);
//    imshow("aaa",plate_threshold);
//    waitKey();
    vector<Rect> vec_rect;
    
    
    vector<vector<Point> > contours;
    findContours(plate_threshold,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);
    for(auto cnt : contours){
        Rect rect = boundingRect(cnt);
        Mat aux_roi = plate_threshold(rect);
        if (verifyCharSizes(aux_roi)){
            vec_rect.push_back(rect);
//            rectangle(plate_gray, rect.tl(), rect.br(), Scalar::all(0));
        }
        aux_roi.release();
    }
//    imshow("plate", plate_gray);
//    waitKey();
    if (vec_rect.empty()){
        return "未找到车牌号码字符";
    }
    vector<Rect> sorted_rect(vec_rect);
    sort(sorted_rect.begin(), sorted_rect.end(),
         [](const Rect& r1, const Rect& r2) { return r1.x < r2.x; });
    int city_index = getCityIndex(sorted_rect);
    Rect chinese_rect;
    getChineseRect(sorted_rect[city_index],chinese_rect);
//    rectangle(plate_gray, chinese_rect.tl(), chinese_rect.br(), Scalar::all(0));
//    imshow("a", plate_gray);
//    waitKey();
    //中文的矩形
    vector<Mat> vec_chars;
    Mat dst = plate_threshold(chinese_rect);
//    resize(dst,dst,ann_size);
//    imshow("a", dst);
//    waitKey();
    vec_chars.push_back(dst);
//    char path[50];
//    sprintf(path, "/Users/xiang/Documents/xcodeWorkSpace/MyCarPlate/MyCarPlate/resource/chinese.jpg");
//    imwrite("/Users/xiang/Documents/xcodeWorkSpace/MyCarPlate/MyCarPlate/resource/chinese.jpg",plate_gray(chinese_rect));
    
//    rectangle(plate, chinese_rect.tl(), chinese_rect.br(), Scalar::all(0));
//    imshow("plate", plate);
//    waitKey();
    //其他矩形 数字 字母
    int count = 6;
    for (int i = city_index; i< sorted_rect.size() && count ;++i){
        Rect rect = sorted_rect[i];
//        if (chinese_rect.x+chinese_rect.width > rect.x) {
//            continue;
//        }
//        rectangle(plate_gray, rect.tl(), rect.br(), Scalar::all(0));
//        imshow("plate", plate);
        
//        char path[100];
//        sprintf(path, "/Users/xiang/Documents/xcodeWorkSpace/MyCarPlate/MyCarPlate/resource/%d.jpg",i);
//        imwrite(path,plate_gray(rect));

        Mat dst = plate_threshold(rect);
//        resize(dst,dst,ann_size);
        vec_chars.push_back(dst);
        --count;
    }
//    imshow("plate", plate_gray);
//    waitKey();
    
//    for(auto a :vec_chars){
//        imshow("char", a);
//        waitKey();
//    }
    
    
    //识别
    string plate_result;

    predict(vec_chars,plate_result);
    
    
    plate_gray.release();
    plate_threshold.release();
//    waitKey();
    return plate_result;
    
}





void CarPlateRecgize::predict(vector<Mat> vec_chars,string& plate_result){
    for(int i = 0;i<vec_chars.size();++i){
        Mat mat = vec_chars[i];
//        char path[100];
//        sprintf(path, "/sdcard/car/i/%d.jpg",i);
//        imwrite(path,mat);
        //结果
        Point maxLoc;
//        Mat1f sample = mat.reshape(1,1);
        Mat features;
        getHOGFeatures(ann_hog,mat, features);
        Mat sample = features.reshape(1,1);
        sample.convertTo(sample, CV_32FC1);
        Mat response;
        if (i) {
            //预测
            ann->predict(sample,response);
            //第三个参数是匹配度
            minMaxLoc(response,0,0,0,&maxLoc);
            plate_result += CHARS[maxLoc.x];
        }else{
            //中文
            ann_zh->predict(sample,response);
//            cout << response << endl;
            minMaxLoc(response,0,0,0,&maxLoc);
//            cout << maxLoc.x << endl;
            plate_result += ZHCHARS[maxLoc.x];
        }
        response.release();
        sample.release();
        features.release();
        mat.release();
    }
}



