#include "CNFG.h"

#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>
#define EGL_EGLEXT_PROTOTYPES
#include <EGL/eglext.h>
#include <GL/gl.h>



static int pbufferWidth = 0;
static int pbufferHeight = 0;
PFNEGLQUERYDEVICESEXTPROC _eglQueryDevicesEXT = NULL;
PFNEGLQUERYDEVICESTRINGEXTPROC _eglQueryDeviceStringEXT = NULL;
PFNEGLGETPLATFORMDISPLAYEXTPROC _eglGetPlatformDisplayEXT = NULL;
int num_devices = 0;
EGLDeviceEXT *devices = NULL;
EGLDisplay dpy = NULL;
EGLConfig *configs = NULL;
EGLSurface pb = 0;
EGLContext ctx = 0;


static EGLint pbattribs[] = {
	EGL_WIDTH, 0,
	EGL_HEIGHT, 0,
	EGL_NONE,
};

static const EGLint attribs[] = {
	EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_NONE
};

static const char *eglErrorString(EGLint error)
{
	switch (error)
	{
		case EGL_SUCCESS:
			return "Success";
		case EGL_NOT_INITIALIZED:
			return "EGL is not or could not be initialized";
		case EGL_BAD_ACCESS:
			return "EGL cannot access a requested resource";
		case EGL_BAD_ALLOC:
			return "EGL failed to allocate resources for the requested operation";
		case EGL_BAD_ATTRIBUTE:
			return "An unrecognized attribute or attribute value was passed in the attribute list";
		case EGL_BAD_CONTEXT:
			return "An EGLContext argument does not name a valid EGL rendering context";
		case EGL_BAD_CONFIG:
			return "An EGLConfig argument does not name a valid EGL frame buffer configuration";
		case EGL_BAD_CURRENT_SURFACE:
			return "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid";
		case EGL_BAD_DISPLAY:
			return "An EGLDisplay argument does not name a valid EGL display connection";
		case EGL_BAD_SURFACE:
			return "An EGLSurface argument does not name a valid surface configured for GL rendering";
		case EGL_BAD_MATCH:
			return "Arguments are inconsistent";
		case EGL_BAD_PARAMETER:
			return "One or more argument values are invalid";
		case EGL_BAD_NATIVE_PIXMAP:
			return "A NativePixmapType argument does not refer to a valid native pixmap";
		case EGL_BAD_NATIVE_WINDOW:
			return "A NativeWindowType argument does not refer to a valid native window";
		case EGL_CONTEXT_LOST:
			return "The application must destroy all contexts and reinitialise";
	}
	return "UNKNOWN EGL ERROR";
}

#define THROW(m) {  \
	fprintf(stderr, "ERROR in line %d: %s\n", __LINE__, m);  \
}

#define THROWEGL() THROW(eglErrorString(eglGetError()))

void CNFGGetDimensions( short * x, short * y )
{
	*x = pbufferWidth;
	*y = pbufferHeight;
}

void CNFGChangeWindowTitle( const char * WindowName )
{
}

void CNFGSetupFullscreen( const char * WindowName, int screen_no )
{
	//Fullscreen is meaningless for this driver, since it doesn't really open a window.
	CNFGSetup( WindowName, 1024, 1024 );
}

void CNFGTearDown()
{
	if(ctx && dpy) eglDestroyContext(dpy, ctx);
	if(pb && dpy) eglDestroySurface(dpy, pb);
	if(dpy) eglTerminate(dpy);
	if(configs) free(configs);
	if(devices) free(devices);
}

int CNFGSetup( const char * WindowName, int w, int h )
{
	atexit( CNFGTearDown );

    int major, minor, ret = 0, nc = 0;
	pbattribs[1] = pbufferWidth = w;
	pbattribs[3] = pbufferHeight = h;

    if((_eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT")) == NULL)
		THROW("eglQueryDevicesEXT() could not be loaded");
	if((_eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)eglGetProcAddress("eglQueryDeviceStringEXT")) == NULL)
		THROW("eglQueryDeviceStringEXT() could not be loaded");
	if((_eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT")) == NULL)
		THROW("eglGetPlatformDisplayEXT() could not be loaded");

	if(!_eglQueryDevicesEXT(0, NULL, &num_devices) || num_devices<1)
		THROWEGL();
	if((devices = (EGLDeviceEXT *)malloc(sizeof(EGLDeviceEXT) * num_devices)) == NULL)
		THROW("Memory allocation failure");
	if(!_eglQueryDevicesEXT(num_devices, devices, &num_devices) || num_devices<1)
		THROWEGL();

	for(int i = 0; i < num_devices; i++)
	{
		const char *devstr = _eglQueryDeviceStringEXT(devices[i], EGL_DRM_DEVICE_FILE_EXT);
		printf("Device 0x%.8lx: %s\n", (unsigned long)devices[i], devstr ? devstr : "NULL");
	}

    if((dpy = _eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[0], NULL)) == NULL)
		THROWEGL();
	if(!eglInitialize(dpy, &major, &minor))
		THROWEGL();
	printf("EGL version %d.%d\n", major, minor);
	if(!eglChooseConfig(dpy, attribs, NULL, 0, &nc) || nc<1)
		THROWEGL();
	if((configs = (EGLConfig *)malloc(sizeof(EGLConfig) * nc)) == NULL)
		THROW("Memory allocation failure");
	if(!eglChooseConfig(dpy, attribs, configs, nc, &nc) || nc<1)
		THROWEGL();

	if((pb = eglCreatePbufferSurface(dpy, configs[0], pbattribs)) == NULL)
		THROWEGL();
	if (!eglBindAPI(EGL_OPENGL_API))
		THROWEGL();
	if((ctx = eglCreateContext(dpy, configs[0], NULL, NULL)) == NULL)
		THROWEGL();
	if(!eglMakeCurrent(dpy, pb, pb, ctx))
		THROWEGL();

	return 0;
}

int CNFGHandleInput()
{
	//Stubbed (No input)
	return 1;
}

void CNFGSetVSync( int vson )
{
	//No-op
}

void CNFGSwapBuffers()
{
	
}

