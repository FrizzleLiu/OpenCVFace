#include <jni.h>
#include <string>
#include "opencv2/opencv.hpp"
#include "macro.h"
#include <android/native_window_jni.h>

using namespace cv;
DetectionBasedTracker *tracker = 0;
ANativeWindow *window;

class CascadeDetectorAdapter : public DetectionBasedTracker::IDetector {
public:
    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector) :
            IDetector(),
            Detector(detector) {
        CV_Assert(detector);
    }

    void detect(const cv::Mat &Image, std::vector<cv::Rect> &objects) {
        Detector->detectMultiScale(Image, objects, scaleFactor, minNeighbours, 0, minObjSize,
                                   maxObjSize);
    }

    virtual ~CascadeDetectorAdapter() {

    }

private:
    CascadeDetectorAdapter();

    cv::Ptr<cv::CascadeClassifier> Detector;
};

//初始化
extern "C"
JNIEXPORT void JNICALL
Java_com_frizzle_opencvface_OpenCVJni_init(JNIEnv *env, jobject instance, jstring path_) {
    const char* path = env->GetStringUTFChars(path_,0);
    if (tracker) {
        tracker->stop();
        delete tracker;
        tracker = 0;
    }
    //智能指针
    Ptr<CascadeClassifier> classifier = makePtr<CascadeClassifier>(path);
    //创建一个跟踪适配器
    Ptr<CascadeDetectorAdapter> mainDetector = makePtr<CascadeDetectorAdapter>(classifier);
    Ptr<CascadeClassifier> classifier1 = makePtr<CascadeClassifier>(path);
    //创建一个跟踪适配器
    Ptr<CascadeDetectorAdapter> trackingDetector = makePtr<CascadeDetectorAdapter>(classifier1);
    //拿去用的跟踪器
    DetectionBasedTracker::Parameters DetectorParams;
    tracker = new DetectionBasedTracker(mainDetector, trackingDetector, DetectorParams);
    //开启跟踪器
    tracker->run();
    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_frizzle_opencvface_OpenCVJni_postData(JNIEnv *env, jobject instance, jbyteArray data_,
                                               jint w, jint h,
                                               jint camera_id) {
    jbyte *data = env->GetByteArrayElements(data_,NULL);
    //src相当于Bitmap
    Mat src(h+h/2,w,CV_8UC1,data);
    //将NV21转换成RGB
    cvtColor(src,src,COLOR_YUV2RGBA_NV21);

    if(camera_id == 1 ){
        //前置摄像头 逆时针旋转90°
        rotate(src,src,ROTATE_90_COUNTERCLOCKWISE);
        //1水平镜像翻转  0垂直镜像翻转
        flip(src,src,1);
    } else {
        //后置摄像头 顺时针旋转90°
        rotate(src,src,ROTATE_90_CLOCKWISE);
    }
    //灰度
    Mat gray;
    cvtColor(src,gray,COLOR_RGBA2GRAY);
    //直方图均衡化，提升对比度
    equalizeHist(gray,gray);

    //检测人脸
    std:: vector<Rect> faces;
    tracker->process(gray);
    tracker->getObjects(faces);
    LOGE("人脸数据%D",faces.size());
    for (Rect face : faces){
        rectangle(src,face,Scalar(255,0,255));
    }

    if (window){
        ANativeWindow_setBuffersGeometry(window,src.cols,src.rows,WINDOW_FORMAT_RGBA_8888);
        ANativeWindow_Buffer window_buffer;
        do{
            //lock失败直接break
            if (ANativeWindow_lock(window,&window_buffer,0)){
                ANativeWindow_release(window);
                LOGE("lock失败");
                window=0;
                break;
            }
            //把src.data copy到buffer.bits中去
            //一行一行的copy数据
            //将rgb数据给dst_data
            uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
            //stride: 一行有多少个数据 (RGBA*4)
            int dst_linesize = window_buffer.stride*4;
            for (int i = 0; i < window_buffer.height; ++i) {
                memcpy(dst_data+i*dst_linesize,src.data+i*src.cols*4,dst_linesize);
            }
            //提交刷新
            ANativeWindow_unlockAndPost(window);
        }while (0);
    }
    src.release();
    gray.release();
    env->ReleaseByteArrayElements(data_,data,0);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_frizzle_opencvface_OpenCVJni_setSurface(JNIEnv *env, jobject thiz, jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
}