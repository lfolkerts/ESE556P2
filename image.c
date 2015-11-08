#include <stdio.h>
#include  <fcntl.h>
#include <stdlib.h>
#include<stdint.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <wand/MagickWand.h>
#include"parameters.h"
#include "node.h"


void writeImage(char* image_filename)
{
	struct node* n;
	int i;
	MagickWand *m_wand = NULL;
	DrawingWand *d_wand = NULL;
	PixelWand *c_wand = NULL;


	m_wand = NewMagickWand();
	d_wand = NewDrawingWand();
	c_wand = NewPixelWand();

	//MagickSetImageFilename(m_wand,  image_filename)
	
	PixelSetColor(c_wand,"white");
	MagickNewImage(m_wand,RowWidth,GridHdr[NumRows]->coordinate,c_wand);

	DrawSetStrokeOpacity(d_wand,1);

	PushDrawingWand(d_wand);
	PixelSetColor(c_wand,"rgb(0,0,1)");

	DrawSetStrokeColor(d_wand,c_wand);
	DrawSetStrokeWidth(d_wand,0);
	DrawSetStrokeAntialias(d_wand,1);
	for(i=0; i<Modules; i++)
	{
		n = N_Arr[i];
		PixelSetColor(c_wand,"red");
		//DrawSetStrokeOpacity(d_wand,1);
		DrawSetFillColor(d_wand,c_wand);
		DrawRectangle(d_wand,n->x,n->y,n->x+n->width,n->y+n->height);
	}
	PopDrawingWand(d_wand);
	
	MagickWriteImage(m_wand, image_filename);

	c_wand = DestroyPixelWand(c_wand);
	m_wand = DestroyMagickWand(m_wand);
	d_wand = DestroyDrawingWand(d_wand);

	MagickWandTerminus();
}
