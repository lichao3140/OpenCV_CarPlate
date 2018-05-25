//
//  common.h
//  CarPlateRecognize
//
//  Created by liuxiang on 2017/7/28.
//  Copyright © 2017年 liuxiang. All rights reserved.
//

#ifndef common_h
#define common_h

//自己训练的是144 33
//#define HEIGHT 33
//#define WIDTH 144

#define HEIGHT 36
#define WIDTH 136

//根据训练样本决定的
//列向量 宽
#define ANN_COLS 8
//行向量  高
#define ANN_ROWS 16




#include <opencv2/opencv.hpp>
using namespace cv;
using namespace ml;
using namespace std;


#include "CarPlateLocation.hpp"
#include "CarSobelPlateLocation.hpp"
#include "CarColorPlateLocation.hpp"
#include "CarPlateRecgnize.hpp"

#endif /* common_h */
