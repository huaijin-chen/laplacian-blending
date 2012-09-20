/*
 * ************************************************************************************
 *
 *       Filename:  laplacianBlending.c
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
 *        Input:  image A & B ; R is the overlap region
 *       Output:  blended image dst
 * *****************************************************************************
 */
int blendImages ( IplImage* A, IplImage* B, IplImage* R, IplImage** dst, int levels )
{
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
	cout << "entery blending" <<endl;
	//1. create LA & LB
	//2. create GB
	//3. costruct LS from LA, LB, with weigth GB
	//4. construct blended image from LS
	
	
	IplImage* GR[LEVELS];
	IplImage* LA[LEVELS];
	IplImage* LB[LEVELS];
	IplImage* LS[LEVELS];

	CvScalar curr_ls, curr_la, curr_lb, curr_gr;

	int i, h, w, l;
	double scalar;
	// get GB  
	cout<< "befor copy"<<endl;
	IplImage* mask = cvCreateImage(cvGetSize(R), IPL_DEPTH_8U, 1);

	if( R->nChannels > 1){
		cvCvtColor( R, mask, CV_BGR2GRAY);  
		GR[0] = cvCloneImage( mask );
	}
	else{
		GR[0] = cvCloneImage( R );
	}

	//cvSaveImage("GRRRR.png",GR[0]);
	for ( i = 1; i < levels; i++ )
	{
		GR[i] = cvCreateImage( cvSize(GR[i-1]->width/2,GR[i-1]->height/2), IPL_DEPTH_8U, 1 );
		cout<<"huaijin"<<endl;
		cvPyrDown( GR[i-1], GR[i], CV_GAUSSIAN_5x5 );
		cout<<"nimei"<<endl;
		//GR[i] = cvCloneImage( down );
	}

	for ( i = 0; i < A->nChannels; i++ )
	{
		cout<< "build laplacian pyr in " << i << endl;
		//get LA
		buildLaplacianPry( A , LA, levels );

		//get LB
		buildLaplacianPry( B , LB, levels );
		
		// construct LS 
		for ( l = 0; l < levels; l++ )
		{
			int width  = LA[l]->width;
			int height = LA[l]->height; 
			//init LS[i]
			LS[l] = cvCreateImage( cvSize(width,height), IPL_DEPTH_8U, 1 );

			for ( h = 0; h < height; h++ )
			{
				for ( w = 0; w < width; w++ )
				{
					// LS(i,j) = GB(i,j)*lA(i,j) + (1-GB(i,j)*LB(i,j)) 
					curr_gr = cvGet2D( GR[l], w, h );
					curr_la = cvGet2D( LA[l], w, h );
					curr_lb = cvGet2D( LB[l], w, h );

					curr_ls.val[0] = curr_gr.val[0] * curr_la.val[0] + (1-curr_gr.val[0])*curr_lb.val[0];

					cvSet2D( LS[l], w, h, curr_ls );
				}/* endfor */
			}/* endfor */

			reConstruct( LS, dst[l], levels );

		}/* endfor */	
	}/* endfor */ 

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
	CvSize curr_size = cvSize( LS[levels-2]->width, LS[levels-1]->height );
	IplImage* curr = cvCreateImage( curr_size, IPL_DEPTH_8U, 1 );
	int i;
	for ( i = levels-1; i > 0; i-- )
	{
		cout << "LS[" << i << "]'s channel is "<< LS[i]->nChannels << endl;  
		cvPyrUp( LS[i], curr, CV_GAUSSIAN_5x5 );
		// 只有上帝才能看得懂的code
		cvAdd( LS[i-1], curr, curr , NULL );
	}
	dst = curr;

	cvReleaseImage( &curr );
	cout << "exit reconstruct()" <<endl;

}

/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  buildLaplacianPry
 *  Description:  
 *  Imput :	
 * *****************************************************************************
 */
void buildLaplacianPry( IplImage* src, IplImage** LPyr, int levels )
{
	cout << "enter build Laplacian Pry" << endl;
	IplImage* currentImage = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U,1); 
    cvCvtColor(src,currentImage,CV_BGR2GRAY);  
	int i;

	//LA(i) = GA(i) - expand(GA(i+1))
	for ( i = 0; i < levels-1; i++ )
	{
		//the size of next level image
		CvSize down_size = cvSize(currentImage->width/2, currentImage->height/2);
		IplImage* down = cvCreateImage( down_size, IPL_DEPTH_8U, 1 );

		cvPyrDown( currentImage, down, CV_GAUSSIAN_5x5 );

		IplImage* up = cvCreateImage( cvGetSize(currentImage), IPL_DEPTH_8U, 1 );
		cvPyrUp( down, up, CV_GAUSSIAN_5x5 );

		LPyr[i] = cvCreateImage( cvGetSize(currentImage), IPL_DEPTH_8U, 1 );

		cvSub( currentImage, up, LPyr[i], NULL );

		currentImage = down;
		cvReleaseImage( &up );
		cvReleaseImage( &down );
	}
	LPyr[levels-1] = cvCreateImage( cvGetSize(currentImage), IPL_DEPTH_8U, 1 );
	//get the lastest level --->Lpyr[last_index]
	cvCopyImage( currentImage, LPyr[levels-1] );

	cvReleaseImage( &currentImage );
	cout << "exit build Laplacian Pyr" << endl;
}

int test_fun_buildLaplacianPry() /*{{{*/
{
	int levels = 4;
	int i;
	IplImage* LaplacianPry[4];
	IplImage* src = cvLoadImage( "hand.bmp" );
	buildLaplacianPry( src, LaplacianPry, levels );
	//for( i = 0; i < levels; i++ )
	//{
		cvSaveImage("level0.png", LaplacianPry[0] );
		cvSaveImage("level1.png", LaplacianPry[1] );
		cvSaveImage("level2.png", LaplacianPry[2] );
		cvSaveImage("level3.png", LaplacianPry[3] );
	//}
	return 0;
}/*}}}*/

int test_fun_reConstruct()
{
	IplImage* LS[4];
	IplImage* temp;

	LS[0] = cvLoadImage( "level0.png" );
    cvCvtColor(,currentImage,CV_BGR2GRAY);  
	LS[1] = cvLoadImage( "level1.png" );
	LS[2] = cvLoadImage( "level2.png" );
	LS[3] = cvLoadImage( "level3.png" );

	IplImage* dst = cvCreateImage( cvGetSize(LS[0]), IPL_DEPTH_8U, 1 );

	reConstruct( LS, dst, 4 );

	cvSaveImage( "testConstruct_fun.png", dst );

	return 0;	
}

int huaijin()
{
	int levels = 4;

	IplImage* A = cvLoadImage( "eye.bmp" );
	IplImage* B = cvLoadImage( "hand.bmp" );
	IplImage* R = cvLoadImage( "mask.bmp" );

	int channels = A->nChannels;
	IplImage* dst[channels];
	//blebding image
	blendImages ( A, B, R, dst, levels );
	
	cvSaveImage("result.png", dst[0]);

	cvReleaseImage( &A );
	cvReleaseImage( &B);
	cvReleaseImage( &R );

  return 0;

}
int main()
{
	//test_fun_reConstruct();
	test_fun_buildLaplacianPry();
	return 0;
}
