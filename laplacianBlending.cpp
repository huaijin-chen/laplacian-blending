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
#include "huihgui.h"
#include <iostream>
#include <vector>
using namespace std;
/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  blendImages
 *  Description:
 *        Input:  image A & B ; R is the overlap region
 *       Output:  blended image dst
 * *****************************************************************************
 */
void blendImages ( IplImage* A, IplImage* B, IplImage* R, IplImage* dst, int nLevels )/*{{{*/
{
	/* 1. create LA & LB
	 * 2. create GB
	 * 3. costruct LS from LA, LB, with weigth GB
	 * 3. construct blended image from LS
	 * */

	vector<IplImage*> GR = R;
	vector<IplImage*> LA, LB, LS;
	vector<IplImage*> dst;

	CvScalar curr_ls, curr_la, curr_lb, curr_gr;

	int i, h, w;
	double scalar;
	/* get GB  */
	for ( i = 1; i < nLevels; i++ )
	{
		cvPyrDown( GB[i-1], GR[i], CV_GAUSSIAN_5x5 );
	}

	for ( i = 0; i < A->nChannels; i++ )
	{
		buildLaplacianPry( A , LA, nLevels );
		buildLaplacianPry( B , LB, nLevels );
		
		/* construct LS */
		for ( l = 0; l < nLevels; l++ )
		{
			scalar = 1.0 / power( 2, l-1 );
			for ( h = 0; h < height*scalar; h++ )
			{
				for ( w = 0; w < width*scalar; w++ )
				{
					/* LS(i,j) = GB(i,j)*lA(i,j) + (1-GB(i,j)*LB(i,j)) */
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
}/*-- end of function blendImages-- */
/*}}}*/


/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  reConstruct
 *  Description:  construct  new image from laplacian pryimd(LS)
 * *****************************************************************************
 */
void reConstruct ( vector<IplImage*> LS, IplImage* dst, int nLevels )/*{{{*/
{

	IplImage* curr;
	int i;
	for ( i = nLevels-1; i > 0; i-- )
	{
		cvPyrUp( LS[i], curr, CV_GAUSSIAN_5x5 );
		/* have a problem  the size ???? */
		cvAdd( LS[i-1], curr, curr , NULL );
	}
	

	return 0;
}/*-- end of function reConstruct-- */
/*}}}*/
/* 
 * ***  FUNCTION  **************************************************************
 *         Name:  buildLaplacianPry
 *  Description:  
 *  Imput :	
 * *****************************************************************************
 */
void buildLaplacianPry ( IplImage* src, vector<IplImage*> LSrc, int nLevels )/*{{{*/
{
	LSrc[0] = src;

	int i;
	/* get Gaussican pyr ---> Lsrc */
	for ( i = 0; i < nLevels-1; i++ )
	{
		cvPyrDown( LSrc[i], LSrc[i+1], CV_GAUSSIAN_5x5 );

	}
	/* LA(i) = GA(i) - expand(GA(i+1))  */
	for ( i = 1; i < nLevels-1; i++ )
	{
		IplImage* curr_expand;
		cvPyrUp( LSrc[i], curr_expand, CV_GAUSSIAN_5x5 );
		cvSub( LSrc[i-1], curr_expand, LSrc[i-1], NULL );
	}

	return 0;
}/*-- end of function buildLaplacianPry-- */
/*}}}*/


int main()
{
	IplImage *src1, *src2;
	IplImage *mask;
	IplImage *dst;
	int nLevel = 2;


	blendImags( src1, src2, mask, dst, nLevels );
	return 0;
}
