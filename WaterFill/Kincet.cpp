#pragma once
#include <opencv2/opencv.hpp>  
using namespace cv;  

#include <iostream>  
#include <stdlib.h>
using namespace std ;  

#include "Open_Kinect.h"
#include "WaterFill.h"


//int main(int argc,char** argv) 
//{  
//	COpenKinect kinect;//首先准备打开Kinect
//	kinect.infor_begin();//报告初始信息
//	kinect.Initialise();//初始化
//	kinect.OpenCamera2();//使用第二种方法打开摄像头，方便快捷好用，即OpenCV中集成的方法
//
//	CWaterFill waterfill;//定义WaterFill类
//	waterfill.Initialise();//初始化一下
//
//	/************************************************************************/
//	/* 这里定义输出框
//	/************************************************************************/
//	vector<RECT> detectBox;//定义一个变量，用于存储检测到的脑袋
//	int threshold = 25;//唯一的参数，范围大概在20~35之间。理论上只和 头与肩膀 的相对高度有关，与摄像机高度无关
//
//	//定义一些小变量
//	int nFrame = 0;//当前是第几帧
//	char m_hitKey = 0;
//
//	while(true)
//	{  
//		++nFrame;//记录一下这是第几帧了，用于指导背景建模学习率
//
//		//用第二种方法获取图像，这里使用哪个方法打开摄像头，就必须使用相应方法获取图像
//		kinect.GetCameraImg2(&waterfill.depthMap, &waterfill.rgbImage);
//
//		detectBox.clear();//每次都清空一下
//		//开始进行灌水处理，颤抖吧，地球人！
//		waterfill.Water(nFrame, threshold, &detectBox, 1);//最后的参数1指使用320*240的图片进行处理，详情见函数Water本身
//		//灌水结束，结果保存在detectBox中
//		imshow("RGB", waterfill.img_8bit);//展示一下惊人的结果
//
//		if (m_hitKey = waitKey(50))//按键响应
//		{
//			if (m_hitKey == 27)
//			{
//				break;
//			}
//		}
//	}  
//
//	destroyAllWindows();
//
//	return 0 ;  
//}  

/************************************************************************/
/*  测试专用，读取硬盘上的图片
/************************************************************************/

int main(int argc,char** argv) 
{  
	//COpenKinect kinect;//首先准备打开Kinect
	//kinect.infor_begin();//报告初始信息
	//kinect.Initialise();//初始化
	//kinect.OpenCamera2();//使用第二种方法打开摄像头，方便快捷好用，即OpenCV中集成的方法

	CWaterFill waterfill;//定义WaterFill类
	waterfill.Initialise();//初始化一下

	vector<RECT> detectBox;//定义一个变量，用于存储检测到的脑袋

	int nFrame = 0;//当前是第几帧
	int threshold = 25;//范围大概在20~35之间。理论上只和 头与肩膀 的相对高度有关，与摄像机高度无关

	char m_hitKey = 0;
	char image_name[100];

	for (int filenum=0; filenum< 2276; filenum++)
	{  
		++nFrame;//记录一下这是第几帧了 
		std::sprintf(image_name, "%s%d%s", "D:\\Test\\MyDep\\dep-", filenum, ".png");

		waterfill.depthMap = imread(image_name, CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH);
		detectBox.clear();//每次都清空一下
		//开始进行灌水处理，颤抖吧，地球人！
		//参数依次为第几帧，阈值参数
		waterfill.Water(nFrame, threshold, &detectBox, 1);

		//灌水结束，结果保存在detectBox中

		if (m_hitKey = waitKey(50))//判断按键
		{
			if (m_hitKey == 27)
			{
				break;
			}
		}

		imshow("RGB", waterfill.img_8bit);
	}  

	destroyAllWindows();

	return 0 ;  
}  