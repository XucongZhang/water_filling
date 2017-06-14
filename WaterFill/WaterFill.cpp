#include "WaterFill.h"

void CWaterFill::Initialise()
{
	depthMap.create(480,640,CV_16UC1); //存储深度图，初始化一下，这里是16位的单通道，但是其实这里的初始化是没有用的，但求心理安慰
	minRGB.create(240, 320, CV_8UC3);//缩小后的彩色图像
	save_minRGB.create(240, 320, CV_8UC3);//用于保存节省空间
	minImg.create(240, 320, CV_16UC1);
	waterImg.create(240, 320, CV_16UC1);
	img_16bit.create(240,320, CV_16UC1);//定义一个16位图像，用于中间计算
	img_8bit.create(240, 320, CV_8UC1);//定义一个8位图像，用于中间计算
	waterImg_8bit.create(240, 320, CV_8UC1);//8bit的最终结果，用于显示
	BgImg.create(240, 320, CV_8UC1);//存储背景建模之后的图像

	//定义一些更小的图片用于WaterFill
	superminImg.create(120,160,CV_16UC1);
	superminWater.create(120,160,CV_16UC1);
	supermin_16bit.create(120,160,CV_16UC1);
	supermin_8bit.create(120,160,CV_16UC1);
	supermin_Water8bit.create(120,160,CV_16UC1);
}

//将最终得到的脑袋头上都标上一个小框框。输出为三通道彩色，头顶为黄色
void CWaterFill::GetHead_Min(Mat Img, vector<RECT>* detectBox)
{
	CvMemStorage* m_storage;
	m_storage = cvCreateMemStorage(0);//开辟默认大小的空间

	CvSeq *m_first_seq = NULL;
	CvSeq *m_prev_seq = NULL;
	CvSeq *m_seq = NULL;

	CvPoint pt1,pt2;//画黄色框的两个顶点

	Mat inputcopy;
	Img.copyTo(inputcopy);//寻找轮廓操作会对原始图像做变化，故复制一个
	IplImage* pp = &IplImage(inputcopy);//首先要转换一下，因为cvFindContours函数不适合采用Mat类型数据

	//////////////////////////////////////////////////////////////////
	cvClearMemStorage(m_storage);
	cvFindContours(pp, m_storage, &m_first_seq, sizeof(CvContour), CV_RETR_LIST );//寻找所有轮廓，会改变输入图像
	for( m_seq = m_first_seq; m_seq; m_seq = m_seq->h_next )
	{
		CvContour* cnt = (CvContour*)m_seq;
		pt1.x = (cnt->rect.x)*2;//画一个框
		pt1.y = (cnt->rect.y)*2;
		pt2.x = (cnt->rect.x + cnt->rect.width)*2;
		pt2.y = (cnt->rect.y + cnt->rect.height)*2;
		rectangle(img_8bit, pt1, pt2, CV_RGB(255,255,255), -1);//最后一个参数小于0的时候填充整个区域

		RECT head;
		head.left = pt1.x;
		head.right = pt2.x;
		head.top = pt1.y;
		head.bottom = pt2.y;
		detectBox->push_back(head);
	}
	//////////////////////////////////////////////////////////////////////////
	cvReleaseMemStorage(&m_storage);//释放空间
}


void CWaterFill::GetHead(Mat Img, vector<RECT>* detectBox)
{
	CvMemStorage* m_storage;
	m_storage = cvCreateMemStorage(0);//开辟默认大小的空间

	CvSeq *m_first_seq = NULL;
	CvSeq *m_prev_seq = NULL;
	CvSeq *m_seq = NULL;

	CvPoint pt1,pt2;//画黄色框的两个顶点

	Mat inputcopy;
	Img.copyTo(inputcopy);//寻找轮廓操作会对原始图像做变化，故复制一个
	IplImage* pp = &IplImage(inputcopy);//首先要转换一下，因为cvFindContours函数不适合采用Mat类型数据

	//////////////////////////////////////////////////////////////////
	cvClearMemStorage(m_storage);
	cvFindContours(pp, m_storage, &m_first_seq, sizeof(CvContour), CV_RETR_LIST );//寻找所有轮廓，会改变输入图像
	for( m_seq = m_first_seq; m_seq; m_seq = m_seq->h_next )
	{
		CvContour* cnt = (CvContour*)m_seq;
		pt1.x = (cnt->rect.x);//画一个框
		pt1.y = (cnt->rect.y);
		pt2.x = (cnt->rect.x + cnt->rect.width);
		pt2.y = (cnt->rect.y + cnt->rect.height);

		RECT head;
		head.left = pt1.x;
		head.right = pt2.x;
		head.top = pt1.y;
		head.bottom = pt2.y;
		detectBox->push_back(head);

		rectangle(img_8bit, pt1, pt2, CV_RGB(255,255,255), -1);//最后一个参数小于0的时候填充整个区域
	}
	//////////////////////////////////////////////////////////////////////////
	cvReleaseMemStorage(&m_storage);//释放空间
}
/************************************************************************/
/*MergeBlack函数用来将头顶上那些不可识别的黑色区域插值为可利用的数据,
输入的图像中数值为0的即为无数据点，数值为1的即为背景，其他为前景
/************************************************************************/
void CWaterFill::MergeBlack(Mat* pImg)
{
	//不确定使用16位图像还是8位图像，修改此处即可
	typedef ushort imgType;
	//用于设置检测窗口半径大小，一般稍微比脑袋大一点即可，设置参数！
	int radius= 30;

	double pData = 0;//储存总和，一定要大
	int num = 0;
	int staPoint_x = 0;//设置检测的初始和结束点
	int staPoint_y = 0;
	int endPoint_x = 0;
	int endPoint_y = 0;
	int width = pImg->cols;
	int height = pImg->rows;

	imgType* m_a = NULL;//定义指针
	imgType* m_b = NULL;//定义指针
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////用平均值填充脑袋上的黑色区域
	m_a = (imgType*)pImg->data;
	for (int y=0; y<height; ++y) //遍历整幅图像
	{		
		for (int x=0; x<width; ++x, ++m_a) 
		{	
			if (*m_a == 0)//如果是无数据区域
			{
				//////确定起始和结束点////////////////////////////
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

				///////////以无数据点为中心，计算周围像素的平均值填充到无数据点
				m_b = (imgType*)pImg->data;
				m_b += staPoint_y*width;
				for (int i=staPoint_y; i<endPoint_y; ++i,m_b+=width)//以（x，y）为中心寻找
				{
					for (int j=staPoint_x; j<endPoint_x; ++j)
					{
						if (m_b[j] !=1 && m_b[j] !=0)//如果既不是背景又不是无数据点
						{
							pData += m_b[j];
							++num;
						}
					}
				}
				pData = pData/num;//计算平均值
				pImg->at<imgType>(y,x) = (imgType)pData;
				pData = 0;
				num = 0;
			} 
		}
	}
}

/****************************************************************************
函数ContourFilter用来消除前景中面积小于一定大小的区域，
其中输入参数Img为输入图像，minSize为最小面积，返回值为处理后的图像。注意其中
输入和输出必须为8位数据图像，且输出图像为二值化图像
***********************************************************************///////
Mat CWaterFill::ContourFilter(Mat Img, int minSize)
{
	int maxValue = 255;//标示每个像素的最大值。如果是8位数据，这里应该是255，如果是16位，则为65535
	//消除前景区域所用变量
	CvMemStorage* m_storage;
	m_storage = cvCreateMemStorage(0);//开辟默认大小的空间

	int m_region_count = 0;
	CvSeq *m_first_seq = NULL;
	CvSeq *m_prev_seq = NULL;
	CvSeq *m_seq = NULL;

	IplImage* ppp = &IplImage(Img);//首先要转换一下，因为cvFindContours函数不适合采用Mat类型数据
	cvMorphologyEx( ppp, ppp, 0, 0, CV_MOP_OPEN , 1);
	cvMorphologyEx( ppp, ppp, 0, 0, CV_MOP_CLOSE, 1);
	//////////////////////////////////////////////////////////////////
	cvClearMemStorage(m_storage);
	cvFindContours(ppp, m_storage, &m_first_seq, sizeof(CvContour), CV_RETR_LIST );//寻找所有轮廓
	for( m_seq = m_first_seq; m_seq; m_seq = m_seq->h_next )
	{
		CvContour* cnt = (CvContour*)m_seq;
		if( cnt->rect.width * cnt->rect.height < minSize)//设置参数
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

	Mat bgImg(ppp, 0);//然后再转换回来

	cvReleaseMemStorage(&m_storage);//释放空间
	return bgImg;
}

/****************************************************************************
函数GMM2用来背景建模，使用OpenCV库中的函数
***********************************************************************///////
void CWaterFill::GMM2(Mat InputImg, int nFrame, double learnRate)
{
	m_bg_model(InputImg, m_fgmask, learnRate);//m_fgmask即为前景
}
/************************************************************************/
/*主要算法函数，注水
/************************************************************************/
Mat CWaterFill::WaterDrop(Mat InputImg, int num_of_drop)
{
	typedef ushort imgType;//不确定使用16位图像还是8位图像，修改此处即可
	imgType maxValue = 65535;//标示每个像素的最大值。如果是8位数据，这里应该是255，如果是16位，则为65535

	imgType* m_a = NULL;
	imgType* m_b = NULL;
	imgType* m_c = NULL;
	int LocX;
	int LocY;

	int minP = maxValue;
	int picWidth = InputImg.cols;
	int picHeight = InputImg.rows;

	//初始化输出图像矩阵
	Mat outputImg(picHeight,picWidth, InputImg.type());
	memset(outputImg.data, 0, picWidth * picHeight*sizeof(imgType));//清零
	//m_c = (imgType*)outputImg.data;
	
	uchar*  Label_P = new uchar[picWidth * picHeight];//用于标识是否之前移动到过这个点
	memset(Label_P, 0, picWidth * picHeight);//清零
	uchar* m_d = NULL;

	Mat inputcopy(picHeight,picWidth, InputImg.type());//因为如果直接对输入图像操作，会造成原始图像变化
	InputImg.copyTo(inputcopy);
	m_a = (imgType*)inputcopy.data;
	m_b = (imgType*)inputcopy.data;///原始图片指针

	int index = 0;

	////将所有背景和无数据点都设置为最大，不参与比较
	for (int i=0; i<picWidth*picHeight; ++i,++m_a)
	{
		if (*m_a < 2)//即对于那些背景和无数据点
		{
			*m_a = maxValue;
		}
	}

	//恢复指针
	m_a = (imgType*)inputcopy.data;

	int waterWay = 0;
	bool isNext = false;//是否进行下一个

	//////////////////////////////////////////////////////////////
	for (int j=0; j<picHeight; j+=5,m_a+=picWidth*5) //遍历整个图片，全部撒点
	{
		for (int i=0; i<picWidth; i+=5)//每N个点为一组，加快处理速度
		{
			if (m_a[i] != maxValue)/////遇到前景才进行处理//如果为8位图像，则为255
			{
				////初始化
				LocX = i;
				LocY = j;

				m_b = (imgType*)inputcopy.data;///原始图片指针
				m_b += picWidth*LocY + LocX;//确定指针
				index = 0;
				index += picWidth*LocY + LocX; 
				m_c = (imgType*)outputImg.data;
				m_c += picWidth*LocY + LocX;//确定指针
				m_d = Label_P;
				m_d += picWidth*LocY + LocX;//确定指针

				for (int k=0; k<25; ++k)//每N个点为一组
				{
					if (LocX>picWidth-1 || LocX<2 || LocY>picHeight-1 || LocY<2)//如果在边界点上我就不管啦！
					{
						break;
					}

					memset(Label_P, 0, picWidth * picHeight);//清零
					isNext = false;//初始化一下，运行新的一个点
					//memset(Label_P, 0, picWidth * picHeight*sizeof(imgType));

					//首先在初始点周围检测8个像素点
						//左上角
						minP = *m_b;
						waterWay = 9;

						if (minP >= *(m_b-picWidth-1))
						{
							waterWay = 1;
							minP =  *(m_b-picWidth-1);
						}
						if (minP >= *(m_b-picWidth))//上方
						{
							waterWay = 2;
							minP = *(m_b-picWidth);
						}
						if (minP >= *(m_b-picWidth+1))//右上方
						{
							waterWay = 3;
							minP = *(m_b-picWidth+1);
						}
						if (minP >= *(m_b+1))//右方
						{
							waterWay = 4;
							minP = *(m_b+1);
						}
						if (minP >= *(m_b+picWidth+1))//右下方
						{
							waterWay = 5;
							minP = *(m_b+picWidth+1);
						}
						if (minP >= *(m_b+picWidth))//下方
						{
							waterWay = 6;
							minP = *(m_b+picWidth);
						}
						if (minP >= *(m_b+picWidth-1))//左下方
						{
							waterWay = 7;
							minP = *(m_b+picWidth-1);
						}
						if (minP >= *(m_b-1))//左方
						{
							waterWay = 8;
							minP =  *(m_b-1);
						}

						while(!isNext)//如果还是没有找到一个合适的点
						{
							switch(waterWay)
							{
							case 1:
								LocX -= 1;
								LocY -= 1;
								if (LocX<2 || LocY <2)
								{
									isNext = true;break;//如果在边界上，就跳出
								}

								m_b = m_b-picWidth-1;//移动到1号位
								index = index-picWidth-1;
								m_c = m_c-picWidth-1;
								m_d = m_d-picWidth-1;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)))//左下方
								{
									waterWay = 7;minP = *(m_b+picWidth-1);
								}
								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))//右上方
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b-1) && !(*(m_d-1)))//左方
								{
									waterWay = 8;minP = *(m_b-1);
								}
								if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))//上方
								{
									waterWay = 2;minP = *(m_b-picWidth);
								}
								if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))//放在最后的这个有最大的优先级，体现了“惯性”
								{
									minP = *(m_b-picWidth-1);//左上角
									waterWay = 1;
								}							
								break;
							case 2:
								LocY -= 1;
								if (LocY < 2)
								{
									isNext = true;break;//如果在边界上，就跳出
								}
								m_b -= picWidth;//2号位
								index -= picWidth;
								m_c -= picWidth;
								m_d -= picWidth;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b-picWidth-1) && !(*(m_d-picWidth-1)))//右上角
								{
									waterWay = 1;minP = *(m_b-picWidth-1);
								}
								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))//左上角
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))//最后的反而有最大的优先级，哦也
								{
									minP = *(m_b-picWidth);//上方
									waterWay = 2;
								}
								break;
							case 3:
								LocY -= 1;
								LocX += 1;
								if (LocY<2 || LocX>picWidth-1)
								{
									isNext = true;break;//如果在边界上，就跳出
								}
								m_b = m_b-picWidth+1;//3号位
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
								if (minP >= *(m_b-picWidth) && !(*(m_d-picWidth)))//上方
								{
									waterWay = 2;minP = *(m_b-picWidth);
								}
								if (minP >= *(m_b+1) && !(*(m_d+1)))
								{
									waterWay = 4;minP = *(m_b+1);
								}
								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)))
								{
									minP = *(m_b-picWidth+1);//右上角
									waterWay = 3;
								}
								break;
							case 4:
								LocX += 1;
								if (LocX >picWidth-1)
								{
									isNext = true;break;//如果在边界上，就跳出
								}
								m_b += 1;
								index +=1;
								m_c += 1;
								m_d += 1;
								*m_d = 1;

								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth)))//右上角
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)))//右上角
								{
									waterWay = 5;minP = *(m_b+picWidth+1);
								}
								if (minP >= *(m_b+1) && !(*(m_d+1)))
								{
									minP = *(m_b+1);//右上角
									waterWay = 4;
								}
								break;
							case 5:
								LocX += 1;
								LocY += 1;
								if (LocX >picWidth-1 || LocY > picHeight-1)
								{
									isNext = true;break;//如果在边界上，就跳出
								}
								m_b += picWidth+1;
								index += picWidth+1;
								m_c += picWidth+1;
								m_d += picWidth+1;
								*m_d = 1;
								minP = *m_b;
								waterWay = 9;

								if (minP >= *(m_b-picWidth+1) && !(*(m_d-picWidth+1)) )//右上角
								{
									waterWay = 3;minP = *(m_b-picWidth+1);
								}
								if (minP >= *(m_b+picWidth-1) && !(*(m_d+picWidth-1)) )//左下方
								{
									waterWay = 7;minP = *(m_b+picWidth-1);
								}
								if (minP >= *(m_b+1) && !(*(m_d+1)))//右方
								{
									waterWay = 4;minP =  *(m_b+1);
								}
								if (minP >= *(m_b+picWidth) && !(*(m_d+picWidth)) )//下方
								{
									waterWay = 6; minP = *(m_b+picWidth);
								}
								if (minP >= *(m_b+picWidth+1) && !(*(m_d+picWidth+1)) )
								{
									minP = *(m_b+picWidth+1);//右下角
									waterWay = 5;
								}
								break;
							case 6:
								LocY += 1;
								if (LocY > picHeight-1)
								{
									isNext = true;break;//如果在边界上，就跳出
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
									minP = *(m_b+picWidth);//下方
									waterWay = 6;
								}
								break;
							case 7:
								LocY += 1;
								LocX -= 1;
								if (LocX <2 || LocY >picHeight-1)
								{
									isNext = true;break;//如果在边界上，就跳出
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
									isNext = true;break;//如果在边界上，就跳出
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

//一个小滤波，当输入图像中的值小于mimNum时就将其变为0
Mat CWaterFill::WaterFilter(Mat inputImg, int mimNum)
{
	typedef ushort imgType;
	imgType maxValue = 65535;//标示每个像素的最大值。如果是8位数据，这里应该是255，如果是16位，则为65535

	Mat outputImg(inputImg.rows, inputImg.cols, inputImg.type());//定义输出图像

	imgType* pSrc = (imgType*)inputImg.data;//获取指针
	imgType* pDest = (imgType*)outputImg.data;

	int piexlNum = inputImg.rows * inputImg.cols;

	for (int i=0; i<piexlNum; ++i,++pSrc,++pDest)
	{
		if (*pSrc <= mimNum)//如果小于设定值
		{
			*pDest =0;//则为0
		}
		else
		{
			*pDest = maxValue;
		}
	}
	return outputImg;
}


/****************************************************************************
函数HalfSizeImg将图像SrcImg缩小为原来的一半，其中SrcImg图像的长和宽必须为偶数，
且其数据类型为16位。返回值即为结果，数据类型也是16位
***********************************************************************///////
Mat CWaterFill::HalfSizeImg(Mat SrcImg)
{

	int hightImg = SrcImg.rows /2;
	int widthImg = SrcImg.cols /2;
	Mat DestImg(hightImg, widthImg, SrcImg.type());
	ushort* pSrc = (ushort*)SrcImg.data;
	ushort* pDest = (ushort*)DestImg.data;

	//这里压缩方法为：每四个点，取左上角的那一个
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
函数Sixth2Eight用来把16位数据图像转为8位数据图像，即把16位图像的每个数据都除以ratio
获取新的数据点
***********************************************************************///////
Mat CWaterFill::Sixth2Eight(Mat SrcImg, int ratio)
{
	int hightImg = SrcImg.rows;
	int widthImg = SrcImg.cols;
	Mat DestImg(hightImg, widthImg, CV_8UC1);
	ushort* pSrc = (ushort*)SrcImg.data;
	uchar* pDest = DestImg.data;
	int piexlNum = hightImg * widthImg;//总共的像素数目

	double temp = 0;
	if (ratio > 1)//如果大于1，则按照倍数来除
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

	else if (ratio == 0)//如果等于0，则特殊处理
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
函数AndOpera对两幅图像进行与操作，其中SrcImg为参考图像，且SrcImg位8位图像，
DestImg为16位图像，敬请注意
***********************************************************************///////
void CWaterFill::AndOpera(Mat SrcImg, Mat* DestImg)//对两幅图像进行与操作
{
	typedef ushort imgType;
	uchar* pSrc = SrcImg.data;
	imgType* pDest = (imgType*)DestImg->data;
	int pixNum = SrcImg.cols * SrcImg.rows;//总共要比较多少点，应该是全部的

	for (int i=0; i<pixNum; ++i, ++pSrc, ++pDest)
	{
		if (*pSrc == 0)//如果是背景
		{
			*pDest = 1;//则标识出来这里是背景
		}
		//只要标识出来背景就行了，其它都不用管了
	}
}

/************************************************************************/
/*进行处理，包括背景建模等等
/************************************************************************/
void CWaterFill::Water(int nFrame, int threshold, vector<RECT>* detectBox, int stype)
{
	int minSize = 0;
	int minHeadSize = 0;
	if (stype == 1)//使用320*240大小图片处理，效果会稍微好一点，速度会慢一些
	{
		minSize = 900;
		minHeadSize = 400;
	}
	else if (stype == 2)//使用160*120大小的图片处理，效果会稍微差一点，速度会快很多
	{
		minSize = 300;
		minHeadSize = 60;
	}
	//resize(rgbImage, minRGB, Size(320,240));//将彩色图像缩小
	//将640*480的图像转换为320*240
	minImg = HalfSizeImg(depthMap);
	//将16位图像转换为8位图像，用于背景建模，16位图像效果差
	img_8bit = Sixth2Eight(minImg, 40);
	///开始背景建模啦！！！！其中生成的前景保存在“m_fgmask”中
	if (nFrame<200)//对于前200帧，进行加强背景学习
	{
		GMM2(img_8bit, nFrame, 0.1);//学习率比较高，背景很容易被学进去
	}
	else
	{
		GMM2(img_8bit, nFrame, 0.0001);//学习率比较低，背景不容易变化，适用于有人长时间站立在摄像头下的情况
	}
	BgImg = ContourFilter(m_fgmask, minSize);//前景滤波，消除太小的区域
	//imshow("BgImg",BgImg);
	//用背景建模后的图像作为参考来取得16位图像的前景
	AndOpera(BgImg, &minImg);  //输入为8位图像，这个步骤会将背景设置为1，无数据区域为0
	//将前景中黑色的区域消除掉。
	MergeBlack(&minImg);//注意这里背景像素为1

	////////激动人心的时刻到了！开始灌水！
	if (stype == 1)//使用320*240大小图片处理，效果会稍微好一点，速度会慢一些
	{
		///中等大小图片处理//////////////////////////////////////////////////////////////////////////////////////
		img_16bit = WaterDrop(minImg, threshold);//每次遇到该填上的点，填多少进去
		waterImg = WaterFilter(img_16bit, 8000);//算法的滤波，如果大于设定值才为前景，这个值和上面的那个参数不是一个东西
		BgImg = Sixth2Eight(waterImg, 0);//转换为8位用于下一步计算
		waterImg_8bit = ContourFilter(BgImg, minHeadSize);//前景滤波
		GetHead(waterImg_8bit, detectBox);//在彩色图像上标注一下头的位置
		////////////////////////////////////////////////////////////////////////////////////////////////
	}
	else if (stype == 2)//使用160*120大小的图片处理，效果会稍微差一点，速度会快很多
	{
		//使用min图片进行处理
		superminImg = HalfSizeImg(minImg);//把图片再缩小为原来四分之一，加快处理速度
		supermin_16bit = WaterDrop(superminImg, threshold);//WaterFill主函数，每一个水滴位4000，容易看清
		superminWater = WaterFilter(supermin_16bit, 8000);//滤波，小于8000的都去掉
		supermin_8bit = Sixth2Eight(superminWater, 0);//转换为8位，进行下一步计算
		supermin_Water8bit = ContourFilter(supermin_8bit, minHeadSize);//前景滤波，小于第二个参数的都去掉
		GetHead_Min(supermin_Water8bit,detectBox);//标示一下头，同时注意两个输入图像大小不一致
	}	
}