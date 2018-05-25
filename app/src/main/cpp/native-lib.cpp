#include "common.h"
#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>

#define TAG "lichao"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)


extern "C" {

CarPlateRecgize *carPlateRecgize = 0;

void bitmap2Mat(JNIEnv *env, jobject bitmap, Mat &dst) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    //获得bitmap信息
    CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
    //必须是 rgba8888 rgb565
    CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888);
    //lock 获得数据
    CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
    CV_Assert(pixels);
    dst.create(info.height, info.width, CV_8UC3);
    LOGI("bitmap2Mat: RGBA_8888 bitmap -> Mat");
    Mat tmp(info.height, info.width, CV_8UC4, pixels);
    cvtColor(tmp, dst, COLOR_RGBA2BGR);
    // cvtColor(dst, dst, COLOR_RGBA2RGB);
    tmp.release();
    AndroidBitmap_unlockPixels(env, bitmap);
}


void mat2Bitmap(JNIEnv *env, Mat &src, jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    LOGI("nMatToBitmap");
    CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
    CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888);
    CV_Assert(src.dims == 2 && info.height == (uint32_t) src.rows &&
              info.width == (uint32_t) src.cols);
    CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
    CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
    CV_Assert(pixels);
    Mat tmp(info.height, info.width, CV_8UC4, pixels);
    if (src.type() == CV_8UC1) {
        LOGI("nMatToBitmap: CV_8UC1 -> RGBA_8888");
        cvtColor(src, tmp, COLOR_GRAY2RGBA);
    } else if (src.type() == CV_8UC3) {
        LOGI("nMatToBitmap: CV_8UC3 -> RGBA_8888");
        cvtColor(src, tmp, COLOR_BGR2RGBA);
    } else if (src.type() == CV_8UC4) {
        LOGI("nMatToBitmap: CV_8UC4 -> RGBA_8888");
        src.copyTo(tmp);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return;

}

JNIEXPORT void JNICALL
Java_com_lichao_opencv_1carplate_MainActivity_init(JNIEnv *env, jobject instance, jstring svm_,
                                               jstring ann_, jstring ann_zh_) {
    const char *svm = env->GetStringUTFChars(svm_, 0);
    const char *ann = env->GetStringUTFChars(ann_, 0);
    const char *ann_zh = env->GetStringUTFChars(ann_zh_, 0);

    carPlateRecgize = new CarPlateRecgize(svm, ann, ann_zh);

    env->ReleaseStringUTFChars(svm_, svm);
    env->ReleaseStringUTFChars(ann_, ann);
    env->ReleaseStringUTFChars(ann_zh_, ann_zh);
}

JNIEXPORT void JNICALL
Java_com_lichao_opencv_1carplate_MainActivity_release(JNIEnv *env, jobject instance) {
    if (carPlateRecgize)
        delete carPlateRecgize;
}

JNIEXPORT jstring JNICALL
Java_com_lichao_opencv_1carplate_MainActivity_recognition(JNIEnv *env, jobject instance,
                                                          jobject bitmap, jobject out) {
    Mat src;
    bitmap2Mat(env, bitmap, src);
    Mat plate;
    string str = carPlateRecgize->plateRecognize(src, plate);
    mat2Bitmap(env, plate, out);
    src.release();
    return env->NewStringUTF(str.c_str());
}
}