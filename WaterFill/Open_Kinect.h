#include <opencv2/opencv.hpp>
#include <XnCppWrapper.h>//OpenNI
using namespace cv;  

#include <iostream>  
#include <stdlib.h>
using namespace std ;  


class COpenKinect
{
public:
	void infor_begin();
	void Initialise();
	bool OpenCamera1();//打开摄像头方法1
	void GetCameraImg1(Mat* depthMap, Mat* rgbImage);
	bool OpenCamera2();//打开摄像头方法2
	void GetCameraImg2(Mat* depthMap, Mat* rgbImage);
	void CheckOpenNIError( XnStatus eResult, string sStatus );

	~COpenKinect();

	VideoCapture m_capture;//用来打开Kinect的

protected:
	//变量
	//用来打开摄像头的变量，主要是OpenNI打开摄像头时用到的变量
	XnStatus eResult; 
	xn::Context mContext; 
	xn::DepthMetaData m_DepthMD;
	xn::ImageMetaData m_ImageMD;
	XnMapOutputMode mapMode;
	xn::DepthGenerator mDepthGenerator;  
	xn::ImageGenerator mImageGenerator;
	
};