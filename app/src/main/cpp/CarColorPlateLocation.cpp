//
//  CarColorPlateLocation.cpp
//  CarPlateRecognize
//
//  Created by liuxiang on 2017/7/30.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#include "common.h"

CarColorPlateLocation::CarColorPlateLocation(){
    
}

CarColorPlateLocation::~CarColorPlateLocation(){
    
}

void CarColorPlateLocation::plateLocate(Mat src, vector<Mat > &plates){
    Mat src_hsv;
    //分割hsv
//    色相（H）是色彩的基本属性，就是平常所说的颜色名称，如红色、黄色等。
//    饱和度（S）是指色彩的纯度，越高色彩越纯，低则逐渐变灰。
//    明度（V），亮度。
//    本身h 值是在0-360 s和v都是0-1 h 为240是蓝色 取值在 200-280 都为蓝色
//    在opencv中hsv 分别为 0-180 0-255 0-255 也就是h取值 /2
    //使用 H分量 进行蓝色 的匹配工作
    cvtColor(src, src_hsv, CV_BGR2HSV);
    vector<Mat> hsv_split;
    split(src_hsv, hsv_split);
//    imshow("a",hsv_split[0]);
//    imshow("b",hsv_split[1]);
//    imshow("c",hsv_split[2]);
    //直方图均衡
    equalizeHist(hsv_split[2], hsv_split[2]);
//    imshow("d",hsv_split[2]);
//     imshow("a",src_hsv);
//    equalizeHist(src_hsv, src_hsv);
//     imshow("b",src_hsv);
    merge(hsv_split, src_hsv);
//    imshow("c", src_hsv);
    
    
    
    //h分量 最大最小蓝色值
    const int min_h = 100;
    const int max_h = 140;

    //图像数据列需要考虑通道数的影响  如rgb=3
    int channels = src_hsv.channels();
    int rows = src_hsv.rows;
    int cols = src_hsv.cols * channels;
    //内存足够大的话图像的每一行是连续存放的 图像是否连续
    if (src_hsv.isContinuous()) {
        cols *= rows;
        rows = 1;
    }
    for (int i = 0; i < rows; ++i){
        //获得该行数据
        uchar*  p = src_hsv.ptr<uchar>(i);
        //hsv
        for (int j = 0; j < cols; j += 3) {
            int H = int(p[j]);      // 0-180
            int S = int(p[j + 1]);  // 0-255
            int V = int(p[j + 2]);  // 0-255
          
            
            bool colorMatched = false;
            
            if (H > min_h && H < max_h) {
                //亮度与饱和度都要在此范围 则匹配 可能是车牌
                //105和95是阀值 可以调整 太高或太低可能就不是蓝色了。
                //可以下载一个hsv调色工具 查看
                if ((S > 105 && S < 255) && (V > 95 && V < 255)){
                    colorMatched = true;
                }
            }
            
            if (colorMatched) {
                p[j] = 0;
                p[j + 1] = 0;
                p[j + 2] = 255;
            } else {
                //不匹配的设置为黑色
                p[j] = 0;
                p[j + 1] = 0;
                p[j + 2] = 0;
            }
            
        }
    }
    
//    imshow("a", src_hsv);
//    waitKey();
    
    
    vector<Mat> hsv_split_c;
    split(src_hsv, hsv_split_c);
    
    Mat src_threshold;
    threshold(hsv_split_c[2], src_threshold, 0, 255,
              CV_THRESH_OTSU + CV_THRESH_BINARY);
//    imshow("a", src_threshold);
//    waitKey();

    
    Mat element = getStructuringElement(MORPH_RECT, Size(10, 2));
    morphologyEx(src_threshold, src_threshold, MORPH_CLOSE, element);
    
//    imshow("a", src_threshold);
//    waitKey();

    
    vector<RotatedRect> colors_plates;
    vector<vector<Point> > contours;
    findContours(src_threshold,contours,RETR_EXTERNAL,CHAIN_APPROX_NONE);
    for (auto cnt : contours) {
         RotatedRect mr = minAreaRect(cnt);
        if (verifySizes(mr)){
            colors_plates.push_back(mr);
//            rectangle(src, mr.boundingRect().tl(), mr.boundingRect().br(), Scalar::all(0));
        }
    }
    
    
//    imshow("b", src);
//    waitKey();

    tortuosity(src, colors_plates, plates);
    
    src_hsv.release();
    for (auto hsv:hsv_split_c) {
        hsv.release();
    }
    for (auto hsv:hsv_split) {
        hsv.release();
    }
    src_threshold.release();
    element.release();
//    
//    imshow("a", src_threshold);
//    waitKey();

    
}
