#include "window.h"
#include "../de_struct.h"
#include "../android/glext.h"

#define DE_WINDOW_DEFAULT_FLAGS SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE
// SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN

int dwindow_init(int x, int y, int flags){
	dwindow_preconfig();

	SDL_Init(SDL_INIT_EVERYTHING);

	if(!flags)
		flags = DE_WINDOW_DEFAULT_FLAGS;

	flags |= SDL_WINDOW_OPENGL;

	#ifdef ANDROID
		// only possible window size is screen size
		flags |= SDL_WINDOW_FULLSCREEN;
		x=0; y=0;
	#endif

	SDL_DisplayMode mode;
	SDL_GetDisplayMode(0, 0, &mode);

	if(!x || !y){
		x = mode.w;
		y = mode.h;
	}

	De.screen_size[0] = mode.w;
	De.screen_size[1] = mode.h;
	De.size[0] = x;
	De.size[1] = y;
	De.res = (float)y / (float)x;

	De.win = SDL_CreateWindow("no title",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		x, y, flags);

	if (De.win == NULL) {
		DE_LOGE("error: could not create window; %s", SDL_GetError());
		return 1;
	}

	De.ctx = SDL_GL_CreateContext(De.win);

	SDL_GL_MakeCurrent(De.win, De.ctx);



	dwindow_get_gl_info();

/* opengl extensions */
	#if defined (ANDROID)
		dandroid_glext_init();
	#else
		GLenum glewError = glewInit();
		if (glewError != GLEW_OK) {
			DE_LOGE("GLEW: ERROR: %s\n", glewGetErrorString(glewError));
			return 2;
		}
		De.glext_has_vao = 1;
	#endif

/* SDL_Image_Init */
	// img_flags |= IMG_INIT_TIF; // windows and android error: not suported
	int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
	int initted = IMG_Init(img_flags);
	if((initted&img_flags) != img_flags) {
		DE_LOG("IMG_Init: %s\n", IMG_GetError());
		return 3;
	}

	#ifdef ANDROID
		dwindow_fullscreen(true);
	#endif

	dwindow_resized(x, y);

	return 0;
}



void dwindow_quit(void){
	SDL_DestroyWindow(De.win);
	SDL_GL_DeleteContext(De.ctx);
	dandroid_glext_quit();
	DE_LOG("window destroyed");
}





















#include "../gl/gl.h" // gl_viewport

void dwindow_resized(int x, int y){
	assert(x>0);
	assert(y>=0);
	dviewport(x, y);
	De.size = (v2i){x, y};
	De.ctx_size = (v2i){x, y};
	De.res = ((float)y) / ((float)x);
	De.ctx_res = De.res;
}



bool dwindow_focus(void){
	return De.window_focus;
}



void dwindow_fullscreen(bool b){
	SDL_SetWindowFullscreen(De.win, b);
}



void dwindow_preconfig(void){
	/* gl version */
	// #ifdef ANDROID // gles 2.0
	// 	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	// 		SDL_GL_CONTEXT_PROFILE_ES);
	// 	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	// #else // opengl 3.0, compatibility profile
	// 	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	// 		SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	// 	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	// #endif
	// SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	//gles2 for all
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);



	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	/* buffer sizes */
	// SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	// SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	// SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	// SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	/* antialias */
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
}



void dwindow_title(char* title){
	SDL_SetWindowTitle(De.win, title);
}



void dwindow_get_gl_info(void){
	De.gl_driver = (char*)SDL_GetCurrentVideoDriver();
	De.gl_version = (char*)glGetString(GL_VERSION);
	De.gl_renderer = (char*)glGetString(GL_RENDERER);
	De.gles_v2 = strstr(De.gl_version, "OpenGL ES 2.") ? true : false;
}



