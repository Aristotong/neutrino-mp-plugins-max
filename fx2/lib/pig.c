/*
** initial coding by fx2
*/

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <config.h>

#if defined(HAVE_SPARK_HARDWARE) || defined(HAVE_DUCKBOX_HARDWARE)
#include "draw.h"

int fx2_use_pig = 1;

void Fx2SetPig(int x __attribute__((unused)), int y __attribute__((unused)), int w __attribute__((unused)), int h __attribute__((unused)))
{
	return;
}

void Fx2ShowPig(int x, int y, int width, int height)
{
	int pig = open("/proc/stb/vmpeg/0/dst_all", O_RDWR);
	if (pig > -1) {
		char s[80];
		if (x == -1 && y == -1 && width == -1 && height == -1)
			x = 0, y = 0, width = 720, height = 576;
		write(pig, s, snprintf(s, sizeof(s), "%x %x %x %x", x, y, width, height));
		close(pig);
		FBSetPig(x, y, width, height);
	}
}

void Fx2StopPig(void)
{
	Fx2ShowPig(-1, -1, -1, -1);
}

void Fx2PigPause(void)
{
	return;
}

void Fx2PigResume(void)
{
	return;
}
#else
#ifdef HAVE_TRIPLEDRAGON
// ugly, but works
int fx2_use_pig = 1;
extern FBFillRect(int x, int y, int dx, int dy, unsigned char col);

void Fx2SetPig(int x, int y, int w, int h)
{
	return;
}

void Fx2ShowPig(int x, int y, int width, int height)
{
	char command[64];
	FBFillRect(x, y, width, height, 0); // Fill transp.
	sprintf(command, "pzapit --pig %d %d %d %d 0", x, y, width, height);
	system(command);
}

void Fx2StopPig(void)
{
	system("pzapit --pig 0 0 0 0 0");
}

void Fx2PigPause(void)
{
	return;
}

void Fx2PigResume(void)
{
	return;
}
#else // !TRIPLEDRAGON

static 	int		fd = -1;
		int		fx2_use_pig = 1;
static	int		l_x = 0;
static	int		l_y = 0;
static	int		l_width = 0;
static	int		l_height = 0;
#ifndef i386

#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
	#include <dbox/avia_gt_pig.h>
	#define PIGDEV "/dev/dbox/pig0"
	extern FBFillRect( int x, int y, int dx, int dy, unsigned char col );
#else
	//Narf ... sucks
	#define _LINUX_TIME_H
	#define PIGDEV "/dev/v4l/video0"
	#include <linux/videodev.h>
	static	struct v4l2_format format;
#endif

void	Fx2SetPig( int x, int y, int width, int height )
{
#ifdef HAVE_DBOX_HARDWARE
	int overlay;
#endif

	if ( fd==-1 )
		return;

#ifdef HAVE_DBOX_HARDWARE
	if (( x == format.fmt.win.w.left ) && ( y == format.fmt.win.w.top ) &&
		( width == format.fmt.win.w.width ) && ( height == format.fmt.win.w.height ))
			return;
	
	overlay = 0;
	
	ioctl(fd, VIDIOC_OVERLAY, &overlay);
	
	format.fmt.win.w.left=x;
	format.fmt.win.w.top=y;
	format.fmt.win.w.width=width;
	format.fmt.win.w.height=height;
	
	ioctl(fd, VIDIOC_S_FMT, &format);
	
	overlay = 1;
	
	ioctl(fd, VIDIOC_OVERLAY, &overlay);
#else
	avia_pig_hide(fd);
	avia_pig_set_pos(fd,x,y);
	avia_pig_set_size(fd,width,height);
	FBFillRect( x, y, width, height, 0 ); // Fill transp.. for dreambox
	avia_pig_show(fd);
	l_x=x;
	l_y=y;
	l_width=width;
	l_height=height;
#endif
}

void	Fx2ShowPig( int x, int y, int width, int height )
{
#ifdef HAVE_DBOX_HARDWARE
	int overlay;
#endif
	if ( fd != -1 )
	{
		Fx2SetPig(x,y,width,height);
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
		l_x=x;
		l_y=y;
		l_width=width;
		l_height=height;
#endif
		return;
	}
	
	if (( fd == -1 ) && fx2_use_pig )
		fd = open( PIGDEV, O_RDONLY );
	
	if ( fd == -1 )
		return;

#ifdef HAVE_DBOX_HARDWARE
	ioctl(fd, VIDIOC_G_FMT, &format);
	format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	format.fmt.win.w.left=x;
	format.fmt.win.w.top=y;
	format.fmt.win.w.width=width;
	format.fmt.win.w.height=height;
	
	ioctl(fd, VIDIOC_S_FMT, &format);
	
//FIXME	avia_pig_set_stack(fd,2);
	
	overlay = 1;
	
	ioctl(fd, VIDIOC_OVERLAY, &overlay);
#else
	FBFillRect( x, y, width, height, 0 ); // fill transp for dreambox
	l_x=x;
	l_y=y;
	l_width=width;
	l_height=height;
	avia_pig_set_pos(fd,x,y);
	avia_pig_set_size(fd,width,height);
	avia_pig_set_stack(fd,2);
	avia_pig_show(fd);
#endif
}

void	Fx2StopPig( void )
{
#ifdef HAVE_DBOX_HARDWARE
	int overlay;
#endif
	if ( fd == -1 )
		return;

#ifdef HAVE_DBOX_HARDWARE
	overlay = 0;
	ioctl(fd, VIDIOC_OVERLAY, &overlay);
#else
	avia_pig_hide(fd);
#endif

	close(fd);
	fd=-1;
}

void	Fx2PigPause( void )
{
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
	if ( fd != -1 )
		avia_pig_hide(fd);
#else
	int overlay;

	if ( fd != -1 ) {
		overlay = 0;
		ioctl(fd, VIDIOC_OVERLAY, &overlay);
	}
#endif
}

void	Fx2PigResume( void )
{
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
	if ( fd != -1 )
		avia_pig_show(fd);
#else
	int overlay;

	if ( fd != -1 ) {
		overlay = 1;
		ioctl(fd, VIDIOC_OVERLAY, &overlay);
	}
#endif
}

#else

#include "draw.h"

void	Fx2SetPig( int x, int y, int width, int height )
{
	return;
}

void	Fx2ShowPig( int x, int y, int width, int height )
{
	FBFillRect( x, y, width, height, 4 );
	l_x=x;
	l_y=y;
	l_width=width;
	l_height=height;
	return;
}
void	Fx2StopPig( void )
{
	FBFillRect( l_x, l_y, l_width, l_height, 1 );
	return;
}
void	Fx2PigPause( void )
{
	return;
}
void	Fx2PigResume( void )
{
	return;
}
#endif
#endif
#endif
