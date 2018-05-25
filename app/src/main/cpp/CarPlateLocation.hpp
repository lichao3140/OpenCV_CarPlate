//
//  CarPlateLocation.hpp
//  CarPlateRecognize
//
//  Created by liuxiang on 2017/7/30.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#ifndef CarPlateLocation_hpp
#define CarPlateLocation_hpp


class CarPlateLocation {
public:
    CarPlateLocation();

    virtual ~CarPlateLocation();

    virtual void plateLocate(Mat src, vector<Mat> &plates) = 0;

protected:
    void safeRect(Mat src, RotatedRect &rect, Rect2f &dst_rect);

    void rotation(Mat src, Mat &dst, Size rect_size, Point2f center, double angle);

    void tortuosity(Mat src,  vector<RotatedRect> &rects, vector<Mat> &dst_plates);

    int verifySizes(RotatedRect rotated_rect);

};

#endif /* CarPlateLocation_hpp */
