#pragma once;
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <cvaux.h>//背景建模的头文件
using namespace cv; 

class CWaterFill
{
public:
	void Initialise();
	//16位数据图像处理函数
	Mat WaterDrop(Mat InputImg, int num_of_drop);//进行算法运算，加速之后的算法，且引入“惯性”
	Mat WaterFilter(Mat inputImg, int mimNum);//滤波，消除小于mimNum的像素

	Mat HalfSizeImg(Mat SrcImg);//将图像缩减至原来的一半，用于640*480图像转换为320*240的非插值缩小，必须为16位单通道图像
	Mat Sixth2Eight(Mat SrcImg, int ratio);//用于16位转8位图像,ratio是除数

	//图像处理的函数
	void GMM2(Mat InputImg, int nFrame, double learnRate);//背景建模函数
	void AndOpera(Mat SrcImg, Mat* DestImg);//对两幅图像进行与操作
	Mat ContourFilter(Mat Img, int minSize);
	void MergeBlack(Mat* pImg);
	void GetHead_Min(Mat Img, vector<RECT>* detectBox);//用于最后获取检测结果
	void GetHead(Mat Img, vector<RECT>* detectBox);//用于最后获取检测结果

	void Water(int nFrame, int threshold, vector<RECT>* detectBox, int stype=1);


	//////////////////////////////////////////////////////////////////////////
	//////定义变量
	Mat depthMap; //存储深度图，初始化一下，这里是16位的单通道，但是其实这里的初始化是没有用的，但求心理安慰
	Mat rgbImage;//存储彩色图像
	Mat minRGB;//缩小后的彩色图像
	Mat save_minRGB;//用于保存节省空间
	Mat minImg;
	Mat waterImg;
	Mat img_16bit;//定义一个16位图像，用于中间计算
	Mat img_8bit;//定义一个8位图像，用于中间计算
	Mat waterImg_8bit;//8bit的最终结果，用于显示
	Mat BgImg;//存储背景建模之后的图像

	//定义一些更小的图片用于WaterFill
	Mat superminImg;
	Mat superminWater;
	Mat supermin_16bit;
	Mat supermin_8bit;
	Mat supermin_Water8bit;

protected:
	BackgroundSubtractorMOG2 m_bg_model;//定义高斯背景模型	
	Mat m_fgmask;//背景建模之后形成的前景目标

};