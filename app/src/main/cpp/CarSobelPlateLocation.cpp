//
//  CarSobelPlateLocation.cpp
//  CarPlateRecognize
//
//  Created by liuxiang on 2017/7/28.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#include "common.h"


CarSobelPlateLocation::CarSobelPlateLocation(){
    
}

CarSobelPlateLocation::~CarSobelPlateLocation(){
    
}

//出来的效果和和pc上不一样。。。。。。
void CarSobelPlateLocation::processMat(Mat src, Mat& dst,int blur_size,int close_w,int close_h){
    //图像预处理 ———— 降噪================================================================
    //高斯滤波 也就是高斯模糊 降噪
    // imwrite("/sdcard/car/src.jpg",src);
    Mat blur;
    GaussianBlur(src, blur, Size(blur_size,blur_size), 0);
    // imwrite("/sdcard/car/blur.jpg",blur);
//    imshow("a", blur);
    //灰度
    Mat gray;
    cvtColor(blur, gray, CV_BGR2GRAY);
    // imwrite("/sdcard/car/gray.jpg",gray);
//    imshow("b", gray);
    //边缘检测滤波器 边缘检测 便于区分车牌
    Mat sobel,abs_sobel;
    Sobel(gray, sobel, CV_16S, 1, 0);
    // imwrite("/sdcard/car/sobel.jpg",sobel);
//    imshow("c", sobel);
    convertScaleAbs(sobel, abs_sobel);
    // imwrite("/sdcard/car/abs_sobel.jpg",abs_sobel);
//    imshow("d", abs_sobel);
    //加权
    Mat weighted;
    addWeighted(abs_sobel, 1, 0, 0, 0, weighted);
    // imwrite("/sdcard/car/weighted.jpg",weighted);
//    imshow("e", weighted);
    //二值
    Mat thresholds;
    threshold(weighted, thresholds, 0, 255, THRESH_OTSU + THRESH_BINARY);
    // imwrite("/sdcard/car/thresholds.jpg",thresholds);
//    imshow("f", thresholds);
    //闭操作 先膨胀、后腐蚀
    //把白色区域连接起来，或者扩大。任何黑色区域如果小于结构元素的大小都会被消除
    //对于结构大小 由于中国车牌比如 湘A 12345 有断层 所以 width过小不行,而过大会连接不必要的区域
    //sobel的方式定位不能100%匹配
    Mat element = getStructuringElement(MORPH_RECT, Size(close_w, close_h));
    morphologyEx(thresholds, dst, MORPH_CLOSE, element);
    // imwrite("/sdcard/car/dst.jpg",dst);
    blur.release();
    gray.release();
    sobel.release();
    abs_sobel.release();
    weighted.release();
    thresholds.release();
    element.release();
    
//    imshow("g", dst);
}


void CarSobelPlateLocation::plateLocate(Mat src, vector<Mat > &plates){
    Mat src_threshold;
    processMat(src, src_threshold, 5, 17, 3);
//    imshow("a", src_threshold);
//    waitKey();
    //获得初步筛选车牌轮廓================================================================
    //轮廓检测
    vector<vector<Point>> contours;
    findContours(src_threshold, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
//    imwrite("/sdcard/car/src_threshold.jpg",src_threshold);
    src_threshold.release();
    //满足初步筛选条件的轮廓
//    vector<RotatedRect> first_rects;
    //由sobel定位到的所有感兴趣区域(进过筛选的车牌区域)
    vector<RotatedRect> vec_sobel_roi;
    for(auto cnt : contours){
        RotatedRect rotated_rect = minAreaRect(Mat(cnt));
        if (verifySizes(rotated_rect)) {
             vec_sobel_roi.push_back(rotated_rect);
        }
    }

//    Mat dst;
//    processMat(src, dst, 3, 10, 3);
    tortuosity(src, vec_sobel_roi, plates);
//    dst.release();
}
