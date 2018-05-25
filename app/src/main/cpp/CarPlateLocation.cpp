//
//  CarPlateLocation.cpp
//  CarPlateRecognize
//
//  Created by liuxiang on 2017/7/30.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#include "common.h"

CarPlateLocation::CarPlateLocation() {}

CarPlateLocation::~CarPlateLocation() {}


int CarPlateLocation::verifySizes(RotatedRect rotated_rect) {
    //容错率
    float error = .75f;
    //中国车牌标准440mm*140mm
    //136 * 32
    float aspect = float(WIDTH) / float(HEIGHT);

    //最小 最大面积 不符合的丢弃
    //给个大概就行 随时调整
    int min = 20 * aspect * 20;
    int max = 180 * aspect * 180;

    //比例浮动 error认为也满足
    float rmin = aspect - aspect * error;
    float rmax = aspect + aspect * error;
    //面积
    float area = rotated_rect.size.height * rotated_rect.size.width;
    //可能是竖的车牌 宽比高小就用 高宽比
    float r = (float) rotated_rect.size.width / (float) rotated_rect.size.height;
    if (r < 1) r = (float) rotated_rect.size.height / (float) rotated_rect.size.width;
    if ((area < min || area > max) || (r < rmin || r > rmax))
        return 0;
    return 1;
}


void CarPlateLocation::rotation(Mat src, Mat &dst, Size rect_size,
                                Point2f center, double angle) {

    //获得旋转矩阵
    Mat rot_mat = getRotationMatrix2D(center, angle, 1);

    //运用仿射变换
    Mat mat_rotated;
    warpAffine(src, mat_rotated, rot_mat, Size(src.cols, src.rows),
               CV_INTER_CUBIC);
    //截取
    getRectSubPix(mat_rotated, Size(rect_size.width, rect_size.height),
                  center, dst);
    mat_rotated.release();
    rot_mat.release();
}


void CarPlateLocation::safeRect(Mat src, RotatedRect &rect, Rect2f &dst_rect) {
    Rect2f boudRect = rect.boundingRect2f();
    float tl_x = boudRect.x > 0 ? boudRect.x : 0;
    float tl_y = boudRect.y > 0 ? boudRect.y : 0;

    float br_x = boudRect.x + boudRect.width < src.cols
                 ? boudRect.x + boudRect.width - 1
                 : src.cols - 1;
    float br_y = boudRect.y + boudRect.height < src.rows
                 ? boudRect.y + boudRect.height - 1
                 : src.rows - 1;

    float roi_width = br_x - tl_x;
    float roi_height = br_y - tl_y;

    if (roi_width <= 0 || roi_height <= 0) return;
    dst_rect = Rect2f(tl_x, tl_y, roi_width, roi_height);
}


void CarPlateLocation::tortuosity(Mat src, vector<RotatedRect> &rects,
                                  vector<Mat> &dst_plates) {
    for (auto roi_rect : rects) {
        float r = (float) roi_rect.size.width / (float) roi_rect.size.height;
        float roi_angle = roi_rect.angle;
        Size roi_rect_size = roi_rect.size;
        //交换宽高
        if (r < 1) {
            roi_angle = 90 + roi_angle;
            swap(roi_rect_size.width, roi_rect_size.height);
        }
        Rect2f rect;
        safeRect(src, roi_rect, rect);
        Mat src_rect = src(rect);
        //相对于roi的中心点 不减去左上角坐标是相对于整个图的
        Point2f roi_ref_center = roi_rect.center - rect.tl();
        Mat deskew_mat;
        //不需要旋转的 旋转角度小没必要旋转了
        if ((roi_angle - 5 < 0 && roi_angle + 5 > 0) || 90.0 == roi_angle ||
            -90.0 == roi_angle) {
            deskew_mat = src_rect.clone();
        } else {
            Mat rotated_mat;
            rotation(src_rect, rotated_mat, roi_rect_size, roi_ref_center, roi_angle);
            deskew_mat = rotated_mat;
        }
        //一个大致宽高比范围
        if (deskew_mat.cols * 1.0 / deskew_mat.rows > 2.3 &&
            deskew_mat.cols * 1.0 / deskew_mat.rows < 6) {
            Mat plate_mat;
            plate_mat.create(HEIGHT, WIDTH, CV_8UC3);
            resize(deskew_mat, plate_mat, plate_mat.size());
            dst_plates.push_back(plate_mat);
        }
        deskew_mat.release();
    }
}

