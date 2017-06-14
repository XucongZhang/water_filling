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
	bool OpenCamera1();//������ͷ����1
	void GetCameraImg1(Mat* depthMap, Mat* rgbImage);
	bool OpenCamera2();//������ͷ����2
	void GetCameraImg2(Mat* depthMap, Mat* rgbImage);
	void CheckOpenNIError( XnStatus eResult, string sStatus );

	~COpenKinect();

	VideoCapture m_capture;//������Kinect��

protected:
	//����
	//����������ͷ�ı�������Ҫ��OpenNI������ͷʱ�õ��ı���
	XnStatus eResult; 
	xn::Context mContext; 
	xn::DepthMetaData m_DepthMD;
	xn::ImageMetaData m_ImageMD;
	XnMapOutputMode mapMode;
	xn::DepthGenerator mDepthGenerator;  
	xn::ImageGenerator mImageGenerator;
	
};