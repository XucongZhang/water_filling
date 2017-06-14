#include "Open_Kinect.h"

void COpenKinect::infor_begin() 
{  
	cout <<  "kinect opening..." << endl ;  
}  

void COpenKinect::Initialise()
{
	eResult = XN_STATUS_OK;//用于打开摄像头过程中监视状态
}

COpenKinect::~COpenKinect()
{
	// 10. stop  
	mContext.StopGeneratingAll();
}
void COpenKinect::CheckOpenNIError( XnStatus eResult, string sStatus )
{ 
	if( eResult != XN_STATUS_OK ) 
		cerr << sStatus << " Error: " << xnGetStatusString( eResult ) << endl;
}

/************************************************************************/
/* 第二种打开摄像头的方法，即使用OpenCV2.3中的函数                                              
/************************************************************************/
bool COpenKinect::OpenCamera2()
{
	m_capture.open(CV_CAP_OPENNI);//打开kinect摄像头
	if ( !m_capture.isOpened()) 
	{  
		cerr << "no device has derected!" << endl;
	}

	cout<<"Kinect is ready!"<<endl;
	return true;
}

void COpenKinect::GetCameraImg2(Mat* depthMap, Mat* rgbImage)
{
	m_capture.grab();//获取图像
	//depthMap (CV_16UC1)，也可以搞点“云”神马的，是3通道的东西
	m_capture.retrieve(*depthMap,CV_CAP_OPENNI_DEPTH_MAP);  
	//color image (CV_8UC3)  
	m_capture.retrieve(*rgbImage,CV_CAP_OPENNI_BGR_IMAGE);
}

/************************************************************************/
/*第一种打开摄像头的方法，即使用OpenNI的方法
/************************************************************************/
bool COpenKinect::OpenCamera1()
{ 
	// 1. initial val
	XnInt16 m_Width = 640,m_Height = 480;

	//// 2. initial context 
	eResult = mContext.Init(); 
	CheckOpenNIError( eResult, "initialize context" );  

	//// 3. create depth generator   
	eResult = mDepthGenerator.Create( mContext ); 
	CheckOpenNIError( eResult, "Create depth generator" );  

	//// 4. create image generator 
	eResult = mImageGenerator.Create( mContext ); 
	CheckOpenNIError( eResult, "Create image generator" );

	//// 5. set map mode   
	mapMode.nXRes = 640;  
	mapMode.nYRes = 480; 
	mapMode.nFPS = 30; 
	eResult = mDepthGenerator.SetMapOutputMode( mapMode );  
	eResult = mImageGenerator.SetMapOutputMode( mapMode );  

	//// 6. correct view port  
	mDepthGenerator.GetAlternativeViewPointCap().SetViewPoint( mImageGenerator ); 

	cout << "The Camera is Open!" << endl;
	return true;
}

void COpenKinect::GetCameraImg1(Mat* depthMap, Mat* rgbImage)
{
	// 7. tart generate data  
	eResult = mContext.StartGeneratingAll();  

	//// 8. read data  
	eResult = mContext.WaitNoneUpdateAll(); 
	// 9a. get the depth map  
	mDepthGenerator.GetMetaData(m_DepthMD); 
	// 9b. get the image map  
	mImageGenerator.GetMetaData(m_ImageMD);
	memcpy(rgbImage->data,m_ImageMD.Data(), rgbImage->rows*rgbImage->cols*3);
	memcpy(depthMap->data,m_DepthMD.Data(), depthMap->rows*depthMap->cols*2);
	cvtColor(*rgbImage, *rgbImage, CV_RGB2BGR);
}
