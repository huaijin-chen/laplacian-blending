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
 *   Organization:  Wu Han University (in China)
 *
 * ************************************************************************************
 */
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <string>
#define  LEVELS 4
using namespace std;

/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  blendImages
 *  Description:
 *        Input:  image A & B ; R is the overlap region
 *       Output:  blended image dst
 * *****************************************************************************
 */
void blendImages ( IplImage** A, IplImage** B, IplImage* R, IplImage* dst, int levels )
{
	//1. create LA & LB
	//2. create GB
	//3. costruct LS from LA, LB, with weigth GB
	//4. construct blended image from LS

	IplImage* GR[LEVELS] ={ R };
	IplImage* LA[LEVELS], LB[LEVELS], LS[LEVELS];
	IplImage* dst[LEVELS];

	CvScalar curr_ls, curr_la, curr_lb, curr_gr;

	int i, h, w;
	double scalar;
	// get GB  
	IplImage* GB[0] = R; 
	for ( i = 1; i < levels; i++ )
	{
		IplImage* GB[i] = cvCreateImage( cvSize(GB[i-1]->width/2,GB[i-1]->height/2), IPL_DEPTH_8U, 1 );
		cvPyrDown( GB[i-1], GR[i], CV_GAUSSIAN_5x5 );
	}

	for ( i = 0; i < A->nChannels; i++ )
	{
		buildLaplacianPry( A , LA, levels );
		buildLaplacianPry( B , LB, levels );
		
		// construct LS 
		for ( l = 0; l < nLevels; l++ )
		{
			scalar = 1.0 / power( 2, l-1 );
			for ( h = 0; h < height*scalar; h++ )
			{
				for ( w = 0; w < width*scalar; w++ )
				{
					// LS(i,j) = GB(i,j)*lA(i,j) + (1-GB(i,j)*LB(i,j)) 
					curr_gr = cvGet2D( GR, w, h );
					curr_la = cvGet2D( LA, w, h );
					curr_lb = cvGet2D( LB, w, h );

					curr_ls.val[0] = curr_gr.val[0] * curr_la.val[0] + (1-curr_gr[0])*curr_lb.val[0];

					cvSet2D( LS[l], w, h );
				}
			}

			reConstruct( LS, dsti[i], nLevels );


		}	
	}

	return 0;
}
/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  reConstruct
 *  Description:  construct  new image from laplacian pryimd(LS)
 * *****************************************************************************
 */
 
void reConstruct ( vector<IplImage*> LS, IplImage* dst, int nLevels )
{

	IplImage* curr;
	int i;
	for ( i = nLevels-1; i > 0; i-- )
	{
		cvPyrUp( LS[i], curr, CV_GAUSSIAN_5x5 );
		// have a problem  the size ???? 
		cvAdd( LS[i-1], curr, curr , NULL );
	}
	

	return 0;
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
}

int main()
{
	int levels = 4;
	int i;
	IplImage* LaplacianPry[4];
	IplImage* src = cvLoadImage( "hand.bmp" );
	buildLaplacianPry( src, LaplacianPry, levels );
	for( i = 0; i < levels; i++ )
	{
		cvSaveImage(i+"level.png", LaplacianPry[i] );
		//cvSaveImage("len1.png", LaplacianPry[1] );
		//cvSaveImage("len2.png", LaplacianPry[2] );
		//cvSaveImage("len3.png", LaplacianPry[3] );
	}
	return 0;
}
