#pragma once;
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <cvaux.h>//������ģ��ͷ�ļ�
using namespace cv; 

class CWaterFill
{
public:
	void Initialise();
	//16λ����ͼ������
	Mat WaterDrop(Mat InputImg, int num_of_drop);//�����㷨���㣬����֮����㷨�������롰���ԡ�
	Mat WaterFilter(Mat inputImg, int mimNum);//�˲�������С��mimNum������

	Mat HalfSizeImg(Mat SrcImg);//��ͼ��������ԭ����һ�룬����640*480ͼ��ת��Ϊ320*240�ķǲ�ֵ��С������Ϊ16λ��ͨ��ͼ��
	Mat Sixth2Eight(Mat SrcImg, int ratio);//����16λת8λͼ��,ratio�ǳ���

	//ͼ����ĺ���
	void GMM2(Mat InputImg, int nFrame, double learnRate);//������ģ����
	void AndOpera(Mat SrcImg, Mat* DestImg);//������ͼ����������
	Mat ContourFilter(Mat Img, int minSize);
	void MergeBlack(Mat* pImg);
	void GetHead_Min(Mat Img, vector<RECT>* detectBox);//��������ȡ�����
	void GetHead(Mat Img, vector<RECT>* detectBox);//��������ȡ�����

	void Water(int nFrame, int threshold, vector<RECT>* detectBox, int stype=1);


	//////////////////////////////////////////////////////////////////////////
	//////�������
	Mat depthMap; //�洢���ͼ����ʼ��һ�£�������16λ�ĵ�ͨ����������ʵ����ĳ�ʼ����û���õģ���������ο
	Mat rgbImage;//�洢��ɫͼ��
	Mat minRGB;//��С��Ĳ�ɫͼ��
	Mat save_minRGB;//���ڱ����ʡ�ռ�
	Mat minImg;
	Mat waterImg;
	Mat img_16bit;//����һ��16λͼ�������м����
	Mat img_8bit;//����һ��8λͼ�������м����
	Mat waterImg_8bit;//8bit�����ս����������ʾ
	Mat BgImg;//�洢������ģ֮���ͼ��

	//����һЩ��С��ͼƬ����WaterFill
	Mat superminImg;
	Mat superminWater;
	Mat supermin_16bit;
	Mat supermin_8bit;
	Mat supermin_Water8bit;

protected:
	BackgroundSubtractorMOG2 m_bg_model;//�����˹����ģ��	
	Mat m_fgmask;//������ģ֮���γɵ�ǰ��Ŀ��

};