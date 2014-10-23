#include "input.h"

#ifdef MARTII
char circle[] =
{
	0,2,2,2,2,2,2,2,2,2,2,0,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,1,1,1,1,1,1,1,1,2,
	0,2,2,2,2,2,2,2,2,2,2,0
};
#else
char circle[] =
{
	0,0,0,0,0,1,1,0,0,0,0,0,
	0,0,0,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,0,0,0,0,0
};
#endif

#ifdef MARTII
#if defined(HAVE_SPARK_HARDWARE) || defined(HAVE_DUCKBOX_HARDWARE)
void FillRect(int _sx, int _sy, int _dx, int _dy, uint32_t color)
{
	uint32_t *p1, *p2, *p3, *p4;
	sync_blitter = 1;

	STMFBIO_BLT_DATA bltData;
	memset(&bltData, 0, sizeof(STMFBIO_BLT_DATA));

	bltData.operation  = BLT_OP_FILL;
	bltData.dstOffset  = 1920 * 1080 * 4;
	bltData.dstPitch   = DEFAULT_XRES * 4;

	bltData.dst_left   = _sx;
	bltData.dst_top    = _sy;
	bltData.dst_right  = _sx + _dx - 1;
	bltData.dst_bottom = _sy + _dy - 1;

	bltData.dstFormat  = SURF_ARGB8888;
	bltData.srcFormat  = SURF_ARGB8888;
	bltData.dstMemBase = STMFBGP_FRAMEBUFFER;
	bltData.srcMemBase = STMFBGP_FRAMEBUFFER;
	bltData.colour     = color;

	if (ioctl(fb, STMFBIO_BLT, &bltData ) < 0)
		perror("RenderBox ioctl STMFBIO_BLT");
#if 0
	if(ioctl(fb, STMFBIO_SYNC_BLITTER) < 0)
		perror("blit ioctl STMFBIO_SYNC_BLITTER 2");
#else
	sync_blitter = 1;
#endif
}
#endif
#endif
void RenderBox(int sx, int sy, int ex, int ey, int rad, int col)
{
	int F,R=rad,ssx=startx+sx,ssy=starty+sy,dxx=ex-sx,dyy=ey-sy,rx,ry,wx,wy,count;

#ifdef MARTII
	uint32_t *pos = lbb + ssx + stride * ssy;
	uint32_t *pos0, *pos1, *pos2, *pos3, *i;
	uint32_t pix = bgra[col];
#else
	unsigned char *pos=(lbb+(ssx<<2)+fix_screeninfo.line_length*ssy);
	unsigned char *pos0, *pos1, *pos2, *pos3, *i;
	unsigned char pix[4]={bl[col],gn[col],rd[col],tr[col]};
#endif
		
	if (dxx<0) 
	{
#ifdef MARTII
		fprintf(stderr, "[input] RenderBox called with dx < 0 (%d)\n", dxx);
#else
		printf("[input] RenderBox called with dx < 0 (%d)\n", dxx);
#endif
		dxx=0;
	}

	if(R)
	{
#if defined(MARTII) && defined(HAVE_SPARK_HARDWARE) || defined(HAVE_DUCKBOX_HARDWARE)
		if(sync_blitter) {
			sync_blitter = 0;
			if (ioctl(fb, STMFBIO_SYNC_BLITTER) < 0)
				perror("RenderString ioctl STMFBIO_SYNC_BLITTER");
		}
#endif
		if(--dyy<=0)
		{
			dyy=1;
		}

		if(R==1 || R>(dxx/2) || R>(dyy/2))
		{
			R=dxx/10;
			F=dyy/10;	
			if(R>F)
			{
				if(R>(dyy/3))
				{
					R=dyy/3;
				}
			}
			else
			{
				R=F;
				if(R>(dxx/3))
				{
					R=dxx/3;
				}
			}
		}
		ssx=0;
		ssy=R;
		F=1-R;

		rx=R-ssx;
		ry=R-ssy;

#ifdef MARTII
		pos0=pos+(dyy-ry)*stride;
		pos1=pos+ry*stride;
		pos2=pos+rx*stride;
		pos3=pos+(dyy-rx)*stride;
#else
		pos0=pos+((dyy-ry)*fix_screeninfo.line_length);
		pos1=pos+(ry*fix_screeninfo.line_length);
		pos2=pos+(rx*fix_screeninfo.line_length);
		pos3=pos+((dyy-rx)*fix_screeninfo.line_length);
#endif
		while (ssx <= ssy)
		{
			rx=R-ssx;
			ry=R-ssy;
			wx=rx<<1;
			wy=ry<<1;

#ifdef MARTII
			for(i=pos0+rx; i<pos0+rx+dxx-wx;i++)
				*i = pix;
			for(i=pos1+rx; i<pos1+rx+dxx-wx;i++)
				*i = pix;
			for(i=pos2+ry; i<pos2+ry+dxx-wy;i++)
				*i = pix;
			for(i=pos3+ry; i<pos3+ry+dxx-wy;i++)
				*i = pix;
#else
			for(i=pos0+(rx<<2); i<pos0+((rx+dxx-wx)<<2);i+=4)
				memcpy(i, pix, 4);
			for(i=pos1+(rx<<2); i<pos1+((rx+dxx-wx)<<2);i+=4)
				memcpy(i, pix, 4);
			for(i=pos2+(ry<<2); i<pos2+((ry+dxx-wy)<<2);i+=4)
				memcpy(i, pix, 4);
			for(i=pos3+(ry<<2); i<pos3+((ry+dxx-wy)<<2);i+=4)
				memcpy(i, pix, 4);
#endif

			ssx++;
#ifdef MARTII
			pos2-=stride;
			pos3+=stride;
#else
			pos2-=fix_screeninfo.line_length;
			pos3+=fix_screeninfo.line_length;
#endif
			if (F<0)
			{
				F+=(ssx<<1)-1;
			}
			else   
			{ 
				F+=((ssx-ssy)<<1);
				ssy--;
#ifdef MARTII
				pos0-=stride;
				pos1+=stride;
#else
				pos0-=fix_screeninfo.line_length;
				pos1+=fix_screeninfo.line_length;
#endif
			}
		}
#ifdef MARTII
		pos+=R*stride;
#else
		pos+=R*fix_screeninfo.line_length;
#endif
	}

#if defined(MARTII) && defined(HAVE_SPARK_HARDWARE) || defined(HAVE_DUCKBOX_HARDWARE)
	FillRect(startx + sx, starty + sy + R, dxx + 1, dyy - 2 * R + 1, pix);
#else
	for (count=R; count<(dyy-R); count++)
	{
#ifdef MARTII
		for(i=pos; i<pos+dxx;i++)
			*i = pix;
		pos+=stride;
#else
		for(i=pos; i<pos+(dxx<<2);i+=4)
			memcpy(i, pix, 4);
		pos+=fix_screeninfo.line_length;
#endif
	}
#endif
}

/******************************************************************************
 * RenderCircle
 ******************************************************************************/

#ifdef MARTII
void RenderCircle(int sx, int sy, int col)
{
	int x, y;
	uint32_t pix = bgra[col];
	uint32_t *p = lbb + startx + sx;
	int s = stride * (starty + sy + y);

	for(y = 0; y < 12 * 12; y += 12, s += stride)
		for(x = 0; x < 12; x++)
			switch(circle[x + y]) {
				case 1: *(p + x + s) = pix; break;
				case 2: *(p + x + s) = 0xFFFFFFFF; break;
			}
}
#else
void RenderCircle(int sx, int sy, char col)
{
	int x, y;
#ifdef MARTII
	uint32_t pix = bgra[col];
#else
	unsigned char pix[4]={bl[col],gn[col],rd[col],tr[col]};
#endif
	//render

		for(y = 0; y < 12; y++)
		{
#ifdef MARTII
			for(x = 0; x < 12; x++) if(circle[x + y*12])
				*(lbb + startx + sx + x + stride*(starty + sy + y)) = pix;
#else
			for(x = 0; x < 12; x++) if(circle[x + y*12]) memcpy(lbb + (startx + sx + x)*4 + fix_screeninfo.line_length*(starty + sy + y), pix, 4);
#endif
		}
}
#endif

