//--------------------------------------------------------------------------------------------
// File:		MyCode.cpp
// Version:		V2.0
// Author:		Matthew Harris
// Description:	Your code goes here
// Notes:		For use with OpenGl 2.0 / DirectX9.0c or higher
//--------------------------------------------------------------------------------------------

// System header files
#include <math.h>

// Windows header files
// --

// Custom header files
#include "MyCode.h"
#include "resource.h"
extern HINSTANCE g_hInstance;
// For dynamic keyboard control
	int m_iMoveX;
	int m_iMoveY;
	int m_iMoveZ;	
	float m_dThetaX;
	float m_dThetaY;
	float m_dThetaZ;
	double m_dViewAngle;

	float m_slopeRight =0.1;
	float m_slopeLeft = 0.1;
	float m_depth = 1;
	//debugging control variables
	int iFaceSelection;			// FaceSelection > 0: enables one face (with index selecton - 1)
								// to be drawn on its own to aid debugging.
	bool bDebug;				// Switches on debug output to file when true.
	// number of polygons read in
	int m_iNumOfPolys;
	//Frame Count
	int m_iOurFrameCount;
	// Input filename
	char m_sFilename[30];
	// Viewpoint data, use for parts B/C
	VECTOR m_vDisp, m_vLight;	// Displacement of object coordinates relative
								// to viewing coordinates and light direction
	//Debug File Handle
	FILE *debugfile;



	// Drawing area dimensions: width and height
	int m_iWidth;
	int m_iHeight;

	//Drawing Surface Handle
	float *m_fDrawingSurface;
// Database data, for the polygons
POLYGON polylist[2000];  // Array to store a list of polygons.

COLOUR Default_Colour={0.5,0.5,0.5};//colours are floats these days
//-----------------------------------------------------------------------------
// Name: KeyboardControl
// Desc: Enable Keyboard Control
//-----------------------------------------------------------------------------
void KeyboardControl( WPARAM wParam )
{
	switch( wParam ) 
	{
	case VK_ESCAPE:	// Exit application
		PostQuitMessage( 0 );
		break;
	case VK_LEFT:
		m_iMoveX--;
		break;
	case VK_RIGHT:
		m_iMoveX++;
		break;
	case VK_UP:
		m_iMoveY--;
		break;
	case VK_DOWN:
		m_iMoveY++;
		break;
	case VK_ADD:
		m_iMoveZ++;
		break;
	case VK_SUBTRACT:
		m_iMoveZ--;
		break;
	case VK_D:
		bDebug = !bDebug; // Toggle Debug output
		break;
	case VK_Z:
		m_dThetaX += 0.01;
		break;
	case VK_X:
		m_dThetaY += 0.01;
		break;
	case VK_C:
		m_dThetaZ += 0.01;
		break;
	case VK_V:
		m_vDisp.x += 0.01;
		break;
	case VK_B:
		m_vDisp.y += 0.01;
		break;
	case VK_N:
		m_vDisp.z += 0.01;
		break;
	case VK_Q: 	DialogBoxParam( g_hInstance, MAKEINTRESOURCE(IDD_INIT),run->m_hWindow, run->DialogMessageHandlerStatic, (LPARAM)(run));
		break;

	}
}

//-----------------------------------------------------------------------------
// Name: KeyboardControl
// Desc: Enable Keyboard Control
//-----------------------------------------------------------------------------
void MouseControl( POINT currentMousePos, POINT oldMousePos )
{
	m_dThetaX -= ( currentMousePos.x - oldMousePos.x );
	m_dThetaY -= ( currentMousePos.y - oldMousePos.y );
}


//-----------------------------------------------------------------------------
// Name: displayFinal
// Desc: Routine to display useful info after program exit
//-----------------------------------------------------------------------------
void displayFinal( )
{
	char sDispString[50];
	sprintf( sDispString, "Total Framecount %d", m_iOurFrameCount );
	run->Alert( "Finished", sDispString );
}

//-----------------------------------------------------------------------------
// Name: displayReadInfo
// Desc: Routine to display useful info after file read, shows light vector
//		 as an example, modify and use for debugging as required
//-----------------------------------------------------------------------------
void displayReadInfo( )
{
	char sDispString[50];
	sprintf( sDispString, "%d polygons read", m_iNumOfPolys );
	run->Alert( m_sFilename, sDispString );
	sprintf( sDispString , "Light Vector %f  %f  %f", m_vLight.x, m_vLight.y, m_vLight.z );
	run->Alert( "Start Values:", sDispString );
}


//-----------------------------------------------------------------------------
// Name: LoadPolys
// Desc: Read polygon info from file
//-----------------------------------------------------------------------------
int LoadPolys( FILE *infile )
{
	char cInString[1000];
	int iNumPolys = 0;
	float fLength;
	float fR, fG, fB;		// red, green, blue values

	do
	{
		fgets( cInString, 1000, infile);						 // Read first/next line of file
		sscanf( cInString, "%d", &polylist[iNumPolys].nv);   // Get number of vertices
		fprintf( debugfile, "number of vertices: %d\n", polylist[iNumPolys].nv);   // print number of vertices to debug file
		if  (polylist[iNumPolys].nv == 0)
			break;	// Get out if terminating zero found

		// Only allocate the memory we need - ALWAYS remember to delete on shutdown

		for (int i = 0; i < polylist[iNumPolys].nv; i++)
		{
			// Read next line of file
			fgets(cInString, 1000, infile);
			//Get Coordinates
			sscanf(cInString, "%f%f%f", &( polylist[iNumPolys].vert[i].x ),
										&( polylist[iNumPolys].vert[i].y ),
										&( polylist[iNumPolys].vert[i].z ) );
		}
	
		polylist[iNumPolys].normal = Cross( VectorDiff( polylist[iNumPolys].vert[0], polylist[iNumPolys].vert[1] ),
									 VectorDiff( polylist[iNumPolys].vert[0],polylist[iNumPolys].vert[2] ) );
		fLength = (float)sqrt( Dot( polylist[iNumPolys].normal, polylist[iNumPolys].normal ) ); // Calculate length of vector

	    polylist[iNumPolys].normal.x /= fLength;	// Normalise
	    polylist[iNumPolys].normal.y /= fLength;	// each
	    polylist[iNumPolys].normal.z /= fLength;	// component
	    fgets(cInString, 1000, infile);		// Read  next line of file
	    sscanf( cInString, "%f%f%f", &fR, &fG, &fB );	// Get Colour, texture
		if (fR>1.0 ||fG>1.0||fB>1.0)//cope with either Open Gll 0-1.0 colours or old style 0-255 colours
		{
			polylist[iNumPolys].colour.r = fR/255.0;
			polylist[iNumPolys].colour.g = fG/255.0;
			polylist[iNumPolys].colour.b = fB/255.0;	
		}
		else
		{
			polylist[iNumPolys].colour.r = fR;
			polylist[iNumPolys].colour.g = fG;
			polylist[iNumPolys].colour.b = fB;	
		}
		iNumPolys++;
	} while( 1 );

	return iNumPolys;  //Return number of polygons read
}


//-----------------------------------------------------------------------------
// Name: ReadFile
// Desc: Read polygon info from file
//-----------------------------------------------------------------------------
void ReadFile()
{
	FILE *flInFile;
	flInFile = fopen( m_sFilename, "r" );
	m_iNumOfPolys = LoadPolys( flInFile );
	displayReadInfo();
	fclose(flInFile);	
}


//-----------------------------------------------------------------------------
// Name: Plotpix
// Desc: Draw a pixel - Calls nRGBAImage::SetColour(...),
//		 m_kImage MUST be initialised before use!
//		 Example usage: Plotpix( x, y, 255, 0, 0 );
//-----------------------------------------------------------------------------
inline void Plotpix( DWORD dwX, DWORD dwY, float fR, float fG, float fB )
{
	DWORD dwYtemp;

	// If using OpenGL we need to mirror the Y coordinates,
	// as OpenGL uses the opposite coordinate system to us and DirectX
#ifdef 	DIRECTX 
	dwYtemp = dwY;
#else  //OPENGL should be defined
	dwYtemp = ( m_iHeight - 1 ) - dwY;
#endif

	int index = 4 * ( dwYtemp * m_iWidth + dwX );
	m_fDrawingSurface[index]	 = fR;	// Red Channel
	m_fDrawingSurface[index + 1] = fG;	// Green Channel
	m_fDrawingSurface[index + 2] = fB;	// Blue Channel
	m_fDrawingSurface[index + 3] = 0.0; // Alpha Channel
}

//-----------------------------------------------------------------------------
// Name: Draw0Image
// Desc: Draws the image
//-----------------------------------------------------------------------------
void DrawImage( )
{
	VECTOR view_in_object_coordinates;
	TRANSFORM object_transformation;
	object_transformation = BuildTrans(m_dThetaX, m_dThetaY, m_dThetaZ, m_vDisp);
	POLYGON polyTempP, polyTempQ, polyTempQT;	// Temporary polygons for copying transformed, projected / clipped
												// versions of scene polys before drawing them. 
	int iCurrentPoly;							// Current polygon in process
	MATRIX inverse = InverseRotationOnly(object_transformation);
	view_in_object_coordinates = MOnV(inverse, m_vDisp);
	

	if (m_iNumOfPolys <= 0) {
		//DrawTrapezium(Default_Colour, 400, 400, -1.6, 1.5, 50, 200);		// This draws the square you see, replace with Trapezium, and later Polygon
		//DrawTrapezium(Default_Colour, 100, 200, 0.5, 3, 200, 220);
		//DrawTrapezium(Default_Colour, 100, 200, 2, -0.5, 300, 340);
		//DrawTrapezium(Default_Colour, 300, 300, -5, 0.1, 400, 430);
	}
	else {
		m_iOurFrameCount++;	// Increment frame counter if we have a polygon to draw
	}
	
	for (iCurrentPoly = 0; iCurrentPoly < m_iNumOfPolys; iCurrentPoly++)	// for each polygon
	{
		VECTOR tmpvec = VectorSum(polylist[iCurrentPoly].vert[0], view_in_object_coordinates);
		if (Dot(tmpvec, polylist[iCurrentPoly].normal) < 0) {
			continue;
		}

		if (iFaceSelection > m_iNumOfPolys)
			iFaceSelection = m_iNumOfPolys;    //Keep debug selector in range


		if (iFaceSelection && (iCurrentPoly + 1) != iFaceSelection)
			continue; // Reject unselected polygons if debug selection active.

		polyTempP = polylist[iCurrentPoly];             //copy static data into temp poly structure

		// Copy each vertex in polygon, add displacement to allow shift
		for (int i = 0; i < polylist[iCurrentPoly].nv; i++)
		{
			VECTOR tmp1, tmp2, tmp3;
			tmp1 = DoTransform(polylist[iCurrentPoly].vert[i], object_transformation);
			tmp2 = Project(tmp1, m_dViewAngle);
			tmp3.x = (tmp2.x + 1)*WINDOWWIDTH*0.5;
			tmp3.y = (tmp2.y + 1)*WINDOWHEIGHT*0.5;
			polyTempP.vert[i] = tmp3;

			polyTempP.vert[i].x += m_iMoveX;
			polyTempP.vert[i].y += m_iMoveY;
		}

		if (bDebug)
			fprintf(debugfile, " number of vertices: %d\n", polyTempP.nv);   // print number of vertices

		fflush(debugfile);

		if (bDebug)	// Print out current poly specs if debug active
		{
			for (int i = 0; i < polyTempP.nv; i++)
			{
				fprintf(debugfile, "before clipping Polygon %d, Vertex %d values: %7.2f, %7.2f, %11.6f\n",
					iCurrentPoly, i, polyTempP.vert[i].x, polyTempP.vert[i].y, polyTempP.vert[i].z);
			}
			fflush(debugfile);

		}
	
			// Call the drawing routine
		// The section below calls clipping and polygon draw routines, commented out to allow the 
		// program to work without them. You may re-instate once you have appropriate routines,
		// or replace with your own code.
	
		ClipPolyXHigh( &polyTempP, &polyTempQT, WINDOWWIDTH );	// Clip against upper x boundary
		ClipPolyYHigh( &polyTempQT, &polyTempQ, WINDOWHEIGHT );	// Clip against upper y boundary (bottom of screen)
		ClipPolyXLow( &polyTempQ, &polyTempQT, 0);				// Clip against lower x boundary
		ClipPolyYLow( &polyTempQT, &polyTempQ, 0);				// Clip against lower y boundary (bottom of screen)

		DrawPolygon(&polyTempQ);

		if ( bDebug )	// Print out current poly specs if debug active
		{
			for ( int i = 0; i < polyTempQ.nv; i++ )
				fprintf( debugfile, "after clipping Polygon %d Vertex %d values:y %7.2f  %7.2f %11.6f\n",
						 iCurrentPoly, i, polyTempQ.vert[i].x, polyTempQ.vert[i].y, polyTempQ.vert[i].z );
			fflush(debugfile);
		}
	}
	if ( m_iNumOfPolys > 0 )
		bDebug = false;	// Switch debug off after first run - switch on again via keyboard control if needed
}

//-----------------------------------------------------------------------------
// Name: DrawSquare
// Desc: Draw a sqaure
//-----------------------------------------------------------------------------
void DrawTrapezium(COLOUR c, float& x_start,float& x_end, float leftslope, float rightslope, int y_start, int y_end )
{
	/*float x_end = 50+m_iMoveX;
	float x_start = m_iMoveX;
	float y_end = 50 + m_iMoveY;
	float y_start = m_iMoveY;
	*/
	//Note no protection to keep in screen bounds...
	// y_end += m_depth;

	for (int j = y_start; j < y_end; j++)
	{
		x_end += rightslope;
		x_start += leftslope;
		for(int i = x_start; i < x_end; i++)
		{
			Plotpix( i, j, c.r, c.g, c.b );
		}
	}	
}


//-----------------------------------------------------------------------------
// Name: DrawPolygon
// Desc: Draw a polygon
//-----------------------------------------------------------------------------
void DrawPolygon(POLYGON *p)
{
	// Your code here
	int i = 0;
	int top_vertex = 0;
	int current_left;
	int current_right;
	int next_left;
	int next_right;
	float leftslope;
	float rightslope;
	int ytop;
	int ybottom;
	float x_start;
	float x_end;
	//finds the top vertex	
	for (i; i < p->nv; i++) {
		if (p->vert[i].y < p->vert[top_vertex].y) {
			top_vertex = i;
		}
	}
	current_left = top_vertex;
	current_right = top_vertex;
	next_left = ((current_left - 1) + p->nv) % p->nv;
	next_right = (current_right + 1) % p->nv;

	//calculates the left and right slopes
	if (p->vert[next_left].y == p->vert[current_left].y) {
		leftslope = 0;
	}
	else {
		leftslope = (p->vert[next_left].x - p->vert[current_left].x) / (p->vert[next_left].y - p->vert[current_left].y);
	}
	if (p->vert[next_right].x == p->vert[current_right].x) {
		rightslope = 0;
	}
	else {
		rightslope = (p->vert[next_right].x - p->vert[current_right].x) / (p->vert[next_right].y - p->vert[current_right].y);
	}
	//calculates the top of the current trapezium
	if (p->vert[current_left].y > p->vert[current_right].y) {
		ytop = p->vert[current_right].y;
	}
	else {
		ytop = p->vert[current_left].y;
	}
	//calculates the bottom of the current trapezium
	if (p->vert[next_left].y > p->vert[next_right].y) {
		ybottom = p->vert[next_right].y;
	}
	else {
		ybottom = p->vert[next_left].y;
	}
	x_start = p->vert[current_left].x;
	x_end = p->vert[current_right].x;

	DrawTrapezium(p->colour, x_start, x_end, leftslope, rightslope, ytop, ybottom);

	do {
		//identifies whether the vertex that terminates the previous trapezium is on the left or right
		ytop = ybottom;
		if (p->vert[next_left].y > p->vert[next_right].y) {
			//vertex that terminates the trapezium is on the right
			current_right = next_right;
			next_right = (current_right + 1) % p->nv;
			rightslope = (p->vert[next_right].x - p->vert[current_right].x) / (p->vert[next_right].y - p->vert[current_right].y);
			ybottom = p->vert[next_right].y;
			x_end = p->vert[current_right].x;
		}
		else {
			//vertex that terminates the trapezium is on the left
			current_left = next_left;
			next_left = ((current_left - 1) + p->nv) % p->nv;
			leftslope = (p->vert[next_left].x - p->vert[current_left].x) / (p->vert[next_left].y - p->vert[current_left].y);
			ybottom = p->vert[next_left].y;
			x_start = p->vert[current_left].x;
		}

		//find out whether the next trapezium finishes on the left or right
		if (p->vert[next_left].y > p->vert[next_right].y) {
			ybottom = p->vert[next_right].y;
		}
		else {
			ybottom = p->vert[next_left].y;
		}

		DrawTrapezium(p->colour, x_start, x_end, leftslope, rightslope, ytop, ybottom); \

	} while (next_left != next_right);

}

//-----------------------------------------------------------------------------
// Name: ClipPolyXLow
// Desc: Clipping Routine for lower x boundary
//-----------------------------------------------------------------------------
int ClipPolyXLow(POLYGON *pIinput, POLYGON *pOutput, int iXBound)
{
	int current_vertex =0;
	int previous_vertex;
	pOutput->colour = pIinput->colour;
	pOutput->nv = 0;

	//for each vertex
	for (current_vertex; current_vertex < pIinput->nv; current_vertex++) {

		previous_vertex = (((current_vertex + pIinput->nv)-1) % (pIinput->nv));

		//if previous vertex on screen
		if (pIinput->vert[previous_vertex].x >= iXBound) {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].x >= iXBound) {
				//copy vertex to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//else if current vertex off screen
			else {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].x = iXBound;
				pOutput->vert[pOutput->nv].y =
					(((iXBound - pIinput->vert[current_vertex].x)*(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) /
					(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) + pIinput->vert[current_vertex].y;
				pOutput->nv += 1;
			}
		}
		//if previous vertex off screen
		else {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].x > iXBound) {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].x = iXBound;
				pOutput->vert[pOutput->nv].y =
					(((iXBound - pIinput->vert[current_vertex].x)*(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) /
					(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) + pIinput->vert[current_vertex].y;
				pOutput->nv += 1;
				//...and copy the one directly after it to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//current vertex off screen
			else{
			//do nothing?
			}
		}
		
	}
	// Tell calling routine how many vertices in pOutput array
	return pOutput->nv;
}

//-----------------------------------------------------------------------------
// Name: ClipPolyYLow
// Desc: Clipping Routine for lower y boundary
//-----------------------------------------------------------------------------
int ClipPolyYLow(POLYGON *pIinput, POLYGON *pOutput, int iYBound)
{
	int current_vertex = 0;
	int previous_vertex;
	pOutput->colour = pIinput->colour;
	pOutput->nv = 0;

	//for each vertex
	for (current_vertex; current_vertex < pIinput->nv; current_vertex++) {

		previous_vertex = (((current_vertex + pIinput->nv) - 1) % (pIinput->nv));

		//if previous vertex on screen
		if (pIinput->vert[previous_vertex].y >= iYBound) {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].y >= iYBound) {
				//copy vertex to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//else if current vertex off screen
			else {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].y = iYBound;
				pOutput->vert[pOutput->nv].x =
					(((iYBound - pIinput->vert[current_vertex].y)*(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) /
					(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) + pIinput->vert[current_vertex].x;
				pOutput->nv += 1;
			}
		}
		//if previous vertex off screen
		else {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].y >= iYBound) {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].y = iYBound;
				pOutput->vert[pOutput->nv].x =
					(((iYBound - pIinput->vert[current_vertex].y)*(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) /
					(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) + pIinput->vert[current_vertex].x;
				pOutput->nv += 1;
				//...and copy the one directly after it to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//current vertex off screen
			else {
				//do nothing?
			}
		}

	}
	// Tell calling routine how many vertices in pOutput array
	return pOutput->nv;
}

//-----------------------------------------------------------------------------
// Name: ClipPolyXHi
// Desc: Clipping Routine for upper x boundary
//-----------------------------------------------------------------------------
int ClipPolyXHigh(POLYGON *pIinput, POLYGON *pOutput, int iXBound)
{
	int current_vertex = 0;
	int previous_vertex;
	pOutput->colour = pIinput->colour;
	pOutput->nv = 0;

	//for each vertex
	for (current_vertex; current_vertex < pIinput->nv; current_vertex++) {

		previous_vertex = (((current_vertex + pIinput->nv) - 1) % (pIinput->nv));

		//if previous vertex on screen
		if (pIinput->vert[previous_vertex].x <= iXBound) {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].x <= iXBound) {
				//copy vertex to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//else if current vertex off screen
			else {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].x = iXBound;
				pOutput->vert[pOutput->nv].y =
					(((iXBound - pIinput->vert[current_vertex].x)*(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) /
					(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) + pIinput->vert[current_vertex].y;
				pOutput->nv += 1;
			}
		}
		//if previous vertex off screen
		else {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].x < iXBound) {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].x = iXBound;
				pOutput->vert[pOutput->nv].y =
					(((iXBound - pIinput->vert[current_vertex].x)*(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) /
					(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) + pIinput->vert[current_vertex].y;
				pOutput->nv += 1;
				//...and copy the one directly after it to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//current vertex off screen
			else {
				//do nothing?
			}
		}

	}
	// Tell calling routine how many vertices in pOutput array
	return pOutput->nv;
}

//-----------------------------------------------------------------------------
// Name: ClipPolyYHi
// Desc: Clipping Routine for upper y boundary
//-----------------------------------------------------------------------------
int ClipPolyYHigh(POLYGON *pIinput, POLYGON *pOutput, int iYBound)
{
	int current_vertex = 0;
	int previous_vertex;
	pOutput->colour = pIinput->colour;
	pOutput->nv = 0;

	//for each vertex
	for (current_vertex; current_vertex < pIinput->nv; current_vertex++) {

		previous_vertex = (((current_vertex + pIinput->nv) - 1) % (pIinput->nv));

		//if previous vertex on screen
		if (pIinput->vert[previous_vertex].y <= iYBound) {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].y <= iYBound) {
				//copy vertex to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//else if current vertex off screen
			else {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].y = iYBound;
				pOutput->vert[pOutput->nv].x =
					(((iYBound - pIinput->vert[current_vertex].y)*(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) /
					(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) + pIinput->vert[current_vertex].x;
				pOutput->nv += 1;
			}
		}
		//if previous vertex off screen
		else {
			//if current vertex on screen
			if (pIinput->vert[current_vertex].y <= iYBound) {
				//generate a clipped vertex
				pOutput->vert[pOutput->nv].y = iYBound;
				pOutput->vert[pOutput->nv].x =
					(((iYBound - pIinput->vert[current_vertex].y)*(pIinput->vert[previous_vertex].x - pIinput->vert[current_vertex].x)) /
					(pIinput->vert[previous_vertex].y - pIinput->vert[current_vertex].y)) + pIinput->vert[current_vertex].x;
				pOutput->nv += 1;
				//...and copy the one directly after it to output
				pOutput->vert[pOutput->nv] = pIinput->vert[current_vertex];
				pOutput->nv += 1;
			}
			//current vertex off screen
			else {
				//do nothing?
			}
		}

	}
	// Tell calling routine how many vertices in pOutput array
	return pOutput->nv;

}

//-----------------------------------------------------------------------------
// Name: Init
// Desc: Initialises Direct3D etc.
//		 This is called before switch to graphics mode,
//		 example of z buffer initialisation shown in comments,
//		 ignore for parts a/b.
//-----------------------------------------------------------------------------
