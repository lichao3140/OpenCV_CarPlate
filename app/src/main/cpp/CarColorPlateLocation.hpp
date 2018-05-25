//
//  CarColorPlateLocation.hpp
//  CarPlateRecognize
//
//  Created by liuxiang on 2017/7/30.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#ifndef CarColorPlateLocation_hpp
#define CarColorPlateLocation_hpp

class CarColorPlateLocation:CarPlateLocation{
public:
    CarColorPlateLocation();
    ~CarColorPlateLocation();
    void plateLocate(Mat src, vector<Mat > &sobel_Plates);
};

#endif /* CarColorPlateLocation_hpp */
