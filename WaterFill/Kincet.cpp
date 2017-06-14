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
//	COpenKinect kinect;//����׼����Kinect
//	kinect.infor_begin();//�����ʼ��Ϣ
//	kinect.Initialise();//��ʼ��
//	kinect.OpenCamera2();//ʹ�õڶ��ַ���������ͷ�������ݺ��ã���OpenCV�м��ɵķ���
//
//	CWaterFill waterfill;//����WaterFill��
//	waterfill.Initialise();//��ʼ��һ��
//
//	/************************************************************************/
//	/* ���ﶨ�������
//	/************************************************************************/
//	vector<RECT> detectBox;//����һ�����������ڴ洢��⵽���Դ�
//	int threshold = 25;//Ψһ�Ĳ�������Χ�����20~35֮�䡣������ֻ�� ͷ���� ����Ը߶��йأ���������߶��޹�
//
//	//����һЩС����
//	int nFrame = 0;//��ǰ�ǵڼ�֡
//	char m_hitKey = 0;
//
//	while(true)
//	{  
//		++nFrame;//��¼һ�����ǵڼ�֡�ˣ�����ָ��������ģѧϰ��
//
//		//�õڶ��ַ�����ȡͼ������ʹ���ĸ�����������ͷ���ͱ���ʹ����Ӧ������ȡͼ��
//		kinect.GetCameraImg2(&waterfill.depthMap, &waterfill.rgbImage);
//
//		detectBox.clear();//ÿ�ζ����һ��
//		//��ʼ���й�ˮ���������ɣ������ˣ�
//		waterfill.Water(nFrame, threshold, &detectBox, 1);//���Ĳ���1ָʹ��320*240��ͼƬ���д������������Water����
//		//��ˮ���������������detectBox��
//		imshow("RGB", waterfill.img_8bit);//չʾһ�¾��˵Ľ��
//
//		if (m_hitKey = waitKey(50))//������Ӧ
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
/*  ����ר�ã���ȡӲ���ϵ�ͼƬ
/************************************************************************/

int main(int argc,char** argv) 
{  
	//COpenKinect kinect;//����׼����Kinect
	//kinect.infor_begin();//�����ʼ��Ϣ
	//kinect.Initialise();//��ʼ��
	//kinect.OpenCamera2();//ʹ�õڶ��ַ���������ͷ�������ݺ��ã���OpenCV�м��ɵķ���

	CWaterFill waterfill;//����WaterFill��
	waterfill.Initialise();//��ʼ��һ��

	vector<RECT> detectBox;//����һ�����������ڴ洢��⵽���Դ�

	int nFrame = 0;//��ǰ�ǵڼ�֡
	int threshold = 25;//��Χ�����20~35֮�䡣������ֻ�� ͷ���� ����Ը߶��йأ���������߶��޹�

	char m_hitKey = 0;
	char image_name[100];

	for (int filenum=0; filenum< 2276; filenum++)
	{  
		++nFrame;//��¼һ�����ǵڼ�֡�� 
		std::sprintf(image_name, "%s%d%s", "D:\\Test\\MyDep\\dep-", filenum, ".png");

		waterfill.depthMap = imread(image_name, CV_LOAD_IMAGE_ANYCOLOR|CV_LOAD_IMAGE_ANYDEPTH);
		detectBox.clear();//ÿ�ζ����һ��
		//��ʼ���й�ˮ���������ɣ������ˣ�
		//��������Ϊ�ڼ�֡����ֵ����
		waterfill.Water(nFrame, threshold, &detectBox, 1);

		//��ˮ���������������detectBox��

		if (m_hitKey = waitKey(50))//�жϰ���
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