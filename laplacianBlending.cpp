/*
 * ************************************************************************************
 *
 *       Filename:  laplacianBlending.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/12/2012 09:34:49 AM
 *       Revision:  none
 *       Compiler:  gcc
 *          Email:  huaijin511@gmail.com
 *         Author:  Huaijin(Chao Chen) (GUN)
 *   Organization:  Wu Han University (China)
 *
 * ************************************************************************************
 */
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <string>
#define  LEVELS 4
using namespace std;

void buildLaplacianPry( IplImage* src, IplImage** LPyr, int levels );

void reConstruct ( IplImage** LS, IplImage* dst, int levels );
/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  blendImages
 *  Description:
 *				//1. create LA & LB
 *				//2. create GB
 *				//3. costruct LS from LA, LB, with weigth GB
 *				//4. construct blended image from LS
 *        Input:  image A & B ; R is the overlap region
 *       Output:  blended image dst
 * *****************************************************************************
 */
int blendImages ( IplImage* A, IplImage* B, IplImage* R, IplImage* dst, int levels )
{
	//check param
	if (A->width != B->width || A->height != B->height )
	{
		cout<< "the size of A is not same with the size of B"<<endl;
		exit( 1 );
	}
	if ( levels < 2 )
	{
		cout << "levels must >= 2" << endl;
		exit( 1 );
	}
	if ( A->nChannels != B->nChannels )
	{
		cout << "error : fail in A.type == B.type  " << endl;
		exit( 1 );
	}
	cout << "entery blending" <<endl;

	int i, h, w, l;
	IplImage* GR[LEVELS];
	CvScalar curr_ls, curr_la, curr_lb, curr_gr;
	
	int depth = IPL_DEPTH_32F;
	int channels = A->nChannels;
	
	// get GB  
	IplImage* mask8U = cvCreateImage( cvGetSize(R), IPL_DEPTH_8U, 1  );
	IplImage*   mask = cvCreateImage( cvGetSize(R), IPL_DEPTH_32F, 1 ); 

	cvCvtColor( R, mask8U, CV_BGR2GRAY);  
	cvConvertScale( mask8U, mask, 1.0/255 );
	GR[0] = cvCloneImage( mask );

	//cvSaveImage("GRRRR.png",GR[0]);
	for ( i = 1; i < levels; i++ )
	{
		GR[i] = cvCreateImage( cvSize(GR[i-1]->width/2,GR[i-1]->height/2), GR[0]->depth, GR[0]->nChannels );
		cvPyrDown( GR[i-1], GR[i], CV_GAUSSIAN_5x5 );
		//GR[i] = cvCloneImage( down );
	}
	for( i = 0 ; i < levels; i++ ){
		/* -------------------------------show image ----------------------------------------- */
		IplImage* GR_8U = cvCreateImage( cvGetSize(GR[i]), IPL_DEPTH_8U, 1 );
		cvConvertScale( GR[i], GR_8U, 255 );
		cvNamedWindow("mask", CV_WINDOW_AUTOSIZE );
		cvShowImage ( "mask", GR_8U );
		cvWaitKey(0);
		cvDestroyWindow("mask");
		cvReleaseImage( &GR_8U );
		/* ----------------------------------end show image ---------------------------------- */
	}
	IplImage* LA[LEVELS];
	IplImage* LB[LEVELS];
	IplImage* LS[LEVELS];
	//get LA
	buildLaplacianPry( A , LA, levels );
	//get LB
	buildLaplacianPry( B, LB, levels );
		
	// construct LS 
	for ( l = 0; l < levels; l++ )
	{
		int width  = LA[l]->width;
		int height = LA[l]->height; 
		//init LS[i]
		LS[l] = cvCreateImage( cvSize(width,height), depth, channels );

		for ( h = 0; h < height; h++ )
		{
			for ( w = 0; w < width; w++ )
			{
				// LS(i,j) = GB(i,j)*lA(i,j) + (1-GB(i,j)*LB(i,j)) 
				curr_gr = cvGet2D( GR[l], w, h );
				curr_la = cvGet2D( LA[l], w, h );
				curr_lb = cvGet2D( LB[l], w, h );

				curr_ls.val[0] = curr_gr.val[0] * curr_la.val[0] + (1-curr_gr.val[0])*curr_lb.val[0];
				if (  channels ==3 ){
					curr_ls.val[1] = curr_gr.val[0] * curr_la.val[1] + (1.0-curr_gr.val[0])*curr_lb.val[1];
					curr_ls.val[2] = curr_gr.val[0] * curr_la.val[2] + (1.0-curr_gr.val[0])*curr_lb.val[2];
				}/* fi */
				cvSet2D( LS[l], w, h, curr_ls );
			}/* endfor */

		}/* endfor */
		cout << "construct new Image " << endl;
		//最后一层的结果很诡异
		/* -------------------------------show image ----------------------------------------- */
		IplImage* temp_8U = cvCreateImage( cvGetSize(LS[l]), IPL_DEPTH_8U, channels );
		cvConvertScale( LS[l], temp_8U, 255 );
		cvNamedWindow("LS", CV_WINDOW_AUTOSIZE );
		cvShowImage ( "LS", temp_8U );
		cvWaitKey(0);
		cvDestroyWindow("LS");
		cvReleaseImage( &temp_8U );
		/* ----------------------------------end show image ---------------------------------- */

	}/* endfor */	

	reConstruct( LS, dst, levels );
	cout << "exit blendImag()" <<endl;
	return 0;
}
/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  reConstruct
 *  Description:  construct  new image from laplacian pryimd(LS)
 * *****************************************************************************
 */
 
void reConstruct ( IplImage** LS, IplImage* dst, int levels )
{

	cout << "entery reconstruct()" <<endl;
	int i;
	int depth = IPL_DEPTH_32F;
	int channels = dst->nChannels;
	IplImage* dst_32F = cvCreateImage( cvGetSize(dst), depth, channels );
	//CvSize curr_size = cvSize( LS[levels-1]->width, LS[levels-1]->height );
	//IplImage* curr = cvCreateImage( curr_size, IPL_DEPTH_8U, 1 );
	IplImage* curr = cvCloneImage( LS[levels-1] );

	for ( i = levels-1; i >= 1; i-- )
	{
		CvSize up_size = cvSize( LS[i-1]->width, LS[i-1]->height );
		IplImage* up = cvCreateImage( up_size, depth, channels  );
		//cout << "LS[" << i << "]'s channel is "<< LS[i]->nChannels << endl;  

		cvPyrUp( curr, up, CV_GAUSSIAN_5x5 );
		curr = up;

		cvAdd( LS[i-1], up, curr , NULL );

		if ( i == 1 ){ 
			cvCopy( curr, dst_32F );
			cvConvertScale( dst_32F, dst, 255  );
		}
	}
	/*
	cvNamedWindow("ok", CV_WINDOW_AUTOSIZE );
	cvShowImage ( "ok", dst );
	cvWaitKey(0);
	cvDestroyWindow("ok");
	*/
	cvReleaseImage( &curr );
	cvReleaseImage( &dst_32F );

	cout << "exit reconstruct()" <<endl;
}

/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  buildLaplacianPry
 *  Description:  
 *		 Input :	
 * *****************************************************************************
 */
void buildLaplacianPry( IplImage* src, IplImage** LPyr, int levels )
{
	cout << "enter build Laplacian Pry" << endl;
	int depth = IPL_DEPTH_32F;
	int channels = src->nChannels;
	IplImage* currentImage = cvCreateImage(cvGetSize(src), depth, channels ); 
	IplImage* src_32F = cvCreateImage( cvGetSize(src), depth, channels );
	cvConvertScale( src,  src_32F, 1.0/255  );
	cvCopy( src_32F, currentImage );
	//create currentImage as the gray Image of src
    //cvCvtColor(src,currentImage,CV_BGR2GRAY);  
	int i;

	//LA(i) = GA(i) - expand(GA(i+1))
	for ( i = 0; i < levels-1; i++ )
	{
		//the size of next level image
		CvSize down_size = cvSize(currentImage->width/2, currentImage->height/2);
		IplImage* down = cvCreateImage( down_size, depth, channels );
		cout << "before down" << endl;
		cvPyrDown( currentImage, down, CV_GAUSSIAN_5x5 );
		cout << "after down" << endl;
		IplImage* up = cvCreateImage( cvGetSize(currentImage), depth, channels );
		cvPyrUp( down, up, CV_GAUSSIAN_5x5 );

		LPyr[i] = cvCreateImage( cvGetSize(currentImage), depth, channels );

		cvSub( currentImage, up, LPyr[i], NULL );

		currentImage = down;
		//当数据比较大的时间，会不会内存泄漏
		//cvReleaseImage( &up );
		//cvReleaseImage( &down );
		/* --------------------------show images-------------------------------------- */
		IplImage* temp_8Ui = cvCreateImage( cvGetSize(LPyr[i]), IPL_DEPTH_8U, channels );
		cvConvertScale( LPyr[i], temp_8Ui, 255 );
		cvNamedWindow("pyr", CV_WINDOW_AUTOSIZE );
		cvShowImage ( "pyr", temp_8Ui );
		cvWaitKey(0);
		cvDestroyWindow("pry");
		cvReleaseImage( &temp_8Ui );

		/* --------------------------end show images-------------------------------------- */
	}
	LPyr[levels-1] = cvCreateImage( cvGetSize(currentImage), depth, channels );
	//get the lastest level --->Lpyr[last_index]
	cout << "in build Channel is " << LPyr[levels-1]->nChannels << endl;
	cvCopyImage( currentImage, LPyr[levels-1] );

	/* ---------------------------show images---- */
	IplImage* temp_8U = cvCreateImage( cvGetSize(LPyr[levels-1]), IPL_DEPTH_8U, channels );
	cvConvertScale( LPyr[levels-1], temp_8U, 255 );
	cvNamedWindow("pyr", CV_WINDOW_AUTOSIZE );
	cvShowImage ( "pyr", temp_8U);
	cvWaitKey(0);
	cvDestroyWindow("pry");
	cvReleaseImage( &temp_8U );
	/* ----------------------------end show images------ */

	cvReleaseImage( &currentImage );
	cout << "exit build Laplacian Pyr" << endl;
}
/* -------------------test modle----------------------------------------------- */
int test_fun_buildLaplacianPry() /*{{{*/
{
	int levels = 4;
	int i;
	IplImage* LaplacianPry[4];
	IplImage* src = cvLoadImage( "love.jpg" );
	//IplImage* temp = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 ); 
    //cvCvtColor(src,temp,CV_BGR2GRAY);  
	buildLaplacianPry( src, LaplacianPry, levels );
	
	IplImage* dst = cvCreateImage( cvGetSize(src), src->depth, src->nChannels );
	reConstruct( LaplacianPry, dst, 4 );

	cvSaveImage("okresult.bmp", dst);

	//for( i = 0; i < levels; i++ )
	//{
		/*cvSaveImage("level0.png", LaplacianPry[0] );
		cvSaveImage("level1.png", LaplacianPry[1] );
		cvSaveImage("level2.png", LaplacianPry[2] );
		cvSaveImage("level3.png", LaplacianPry[3] );
		*/
	//}
//	cvReleaseImage( &temp );
	cvReleaseImage( &src );
	return 0;
}/*}}}*/

int test_fun_reConstruct()
{
	IplImage* LS[4];
	IplImage* temp;

	LS[0] = cvLoadImage( "./dataShop/eyelevel0.png" );
	cout << "channel is " << LS[0]->nChannels <<endl;
    //cvCvtColor(,currentImage,CV_BGR2GRAY);  
	LS[1] = cvLoadImage( "./dataShop/eyelevel1.png" );
	LS[2] = cvLoadImage( "./dataShop/eyelevel2.png" );
	LS[3] = cvLoadImage( "./dataShop/eyelevel3.png" );

	IplImage* dst = cvCreateImage( cvGetSize(LS[3]), IPL_DEPTH_32F, 1 );

	reConstruct( LS, dst, 4 );

	cvSaveImage( "testConstruct_fun.png", dst );

	return 0;	
}

int test_fun_blendImage()
{
	int levels = 4;

	IplImage* A = cvLoadImage( "eye.bmp" );
	IplImage* B = cvLoadImage( "hand.bmp" );
	IplImage* R = cvLoadImage( "mask.bmp" );

	int channels = A->nChannels;
	IplImage* dst = cvCreateImage( cvGetSize(A), A->depth, A->nChannels );
	//blebding image
	blendImages ( A, B, R, dst, levels );
	

	cvSaveImage("result.png", dst);

	cvReleaseImage( &A );
	cvReleaseImage( &B);
	cvReleaseImage( &R );

  return 0;

}
int main()
{
	//test_fun_reConstruct();
	//ok--->fun_buildP
	//test_fun_buildLaplacianPry();
	test_fun_blendImage();
	return 0;

}
