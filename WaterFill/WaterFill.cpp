#include "WaterFill.h"

void CWaterFill::Initialise()
{
	depthMap.create(480,640,CV_16UC1); //�洢���ͼ����ʼ��һ�£�������16λ�ĵ�ͨ����������ʵ����ĳ�ʼ����û���õģ���������ο
	minRGB.create(240, 320, CV_8UC3);//��С��Ĳ�ɫͼ��
	save_minRGB.create(240, 320, CV_8UC3);//���ڱ����ʡ�ռ�
	minImg.create(240, 320, CV_16UC1);
	waterImg.create(240, 320, CV_16UC1);
	img_16bit.create(240,320, CV_16UC1);//����һ��16λͼ�������м����
	img_8bit.create(240, 320, CV_8UC1);//����һ��8λͼ�������м����
	waterImg_8bit.create(240, 320, CV_8UC1);//8bit�����ս����������ʾ
	BgImg.create(240, 320, CV_8UC1);//�洢������ģ֮���ͼ��

	//����һЩ��С��ͼƬ����WaterFill
	superminImg.create(120,160,CV_16UC1);
	superminWater.create(120,160,CV_16UC1);
	supermin_16bit.create(120,160,CV_16UC1);
	supermin_8bit.create(120,160,CV_16UC1);
	supermin_Water8bit.create(120,160,CV_16UC1);
}

//�����յõ����Դ�ͷ�϶�����һ��С������Ϊ��ͨ����ɫ��ͷ��Ϊ��ɫ
void CWaterFill::GetHead_Min(Mat Img, vector<RECT>* detectBox)
{
	CvMemStorage* m_storage;
	m_storage = cvCreateMemStorage(0);//����Ĭ�ϴ�С�Ŀռ�

	CvSeq *m_first_seq = NULL;
	CvSeq *m_prev_seq = NULL;
	CvSeq *m_seq = NULL;

	CvPoint pt1,pt2;//����ɫ�����������

	Mat inputcopy;
	Img.copyTo(inputcopy);//Ѱ�������������ԭʼͼ�����仯���ʸ���һ��
	IplImage* pp = &IplImage(inputcopy);//����Ҫת��һ�£���ΪcvFindContours�������ʺϲ���Mat��������

	//////////////////////////////////////////////////////////////////
	cvClearMemStorage(m_storage);
	cvFindContours(pp, m_storage, &m_first_seq, sizeof(CvContour), CV_RETR_LIST );//Ѱ��������������ı�����ͼ��
	for( m_seq = m_first_seq; m_seq; m_seq = m_seq->h_next )
	{
		CvContour* cnt = (CvContour*)m_seq;
		pt1.x = (cnt->rect.x)*2;//��һ����
		pt1.y = (cnt->rect.y)*2;
		pt2.x = (cnt->rect.x + cnt->rect.width)*2;
		pt2.y = (cnt->rect.y + cnt->rect.height)*2;
		rectangle(img_8bit, pt1, pt2, CV_RGB(255,255,255), -1);//���һ������С��0��ʱ�������������

		RECT head;
		head.left = pt1.x;
		head.right = pt2.x;
		head.top = pt1.y;
		head.bottom = pt2.y;
		detectBox->push_back(head);
	}
	//////////////////////////////////////////////////////////////////////////
	cvReleaseMemStorage(&m_storage);//�ͷſռ�
}


void CWaterFill::GetHead(Mat Img, vector<RECT>* detectBox)
{
	CvMemStorage* m_storage;
	m_storage = cvCreateMemStorage(0);//����Ĭ�ϴ�С�Ŀռ�

	CvSeq *m_first_seq = NULL;
	CvSeq *m_prev_seq = NULL;
	CvSeq *m_seq = NULL;

	CvPoint pt1,pt2;//����ɫ�����������

	Mat inputcopy;
	Img.copyTo(inputcopy);//Ѱ�������������ԭʼͼ�����仯���ʸ���һ��
	IplImage* pp = &IplImage(inputcopy);//����Ҫת��һ�£���ΪcvFindContours�������ʺϲ���Mat��������

	//////////////////////////////////////////////////////////////////
	cvClearMemStorage(m_storage);
	cvFindContours(pp, m_storage, &m_first_seq, sizeof(CvContour), CV_RETR_LIST );//Ѱ��������������ı�����ͼ��
	for( m_seq = m_first_seq; m_seq; m_seq = m_seq->h_next )
	{
		CvContour* cnt = (CvContour*)m_seq;
		pt1.x = (cnt->rect.x);//��һ����
		pt1.y = (cnt->rect.y);
		pt2.x = (cnt->rect.x + cnt->rect.width);
		pt2.y = (cnt->rect.y + cnt->rect.height);

		RECT head;
		head.left = pt1.x;
		head.right = pt2.x;
		head.top = pt1.y;
		head.bottom = pt2.y;
		detectBox->push_back(head);

		rectangle(img_8bit, pt1, pt2, CV_RGB(255,255,255), -1);//���һ������С��0��ʱ�������������
	}
	//////////////////////////////////////////////////////////////////////////
	cvReleaseMemStorage(&m_storage);//�ͷſռ�
}
/************************************************************************/
/*MergeBlack����������ͷ������Щ����ʶ��ĺ�ɫ�����ֵΪ�����õ�����,
�����ͼ������ֵΪ0�ļ�Ϊ�����ݵ㣬��ֵΪ1�ļ�Ϊ����������Ϊǰ��
/************************************************************************/
void CWaterFill::MergeBlack(Mat* pImg)
{
	//��ȷ��ʹ��16λͼ����8λͼ���޸Ĵ˴�����
	typedef ushort imgType;
	//�������ü�ⴰ�ڰ뾶��С��һ����΢���Դ���һ�㼴�ɣ����ò�����
	int radius= 30;

	double pData = 0;//�����ܺͣ�һ��Ҫ��
	int num = 0;
	int staPoint_x = 0;//���ü��ĳ�ʼ�ͽ�����
	int staPoint_y = 0;
	int endPoint_x = 0;
	int endPoint_y = 0;
	int width = pImg->cols;
	int height = pImg->rows;

	imgType* m_a = NULL;//����ָ��
	imgType* m_b = NULL;//����ָ��
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////��ƽ��ֵ����Դ��ϵĺ�ɫ����
	m_a = (imgType*)pImg->data;
	for (int y=0; y<height; ++y) //��������ͼ��
	{		
		for (int x=0; x<width; ++x, ++m_a) 
		{	
			if (*m_a == 0)//���������������
			{
				//////ȷ����ʼ�ͽ�����////////////////////////////
				if (y-radius/2 <= 0)
				{
					staPoint_y = 0;
				}
				else
				{
					staPoint_y = y-radius/2;
				}
				////////////////////////////////////////
				if (x-radius/2 <= 0)
				{
					staPoint_x = 0;
				}
				else
				{
					staPoint_x = x-radius/2;
				}
				/////////////////////////////////////////
				if (y+radius/2 >= height)
				{
					endPoint_y = height;
				}
				else
				{
					endPoint_y = y+radius/2;
				}
				////////////////////////////////////////////
				if (x+radius/2 >= width)
				{
					endPoint_x = width;
				}
				else
				{
					endPoint_x = x+radius/2;
				}
				////////////////////////////////////////////

				///////////�������ݵ�Ϊ���ģ�������Χ���ص�ƽ��ֵ��䵽�����ݵ�
				m_b = (imgType*)pImg->data;
				m_b += staPoint_y*width;
				for (int i=staPoint_y; i<endPoint_y; ++i,m_b+=width)//�ԣ�x��y��Ϊ����Ѱ��
				{
					for (int j=staPoint_x; j<endPoint_x; ++j)
					{
						if (m_b[j] !=1 && m_b[j] !=0)//����Ȳ��Ǳ����ֲ��������ݵ�
						{
							pData += m_b[j];
							++num;
						}
					}
				}
				pData = pData/num;//����ƽ��ֵ
				pImg->at<imgType>(y,x) = (imgType)pData;
				pData = 0;
				num = 0;
			} 
		}
	}
}

/****************************************************************************
����ContourFilter��������ǰ�������С��һ����С������
�����������ImgΪ����ͼ��minSizeΪ��С���������ֵΪ������ͼ��ע������
������������Ϊ8λ����ͼ�������ͼ��Ϊ��ֵ��ͼ��
***********************************************************************///////
Mat CWaterFill::ContourFilter(Mat Img, int minSize)
{
	int maxValue = 255;//��ʾÿ�����ص����ֵ�������8λ���ݣ�����Ӧ����255�������16λ����Ϊ65535
	//����ǰ���������ñ���
	CvMemStorage* m_storage;
	m_storage = cvCreateMemStorage(0);//����Ĭ�ϴ�С�Ŀռ�

	int m_region_count = 0;
	CvSeq *m_first_seq = NULL;
	CvSeq *m_prev_seq = NULL;
	CvSeq *m_seq = NULL;

	IplImage* ppp = &IplImage(Img);//����Ҫת��һ�£���ΪcvFindContours�������ʺϲ���Mat��������
	cvMorphologyEx( ppp, ppp, 0, 0, CV_MOP_OPEN , 1);
	cvMorphologyEx( ppp, ppp, 0, 0, CV_MOP_CLOSE, 1);
	//////////////////////////////////////////////////////////////////
	cvClearMemStorage(m_storage);
	cvFindContours(ppp, m_storage, &m_first_seq, sizeof(CvContour), CV_RETR_LIST );//Ѱ����������
	for( m_seq = m_first_seq; m_seq; m_seq = m_seq->h_next )
	{
		CvContour* cnt = (CvContour*)m_seq;
		if( cnt->rect.width * cnt->rect.height < minSize)//���ò���
		{
			//delete small contour
			m_prev_seq = m_seq->h_prev;
			if( m_prev_seq )
			{
				m_prev_seq->h_next = m_seq->h_next;
				if( m_seq->h_next ) m_seq->h_next->h_prev = m_prev_seq;
			}
			else
			{
				m_first_seq = m_seq->h_next;
				if( m_seq->h_next ) m_seq->h_next->h_prev = NULL;
			}
		}
		else
		{
			m_region_count++;
		}
	}
	cvZero(ppp);
	cvDrawContours(ppp, m_first_seq, CV_RGB(0, 0, maxValue), CV_RGB(0, 0, maxValue), 10, -1);
	//////////////////////////////////////////////////////////////////////////

	Mat bgImg(ppp, 0);//Ȼ����ת������

	cvReleaseMemStorage(&m_storage);//�ͷſռ�
	return bgImg;
}

/****************************************************************************
����GMM2����������ģ��ʹ��OpenCV���еĺ���
***********************************************************************///////
void CWaterFill::GMM2(Mat InputImg, int nFrame, double learnRate)
{
	m_bg_model(InputImg, m_fgmask, learnRate);//m_fgmask��Ϊǰ��
}
/************************************************************************/
/*��Ҫ�㷨������עˮ
/************************************************************************/
Mat CWaterFill::WaterDrop(Mat InputImg, int num_of_drop)
{
	typedef ushort imgType;//��ȷ��ʹ��16λͼ����8λͼ���޸Ĵ˴�����
	imgType maxValue = 65535;//��ʾÿ�����ص����ֵ�������8λ���ݣ�����Ӧ����255�������16λ����Ϊ65535

	imgType* m_a = NULL;
	imgType* m_b = NULL;
	imgType* m_c = NULL;
	int LocX;
	int LocY;

	int minP = maxValue;
	int picWidth = InputImg.cols;
	int picHeight = InputImg.rows;

	//��ʼ�����ͼ�����
	Mat outputImg(picHeight,picWidth, InputImg.type());
	memset(outputImg.data, 0, picWidth * picHeight*sizeof(imgType));//����
	//m_c = (imgType*)outputImg.data;
	
	uchar*  Label_P = new uchar[picWidth * picHeight];//���ڱ�ʶ�Ƿ�֮ǰ�ƶ����������
	memset(Label_P, 0, picWidth * picHeight);//����
	uchar* m_d = NULL;

	Mat inputcopy(picHeight,picWidth, InputImg.type());//��Ϊ���ֱ�Ӷ�����ͼ������������ԭʼͼ��仯
	InputImg.copyTo(inputcopy);
	m_a = (imgType*)inputcopy.data;
	m_b = (imgType*)inputcopy.data;///ԭʼͼƬָ��

	int index = 0;

	////�����б����������ݵ㶼����Ϊ��󣬲�����Ƚ�
	for (int i=0; i<picWidth*picHeight; ++i,++m_a)
	{
		if (*m_a < 2)//��������Щ�����������ݵ�
		{
			*m_a = maxValue;
		}
	}

	//�ָ�ָ��
	m_a = (imgType*)inputcopy.data;

	int waterWay = 0;
	bool isNext = false;//�Ƿ������һ��

	//////////////////////////////////////////////////////////////
	for (int j=0; j<picHeight; j+=5,m_a+=picWidth*5) //��������ͼƬ��ȫ������
	{
		for (int i=0; i<picWidth; i+=5)//ÿN����Ϊһ�飬�ӿ촦���ٶ�
		{
			if (m_a[i] != maxValue)/////����ǰ���Ž��д���//���Ϊ8λͼ����Ϊ255
			{
				////��ʼ��
				LocX = i;
				LocY = j;

				m_b = (imgType*)inputcopy.data;///ԭʼͼƬָ��
				m_b += picWidth*LocY + LocX;//ȷ��ָ��
				index = 0;
				index += picWidth*LocY + LocX; 
				m_c = (imgType*)outputImg.data;
				m_c += picWidth*LocY + LocX;//ȷ��ָ��
				m_d = Label_P;
				m_d += picWidth*LocY + LocX;//ȷ��ָ��

				for (int k=0; k<25; ++k)//ÿN����Ϊһ��
				{
					if (LocX>picWidth-1 || LocX<2 || LocY>picHeight-1 || LocY<2)//����ڱ߽�����ҾͲ�������
					{
						break;
					}

					memset(Label_P, 0, picWidth * picHeight);//����
					isNext = false;//��ʼ��һ�£������µ�һ����
					//memset(Label_P, 0, picWidth * picHeight*sizeof(imgType));

					//�����ڳ�ʼ����Χ���8�����ص�
						//���Ͻ�
						minP = *m_b;
						waterWay = 9;

						if (minP >= *(m_b-picWidth-1))
						{
							waterWay = 1;
							minP =  *(m_b-picWidth-1);
						}
						if (minP >= *(m_b-picWidth))//�Ϸ�
						{
							waterWay = 2;
							minP = *(m_b-picWidth);
						}
						if (minP >= *(m_b-picWidth+1))//���Ϸ�
						{
							waterWay = 3;
							minP = *(m_b-picWidth+1);
						}
						if (minP >= *(m_b+1))//�ҷ�
						{
							waterWay = 4;
							minP = *(m_b+1);
						}
						if (minP >= *(m_b+picWidth+1))//���·�
						{
							waterWay = 5;
							minP = *(m_b+picWidth+1);
						}
						if (minP >= *(m_b+picWidth))//�·�
						{
							waterWay = 6;
							minP = *(m_b+picWidth);
						}
						if (minP >= *(m_b+picWidth-1))//���·�
						{
							waterWay = 7;
							minP = *(m_b+picWidth-1);
						}
						if (minP >= *(m_b-1))//��
						{
							waterWay = 8;
							minP =  *(m_b-1);
						}

						while(!isNext)//�������û���ҵ�һ�����ʵĵ�
						{
							switch(waterWay)
							{
							case 1:
								LocX -= 1;
								LocY -= 1;
								if (LocX<2 || LocY <2)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}

								m_b = m_b-picWidth-1;//�ƶ���1��λ
								index = index-picWidth-1;
								m_c = m_c-picWidth-1;
								m_d = m_d-picWidth-1;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)))//���·�
								{
									waterWay = 7;minP = *(m_b+picWidth-1);
								}
								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))//���Ϸ�
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b-1) && !(*(m_d-1)))//��
								{
									waterWay = 8;minP = *(m_b-1);
								}
								if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))//�Ϸ�
								{
									waterWay = 2;minP = *(m_b-picWidth);
								}
								if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))//��������������������ȼ��������ˡ����ԡ�
								{
									minP = *(m_b-picWidth-1);//���Ͻ�
									waterWay = 1;
								}							
								break;
							case 2:
								LocY -= 1;
								if (LocY < 2)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}
								m_b -= picWidth;//2��λ
								index -= picWidth;
								m_c -= picWidth;
								m_d -= picWidth;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))//���Ͻ�
								{
									waterWay = 1;minP = *(m_b-picWidth-1);
								}
								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))//���Ͻ�
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))//���ķ������������ȼ���ŶҲ
								{
									minP = *(m_b-picWidth);//�Ϸ�
									waterWay = 2;
								}
								break;
							case 3:
								LocY -= 1;
								LocX += 1;
								if (LocY<2 || LocX>picWidth-1)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}
								m_b = m_b-picWidth+1;//3��λ
								index = index -picWidth+1;
								m_c = m_c-picWidth+1;
								m_d = m_d-picWidth+1;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))
								{
									waterWay = 1;minP = *(m_b-picWidth-1); 
								}
								if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)))
								{
									waterWay = 5;minP = *(m_b+picWidth+1);
								}
								if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))//�Ϸ�
								{
									waterWay = 2;minP = *(m_b-picWidth);
								}
								if (minP >= *(m_b+1) && !(*(m_d+1)))
								{
									waterWay = 4;minP = *(m_b+1);
								}
								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))
								{
									minP = *(m_b-picWidth+1);//���Ͻ�
									waterWay = 3;
								}
								break;
							case 4:
								LocX += 1;
								if (LocX >picWidth-1)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}
								m_b += 1;
								index +=1;
								m_c += 1;
								m_d += 1;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth)))//���Ͻ�
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)))//���Ͻ�
								{
									waterWay = 5;minP = *(m_b+picWidth+1);
								}
								if (minP >= *(m_b+1) && !(*(m_d+1)))
								{
									minP = *(m_b+1);//���Ͻ�
									waterWay = 4;
								}
								break;
							case 5:
								LocX += 1;
								LocY += 1;
								if (LocX >picWidth-1 || LocY > picHeight-1)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}
								m_b += picWidth+1;
								index += picWidth+1;
								m_c += picWidth+1;
								m_d += picWidth+1;
								*m_d = 1;
								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)) )//���Ͻ�
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )//���·�
								{
									waterWay = 7;minP = *(m_b+picWidth-1);
								}
								if (minP >= *(m_b+1) && !(*(m_d+1)))//�ҷ�
								{
									waterWay = 4;minP =  *(m_b+1);
								}
								if (minP >= *(m_b+picWidth) && !(*(m_d+picWidth)) )//�·�
								{
									waterWay = 6; minP = *(m_b+picWidth);
								}
								if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)) )
								{
									minP = *(m_b+picWidth+1);//���½�
									waterWay = 5;
								}
								break;
							case 6:
								LocY += 1;
								if (LocY > picHeight-1)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}
								m_b += picWidth;
								index += picWidth;
								m_c += picWidth;
								m_d += picWidth;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)) )
								{
									waterWay = 5;minP = *(m_b+picWidth+1);
								}
								if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )
								{
									waterWay = 7;
									minP = *(m_b+picWidth-1);
								}
								if (minP >= *(m_b+picWidth) && !(*(m_d+picWidth)) )
								{
									minP = *(m_b+picWidth);//�·�
									waterWay = 6;
								}
								break;
							case 7:
								LocY += 1;
								LocX -= 1;
								if (LocX <2 || LocY >picHeight-1)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}
								m_b += picWidth-1;
								index += picWidth-1;
								m_c += picWidth-1;
								m_d += picWidth-1;
								*m_d = 1;
								minP = *m_b;
								waterWay = 9;

								if (minP > *(m_b+picWidth+1) && !(*(m_d+picWidth+1)) )
								{
									waterWay = 5;minP = *(m_b+picWidth+1); 
								}
								if (minP > *(m_b-picWidth-1) && !(*(m_d-picWidth-1)) )
								{
									waterWay = 1;minP = *(m_b-picWidth-1);
								}
								if (minP > *(m_b+picWidth) && !(*(m_d+picWidth)) )
								{
									waterWay = 6;minP = *(m_b+picWidth);
								}
								if (minP > *(m_b-1) && !(*(m_d-1)) )
								{
									waterWay = 8;minP = *(m_b-1);
								}		
								if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )
								{
									minP = *(m_b+picWidth-1);
									waterWay = 7;
								}
								break;
							case 8:
								LocX -= 1;
								if (LocX < 2)
								{
									isNext = true;break;//����ڱ߽��ϣ�������
								}
								
								m_b -= 1;
								index -=1;
								m_c -= 1;
								m_d -= 1;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )
								{
									waterWay = 7;minP = *(m_b+picWidth-1);
								}
								if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)) )
								{
									waterWay = 1;
									minP = *(m_b-picWidth-1);
								}
								if (minP >= *(m_b-1) && !(*(m_d-1)) )
								{
									minP = *(m_b-1);
									waterWay = 8;
								}
								break;
							case 9:
								*m_b += num_of_drop;
								*m_c += 4000;
								isNext = true;
								break;
							}
						}

					
				}
			}
		}
	}

	delete []Label_P;
	
	return outputImg;

}

//һ��С�˲���������ͼ���е�ֵС��mimNumʱ�ͽ����Ϊ0
Mat CWaterFill::WaterFilter(Mat inputImg, int mimNum)
{
	typedef ushort imgType;
	imgType maxValue = 65535;//��ʾÿ�����ص����ֵ�������8λ���ݣ�����Ӧ����255�������16λ����Ϊ65535

	Mat outputImg(inputImg.rows, inputImg.cols, inputImg.type());//�������ͼ��

	imgType* pSrc = (imgType*)inputImg.data;//��ȡָ��
	imgType* pDest = (imgType*)outputImg.data;

	int piexlNum = inputImg.rows * inputImg.cols;

	for (int i=0; i<piexlNum; ++i,++pSrc,++pDest)
	{
		if (*pSrc <= mimNum)//���С���趨ֵ
		{
			*pDest =0;//��Ϊ0
		}
		else
		{
			*pDest = maxValue;
		}
	}
	return outputImg;
}


/****************************************************************************
����HalfSizeImg��ͼ��SrcImg��СΪԭ����һ�룬����SrcImgͼ��ĳ��Ϳ����Ϊż����
������������Ϊ16λ������ֵ��Ϊ�������������Ҳ��16λ
***********************************************************************///////
Mat CWaterFill::HalfSizeImg(Mat SrcImg)
{

	int hightImg = SrcImg.rows /2;
	int widthImg = SrcImg.cols /2;
	Mat DestImg(hightImg, widthImg, SrcImg.type());
	ushort* pSrc = (ushort*)SrcImg.data;
	ushort* pDest = (ushort*)DestImg.data;

	//����ѹ������Ϊ��ÿ�ĸ��㣬ȡ���Ͻǵ���һ��
	for (int i=0;i<hightImg; ++i, pSrc+=widthImg*2)
	{
		for (int j=0;j<widthImg;++j, pSrc +=2, ++pDest)
		{
			*pDest = *pSrc;
		}
	}
	return DestImg;
}

/****************************************************************************
����Sixth2Eight������16λ����ͼ��תΪ8λ����ͼ�񣬼���16λͼ���ÿ�����ݶ�����ratio
��ȡ�µ����ݵ�
***********************************************************************///////
Mat CWaterFill::Sixth2Eight(Mat SrcImg, int ratio)
{
	int hightImg = SrcImg.rows;
	int widthImg = SrcImg.cols;
	Mat DestImg(hightImg, widthImg, CV_8UC1);
	ushort* pSrc = (ushort*)SrcImg.data;
	uchar* pDest = DestImg.data;
	int piexlNum = hightImg * widthImg;//�ܹ���������Ŀ

	double temp = 0;
	if (ratio > 1)//�������1�����ձ�������
	{
		for (int i=0;i<piexlNum; ++i, ++pSrc, ++pDest)
		{
			if (*pSrc!=0)
			{
				temp = *pSrc/ratio;
				if (temp > 255)
				{
					*pDest = 255;
				}
				else if (temp < 0)
				{
					*pDest = 0;
				}
				else
				{
					*pDest = (uchar)temp;
				}
			}
			else
			{
				*pDest = 0;
			}
		}
	}

	else if (ratio == 0)//�������0�������⴦��
	{
		for (int i=0;i<piexlNum; ++i, ++pSrc, ++pDest)
		{
			if (*pSrc == 65535)
			{
				*pDest = 255;
			}
			else
			{
				*pDest = 0;
			}
		}
	}

	return DestImg;
}

/****************************************************************************
����AndOpera������ͼ����������������SrcImgΪ�ο�ͼ����SrcImgλ8λͼ��
DestImgΪ16λͼ�񣬾���ע��
***********************************************************************///////
void CWaterFill::AndOpera(Mat SrcImg, Mat* DestImg)//������ͼ����������
{
	typedef ushort imgType;
	uchar* pSrc = SrcImg.data;
	imgType* pDest = (imgType*)DestImg->data;
	int pixNum = SrcImg.cols * SrcImg.rows;//�ܹ�Ҫ�Ƚ϶��ٵ㣬Ӧ����ȫ����

	for (int i=0; i<pixNum; ++i, ++pSrc, ++pDest)
	{
		if (*pSrc == 0)//����Ǳ���
		{
			*pDest = 1;//���ʶ���������Ǳ���
		}
		//ֻҪ��ʶ�������������ˣ����������ù���
	}
}

/************************************************************************/
/*���д�������������ģ�ȵ�
/************************************************************************/
void CWaterFill::Water(int nFrame, int threshold, vector<RECT>* detectBox, int stype)
{
	int minSize = 0;
	int minHeadSize = 0;
	if (stype == 1)//ʹ��320*240��СͼƬ����Ч������΢��һ�㣬�ٶȻ���һЩ
	{
		minSize = 900;
		minHeadSize = 400;
	}
	else if (stype == 2)//ʹ��160*120��С��ͼƬ����Ч������΢��һ�㣬�ٶȻ��ܶ�
	{
		minSize = 300;
		minHeadSize = 60;
	}
	//resize(rgbImage, minRGB, Size(320,240));//����ɫͼ����С
	//��640*480��ͼ��ת��Ϊ320*240
	minImg = HalfSizeImg(depthMap);
	//��16λͼ��ת��Ϊ8λͼ�����ڱ�����ģ��16λͼ��Ч����
	img_8bit = Sixth2Eight(minImg, 40);
	///��ʼ������ģ�����������������ɵ�ǰ�������ڡ�m_fgmask����
	if (nFrame<200)//����ǰ200֡�����м�ǿ����ѧϰ
	{
		GMM2(img_8bit, nFrame, 0.1);//ѧϰ�ʱȽϸߣ����������ױ�ѧ��ȥ
	}
	else
	{
		GMM2(img_8bit, nFrame, 0.0001);//ѧϰ�ʱȽϵͣ����������ױ仯�����������˳�ʱ��վ��������ͷ�µ����
	}
	BgImg = ContourFilter(m_fgmask, minSize);//ǰ���˲�������̫С������
	//imshow("BgImg",BgImg);
	//�ñ�����ģ���ͼ����Ϊ�ο���ȡ��16λͼ���ǰ��
	AndOpera(BgImg, &minImg);  //����Ϊ8λͼ���������Ὣ��������Ϊ1������������Ϊ0
	//��ǰ���к�ɫ��������������
	MergeBlack(&minImg);//ע�����ﱳ������Ϊ1

	////////�������ĵ�ʱ�̵��ˣ���ʼ��ˮ��
	if (stype == 1)//ʹ��320*240��СͼƬ����Ч������΢��һ�㣬�ٶȻ���һЩ
	{
		///�еȴ�СͼƬ����//////////////////////////////////////////////////////////////////////////////////////
		img_16bit = WaterDrop(minImg, threshold);//ÿ�����������ϵĵ㣬����ٽ�ȥ
		waterImg = WaterFilter(img_16bit, 8000);//�㷨���˲�����������趨ֵ��Ϊǰ�������ֵ��������Ǹ���������һ������
		BgImg = Sixth2Eight(waterImg, 0);//ת��Ϊ8λ������һ������
		waterImg_8bit = ContourFilter(BgImg, minHeadSize);//ǰ���˲�
		GetHead(waterImg_8bit, detectBox);//�ڲ�ɫͼ���ϱ�עһ��ͷ��λ��
		////////////////////////////////////////////////////////////////////////////////////////////////
	}
	else if (stype == 2)//ʹ��160*120��С��ͼƬ����Ч������΢��һ�㣬�ٶȻ��ܶ�
	{
		//ʹ��minͼƬ���д���
		superminImg = HalfSizeImg(minImg);//��ͼƬ����СΪԭ���ķ�֮һ���ӿ촦���ٶ�
		supermin_16bit = WaterDrop(superminImg, threshold);//WaterFill��������ÿһ��ˮ��λ4000�����׿���
		superminWater = WaterFilter(supermin_16bit, 8000);//�˲���С��8000�Ķ�ȥ��
		supermin_8bit = Sixth2Eight(superminWater, 0);//ת��Ϊ8λ��������һ������
		supermin_Water8bit = ContourFilter(supermin_8bit, minHeadSize);//ǰ���˲���С�ڵڶ��������Ķ�ȥ��
		GetHead_Min(supermin_Water8bit,detectBox);//��ʾһ��ͷ��ͬʱע����������ͼ���С��һ��
	}	
}