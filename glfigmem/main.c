#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "png.h"

#define EVT_MASK (ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask | PointerMotionMask)
#define EVT_WAIT_EVENT(_ir) (_ir && !XNextEvent(g_dpy, &g_evt))
#define EVT_IS_EXPOSE() (g_evt.type == Expose && !g_evt.xexpose.count)
#define EVT_IS_BUTTONPRESS() (g_evt.type == ButtonPress)
#define EVT_IS_MOTION() (g_evt.type == MotionNotify)
#define EVT_IS_IN_SAMPLES() (xy[0] > (SAMPLE_X - 0.5) &&\
                             xy[0] < (SAMPLE_X + 0.5) &&\
                             xy[1] > (SAMPLE_Y - 0.5) &&\
                             xy[1] < (SAMPLE_Y + SAMPLE_COUNT - 0.5))
#define EVT_IS_IN_BOARD() (xy[0] > (BOARD_X - 0.5) &&\
                           xy[0] < (BOARD_X + BOARD_SIZE - 0.5) &&\
                           xy[1] > (BOARD_Y - 0.5) &&\
                           xy[1] < (BOARD_Y + BOARD_SIZE - 0.5))
#define EVT_IS_MOTION_TIME(_lt) (_lt + MOUSE_MOVE_PERIOD < g_evt.xmotion.time)
#define MOUSE_MOVE_PERIOD 30
#define SAMPLE_COUNT 4
#define SAMPLE_X 8
#define SAMPLE_Y 2
#define SAMPLE_Z (-24)
#define BOARD_SIZE  6
#define BOARD_X (-BOARD_SIZE / 2.0)
#define BOARD_Y (-BOARD_SIZE / 2.0)
#define BOARD_Z (-BOARD_SIZE * 3.0)
#define PUT_Z (BOARD_Z + 1)
#define OBJ_MAX_COUNT (BOARD_SIZE * BOARD_SIZE)
#define OBJ_MIN_COUNT 4
#define SCR_FAR 100
#define PROGRESS_SCALE(_oc) ((GLfloat)_oc * BOARD_SIZE / OBJ_MAX_COUNT)
#define PROGRESS_X (BOARD_X - 3)
#define PROGRESS_Y(_oc) (BOARD_Y + PROGRESS_SCALE(_oc) / 2 - 0.5)
#define PROGRESS_Z (BOARD_Z)

#define TRY_SCALE(_oc, _tr) ((GLfloat)_tr / _oc * 3)
#define TRY_X (PROGRESS_X + 1)
#define TRY_Y(_oc, _tr) (BOARD_Y + TRY_SCALE(_oc, _tr) / 2 - 0.5)
#define TRY_Z (BOARD_Z)


enum
{
  OBJ_RED_SPHERE = 1,
  OBJ_YELLOW_SPHERE,
  OBJ_GREEN_SPHERE,
  OBJ_BLUE_SPHERE,
  OBJ_GREY_SPHERE,
  OBJ_FOCUS_RECT,
  OBJ_CHECK_BUTTON,
  OBJ_FRAME1,
  OBJ_FRAME2,
  OBJ_PROGRESS,
  OBJ_TRY,
  OBJ_SAMPLE_BOARD,
  OBJ_ACTUAL_BOARD,
  OBJ_TEXTURE_TEST,
  OBJ_LAST
};

enum
{
  EVT_DRAW,
  EVT_SIZE,
  EVT_BUTTON,
  EVT_MOVE,
  EVT_LAST
};

static const GLint g_SphereIdArr[] =
{
  OBJ_RED_SPHERE,
  OBJ_YELLOW_SPHERE,
  OBJ_GREEN_SPHERE,
  OBJ_BLUE_SPHERE,
  OBJ_GREY_SPHERE
};

#ifdef WIN32
#else
static XEvent g_evt = {0};
static Atom g_wm_quit = 0;
static Display* g_dpy = 0;
static Window g_win = 0;
static GLXContext g_glc = 0;
#endif

static GLuint pngLoadTexture(const char *name, GLint* piWidth, GLint* piHeight)
{
   png_byte header[8] = {0};
   //open file as binary
   FILE *fp = fopen(name, "rb");
   if(!fp)
     return 0;

   //read the header
   fread(header, 1, 8, fp);

   //test if png
   if(png_sig_cmp(header, 0, 8))
   {
     fclose(fp);
     return 0;
   }

   //create png struct
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
   if(!png_ptr)
   {
     fclose(fp);
     return 0;
   }
   //create png info struct
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if(!info_ptr)
   {
     png_destroy_read_struct(&png_ptr, (png_infopp)0, (png_infopp)0);
     fclose(fp);
     return 0;
   }

   //create png info struct
   png_infop end_info = png_create_info_struct(png_ptr);
   if(!end_info)
   {
     png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
     fclose(fp);
     return 0;
   }

   //png error stuff, not sure libpng man suggests this.
   if(setjmp(png_jmpbuf(png_ptr)))
   {
     png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
     fclose(fp);
     return 0;
   }

   //init png reading
   png_init_io(png_ptr, fp);

   //let libpng know you already read the first 8 bytes
   png_set_sig_bytes(png_ptr, 8);

   // read all the info up to the image data
   png_read_info(png_ptr, info_ptr);

   //variables to pass to get info
   int bit_depth, color_type;
   png_uint_32 twidth, theight;

   // get info about png
   png_get_IHDR(png_ptr, info_ptr, &twidth, &theight, &bit_depth, &color_type, 0, 0, 0);

   //update width and height based on png info
   *piWidth = twidth;
   *piHeight = theight;

   // Update the png info struct.
   png_read_update_info(png_ptr, info_ptr);

   // Row size in bytes.
   int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

   // Allocate the image_data as a big block, to be given to opengl
   png_byte image_data[rowbytes * theight];

   //row_pointers is for pointing to image_data for reading the png with libpng
   png_bytep row_pointers[theight];

   // set the individual row_pointers to point at the correct offsets of image_data
   int i;
   for(i = 0; i < theight; ++i)
     row_pointers[theight - 1 - i] = image_data + i * rowbytes;

   //read the png into image_data through row_pointers
   png_read_image(png_ptr, row_pointers);

   //Now generate the OpenGL texture object
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D, texture);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, twidth, theight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)image_data);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 
   //clean up memory and close stuff
   png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   fclose(fp);
 
   return texture;
}


////////////////////////////////////////////////////////////////////////////////
static GLvoid objLoadTexture()
{

//  glBindTexture(GL_TEXTURE_2D, texture);
//  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  // when texture area is small, bilinear filter the closest mipmap
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  // when texture area is large, bilinear filter the original
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // the texture wraps over at the edges (repeat)
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  GLint iWidth, iHeight;
  GLuint texture = pngLoadTexture("test00.png", &iWidth, &iHeight);
  if(!texture)
    return;
//  glBindTexture(GL_TEXTURE_2D, texture);
//  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
//  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//  gluBuild2DMipmaps(GL_TEXTURE_2D, 4, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
//  glDeleteTextures(1, &texture);
//  GLuint theTexture;
//  glGenTextures(1, &theTexture);
//  glBindTexture(GL_TEXTURE_2D, theTexture);
//  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//  glTexImage2D(GL_TEXTURE_2D, 0, bHasAlpha ? 4 : 3, iWidth, iHeight, 0, bHasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);


  glNewList(OBJ_TEXTURE_TEST, GL_COMPILE);
    glEnable(GL_TEXTURE_2D);
//      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glPushMatrix();
        glTranslatef(4, 0, BOARD_Z + -5);
//        glRotatef(90, 0, 1, 0);
        glBegin(GL_QUADS);
//          glNormal3f(-1, 0, 0);
//          glVertex3f(0, 0, 1);
//          glVertex3f(0, 1, 1);
//          glVertex3f(0, 1, 0);
//          glVertex3f(0, 0, 0);
//          glTexCoord2d(0, 0); glVertex3f(0.0, 0.0, 0.0);
//          glTexCoord2d(0, 1); glVertex3f(0.0, 1.0, 0.0);
//          glTexCoord2d(1, 1); glVertex3f(0.0, 1.0, 1.0);
//          glTexCoord2d(1, 0); glVertex3f(0.0, 0.0, 1.0);
//          glNormal3f(0.0, 0.0, 1.0);
//          glTexCoord2d(1, 1); glVertex3f(0.0, 0.0, 0.0);
//          glTexCoord2d(1, 0); glVertex3f(0.0, 1.0, 0.0);
//          glTexCoord2d(0, 0); glVertex3f(1.0, 1.0, 0.0);
//          glTexCoord2d(0, 1); glVertex3f(1.0, 0.0, 0.0);
          glNormal3f(-1, 0.0, 0);
          glTexCoord2d(0, 0); glVertex3f(0.0, 0.0, 0.0);
          glTexCoord2d(0, 1); glVertex3f(0.0, 1.0, 0.0);
          glTexCoord2d(1, 1); glVertex3f(1.0, 1.0, 0.0);
          glTexCoord2d(1, 0); glVertex3f(1.0, 0.0, 0.0);
        glEnd();
      glPopMatrix();
    glDisable(GL_TEXTURE_2D);
  glEndList();
}

////////////////////////////////////////////////////////////////////////////////
static GLvoid objInitObjects(GLint iObjActualArr[OBJ_MAX_COUNT], GLint iObjCount, GLint iTries)
{
  static const GLfloat matSpecular[] = {0, 0, 0, 1};
  static const GLfloat matShiness[] = {100};
  static const GLfloat matArr[] =
  {
    0.2,  0,    0,    1,
    0.8,  0,    0,    1,
    0.2,  0.2,  0,    1,
    0.8,  0.8,  0,    1,
    0,    0.2,  0,    1,
    0,    0.8,  0,    1,
    0,    0,    0.2,  1,
    0,    0,    0.8,  1,
    0.2,  0.2,  0.2,  1,
    0.2,  0.2,  0.2,  1,
    0.4,  0.4,  0.4,  1,
    0.4,  0.4,  0.4,  1,
    1,  1,  1,  1,
    1,  1,  1,  1
  };
  static const GLfloat vpFrame[] =
  {
    -0.5, -0.5, 0,
    0.5,  -0.5, 0,
    0.5,  0.5,  0,
    -0.5, 0.5,  0,
    -0.5, -0.4, 0,
    -0.4, -0.5, 0,
    0.4,  -0.5, 0,
    0.5,  -0.4, 0,
    0.5,  0.4,  0,
    0.4,  0.5,  0,
    -0.4, 0.5,  0,
    -0.5, 0.4,  0
  };
  static const GLfloat vpCube[] =
  {
    0, 0, 1, -0.5, -0.5, 0.5,
    0, 0, 1, 0.5, -0.5, 0.5,
    0, 0, 1, 0.5, 0.5, 0.5,
    0, 0, 1, -0.5, 0.5, 0.5,
    1, 0, 0, 0.5, -0.5, 0.5,
    1, 0, 0, 0.5, 0.5, 0.5,
    1, 0, 0, 0.5, 0.5, -0.5,
    1, 0, 0, 0.5, -0.5, -0.5,
    0, 0.5, 0, -0.5, 0.5, 0.5,
    0, 0.5, 0, 0.5, 0.5, 0.5,
    0, 0.5, 0, 0.5, 0.5, -0.5,
    0, 0.5, 0, -0.5, 0.5, -0.5
  };
  const GLfloat* pf = matArr;
  const GLint* pi = g_SphereIdArr;
  GLint x, y, idx;
  objLoadTexture();
  GLUquadricObj* quad = gluNewQuadric();
    gluQuadricDrawStyle(quad, GLU_FILL);
    gluQuadricNormals(quad, GLU_SMOOTH);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShiness);
    for(idx = 0; idx < sizeof(g_SphereIdArr) / sizeof(g_SphereIdArr[0]); idx ++)
    {
      glNewList(*pi ++, GL_COMPILE);
      glMaterialfv(GL_FRONT, GL_AMBIENT, pf); pf += 4;
      glMaterialfv(GL_FRONT, GL_DIFFUSE, pf); pf += 4;
      gluSphere(quad, 0.4, 64, 64);
      glEndList();
    }
  gluDeleteQuadric(quad);
  glEnableClientState(GL_VERTEX_ARRAY);
  glNewList(OBJ_FOCUS_RECT, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT, pf);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, pf + 4);
    glVertexPointer(3, GL_FLOAT, 0, vpFrame);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
  glEndList();
  glNewList(OBJ_FRAME1, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT, pf); pf += 4;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, pf); pf += 4;
    glVertexPointer(3, GL_FLOAT, 0, vpFrame);
    glDrawArrays(GL_LINE_LOOP, 4, 8);
  glEndList();
  glNewList(OBJ_FRAME2, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_AMBIENT, pf);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, pf + 4);
    glVertexPointer(3, GL_FLOAT, 0, vpFrame);
    glDrawArrays(GL_LINE_LOOP, 4, 8);
  glEndList();
  glEnableClientState(GL_NORMAL_ARRAY);
  glNewList(OBJ_PROGRESS, GL_COMPILE);
    pf = matArr;
    glMaterialfv(GL_FRONT, GL_AMBIENT, pf); pf += 4;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, pf);
    glPushMatrix();
      glTranslatef(PROGRESS_X, PROGRESS_Y(iObjCount), PROGRESS_Z);
      glScalef(0.5, PROGRESS_SCALE(iObjCount), 0.5);
      glNormalPointer(GL_FLOAT, 6 * sizeof(GLfloat), vpCube);
      glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), vpCube + 3);
      glDrawArrays(GL_QUADS, 0, 12);
    glPopMatrix();
  glEndList();
  glNewList(OBJ_TRY, GL_COMPILE);
    pf = &matArr[16];
    glMaterialfv(GL_FRONT, GL_AMBIENT, pf); pf += 4;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, pf);
    glPushMatrix();
      glTranslatef(TRY_X, TRY_Y(iObjCount, iTries), TRY_Z);
      glScalef(0.5, TRY_SCALE(iObjCount, iTries), 0.5);
      glNormalPointer(GL_FLOAT, 6 * sizeof(GLfloat), vpCube);
      glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), vpCube + 3);
      glDrawArrays(GL_QUADS, 0, 12);
    glPopMatrix();
  glEndList();
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glNewList(OBJ_SAMPLE_BOARD, GL_COMPILE);
    pi = g_SphereIdArr;
    glPushMatrix();
      glTranslatef(SAMPLE_X, SAMPLE_Y, SAMPLE_Z);
      for(idx = 0; idx < SAMPLE_COUNT; idx ++)
      {
        glCallList(*pi ++);
        glCallList(OBJ_FRAME1);
        glTranslatef(0, 1, 0);
      }
    glPopMatrix();
  glEndList();
  glNewList(OBJ_ACTUAL_BOARD, GL_COMPILE);
    glPushMatrix();
      glTranslatef(BOARD_X, BOARD_Y, BOARD_Z);
      for(y = 0; y < BOARD_SIZE; y ++)
      {
        glPushMatrix();
        for(x = 0; x < BOARD_SIZE; x ++)
        {
          int idx = x + y * BOARD_SIZE;
          if(iObjActualArr[idx])
          {
            glCallList(iObjActualArr[idx]);
            glCallList(OBJ_FRAME1);
          }
          glTranslatef(1, 0, 0);
        }
        glPopMatrix();
        glTranslatef(0, 1, 0);
      }
    glPopMatrix();
    glCallList(OBJ_PROGRESS);
    glCallList(OBJ_TEXTURE_TEST);
  glEndList();
}

static GLvoid objDeleteObjects()
{
  glDeleteLists(OBJ_RED_SPHERE, OBJ_LAST - 1);
}

static GLvoid objInitRandom(GLint iObjRightArr[OBJ_MAX_COUNT], GLint iObjActualArr[OBJ_MAX_COUNT], GLint iObjCount)
{
  static const GLint iObjSampleArr[] =
  {
    OBJ_RED_SPHERE, OBJ_YELLOW_SPHERE, OBJ_GREEN_SPHERE, OBJ_BLUE_SPHERE
  };
  GLint i = 0;
  GLint idx = 0;
  for(i = 0; i < OBJ_MAX_COUNT; i ++)
    iObjActualArr[i] = iObjRightArr[i] = 0;
  i = 0;
  while(i < iObjCount)
  {
    do
    {
      idx = rand() % OBJ_MAX_COUNT;
    }while(iObjActualArr[idx]);
    iObjActualArr[idx] = iObjRightArr[idx] = iObjSampleArr[rand() % SAMPLE_COUNT];
    i ++;
  }
  objInitObjects(iObjActualArr, iObjCount, iObjCount * 2);
}

static GLvoid objInitCheckBalls(GLint iObjActualArr[OBJ_MAX_COUNT], GLint iObjCount)
{
  GLint idx;
  for(idx = 0;idx < OBJ_MAX_COUNT; idx ++)
    iObjActualArr[idx] = iObjActualArr[idx] ? OBJ_GREY_SPHERE : 0;
  objInitObjects(iObjActualArr, iObjCount, iObjCount * 2);
}


////////////////////////////////////////////////////////////////////////////////
/*
static void evtEventLoop(int (*procArr[EVT_LAST])())
{
  while(g_iRunning && cont)
  {
    XNextEvent(g_dpy, &g_evt);
    switch(g_evt.type)
    {
    case Expose:
      if(g_evt.xexpose.count)
        break;
      cont = procArr[EVT_DRAW]();
      break;
    case ConfigureNotify:
      cont = procArr[EVT_SIZE]();
      break;
    case KeyPress:
      cont = procArr[EVT_BUTTON]();
      break;
    case ButtonPress:
      cont = procArr[EVT_BUTTON]();
      break;
    case MotionNotify:
      cont = procArr[EVT_MOVE]();
      break;
    case ClientMessage:
      if(g_evt.xclient.data.l[0] != g_wm_quit)
        continue;
      g_iRunning = 0;
      return;
    }
  }
}


static void evtPassEvent(int (*procArr[EVT_LAST])())
{
  int cont = 1;
  while(cont && XCheckWindowEvent(g_dpy, g_win, EVT_MASK, &g_evt))
  {
    switch(g_evt.type)
    {
    case Expose:
      if(g_evt.xexpose.count)
        break;
      cont = procArr[EVT_DRAW]();
      break;
    case ConfigureNotify:
      cont = procArr[EVT_SIZE]();
      break;
    case KeyPress:
      cont = procArr[EVT_BUTTON]();
      break;
    case ButtonPress:
      cont = procArr[EVT_BUTTON]();
      break;
    case MotionNotify:
      cont = procArr[EVT_MOVE]();
      break;
    case ClientMessage:
      if(g_evt.xclient.data.l[0] != g_wm_quit)
        continue;
      g_iRunning = 0;
      return;
    }
  }
}*/

static GLvoid evtGetMouseWorldCoord(GLfloat xy[2], GLfloat z)
{
  GLdouble xyz[3];
  GLint viewport[4];
  GLdouble modelview[16], projection[16];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  gluUnProject(g_evt.xbutton.x, viewport[3] - g_evt.xbutton.y, 0, modelview, projection, viewport, &xyz[0], &xyz[1], &xyz[2]);
  xy[0] = xyz[0] * (-z);
  xy[1] = xyz[1] * (-z);
}

////////////////////////////////////////////////////////////////////////////////
static GLvoid evtDefaultHandler(GLint* piRunning)
{
  switch(g_evt.type)
  {
  case ConfigureNotify:
    glViewport(0, 0, g_evt.xconfigure.width, g_evt.xconfigure.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (GLfloat)g_evt.xconfigure.width / (GLfloat)g_evt.xconfigure.height, 1, SCR_FAR);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    break;
  case ClientMessage:
    *piRunning = g_evt.xclient.data.l[0] != g_wm_quit;
    break;
  }
}

static GLvoid scrPutScreen(GLint* piRunning, GLint id, GLint iObjCount, GLint iObjActualArr[OBJ_MAX_COUNT], GLint* piTries)
{
  if(!*piRunning)
    return;
  GLfloat xy[2];
  Time lastMoveTime = 0;
expose:
  evtGetMouseWorldCoord(xy, PUT_Z);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//  glCallList(OBJ_SAMPLE_BOARD);
	glCallList(OBJ_ACTUAL_BOARD);
	glCallList(OBJ_TRY);
  glPushMatrix();
    glTranslatef(xy[0], xy[1], PUT_Z);
    glCallList(id);
  glPopMatrix();
  glXSwapBuffers(g_dpy, g_win);
  while(EVT_WAIT_EVENT(*piRunning))
  {
    if(EVT_IS_EXPOSE()) goto expose;
    if(EVT_IS_MOTION() && EVT_IS_MOTION_TIME(lastMoveTime))
    {
      lastMoveTime = g_evt.xmotion.time;
      goto expose;
    }
    if(EVT_IS_BUTTONPRESS())
    {
      evtGetMouseWorldCoord(xy, BOARD_Z);
      if(EVT_IS_IN_BOARD())
      {
        xy[0] -= BOARD_X;
        xy[1] -= BOARD_Y;
        GLint idx = (GLint)(xy[0] + 0.5) + (GLint)(xy[1] + 0.5) * BOARD_SIZE;
        GLint idOld = iObjActualArr[idx];
        if(idOld)
        {
          (*piTries) --;
          iObjActualArr[idx] = id;
          objInitObjects(iObjActualArr, iObjCount, *piTries);
        }
      }
      break;
    }
    evtDefaultHandler(piRunning);
  }
}

static GLvoid scrCheckScreen(GLint* piRunning, GLint iObjRightArr[OBJ_MAX_COUNT], GLint iObjActualArr[OBJ_MAX_COUNT], GLint* piObjCount)
{
  if(!*piRunning)
    return;
  GLint iTries = *piObjCount * 2;
  GLfloat xy[2];
  Time lastMoveTime = 0;
  objInitCheckBalls(iObjActualArr, *piObjCount);
expose:
  evtGetMouseWorldCoord(xy, SAMPLE_Z);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glCallList(OBJ_SAMPLE_BOARD);
	glCallList(OBJ_ACTUAL_BOARD);
	glCallList(OBJ_TRY);
  glPushMatrix();
    glTranslatef(xy[0], xy[1], SAMPLE_Z);
    glCallList(OBJ_FRAME1);
  glPopMatrix();
  glXSwapBuffers(g_dpy, g_win);
  while(EVT_WAIT_EVENT(*piRunning))
  {
    if(EVT_IS_EXPOSE()) goto expose;
    if(EVT_IS_MOTION() && EVT_IS_MOTION_TIME(lastMoveTime))
    {
      lastMoveTime = g_evt.xmotion.time;
      goto expose;
    }
    if(EVT_IS_BUTTONPRESS())
    {
      evtGetMouseWorldCoord(xy, SAMPLE_Z);
      if(EVT_IS_IN_SAMPLES())
      {
        xy[0] -= SAMPLE_X;
        xy[1] -= SAMPLE_Y;
        scrPutScreen(piRunning, g_SphereIdArr[(GLint)(xy[1] + 0.5)], *piObjCount, iObjActualArr, &iTries);
        if(!memcmp(iObjRightArr, iObjActualArr, OBJ_MAX_COUNT * sizeof(int)))
        {
          (*piObjCount) ++;
          break;
        }
        if(!iTries)
        {
          (*piObjCount) --;
          break;
        }
        goto expose;
      }
    }
    evtDefaultHandler(piRunning);
  }
}

static GLvoid scrShowScreen(GLint* piRunning, GLint iObjRightArr[OBJ_MAX_COUNT], GLint iObjActualArr[OBJ_MAX_COUNT], GLint iObjCount)
{
  if(!*piRunning)
    return;
  objInitRandom(iObjRightArr, iObjActualArr, iObjCount);
  objInitObjects(iObjActualArr, iObjCount, iObjCount * 2);
expose:
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glCallList(OBJ_ACTUAL_BOARD);
  glXSwapBuffers(g_dpy, g_win);
  while(EVT_WAIT_EVENT(*piRunning))
  {
    if(EVT_IS_EXPOSE())
      goto expose;
    if(EVT_IS_BUTTONPRESS())
    {
      GLfloat xy[2];
      evtGetMouseWorldCoord(xy, -25);
      break;
    }
    evtDefaultHandler(piRunning);
  }
}

////////////////////////////////////////////////////////////////////////////////

static int wndInitWindow(const char* title)
{
  static GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 0, GLX_DOUBLEBUFFER, None };
  static GLint bits[] = {32, 24, 16, 8, 0};
  XSetWindowAttributes swa;
  XVisualInfo* vi = 0;
  g_dpy = XOpenDisplay(0);
  if(!g_dpy)
  {
    fprintf(stderr, "Couldn't connect to X server\n");
    return 1;
  }
//  XSynchronize(g_dpy, 1);//this slows down but makes synch calls. REMOVE in release
  int sn = DefaultScreen(g_dpy);
//  unsigned dw = DisplayWidth(g_dpy, sn);
//  unsigned dh = DisplayHeight(g_dpy, sn);
  Window root = RootWindow(g_dpy, sn);
  int* pb = bits;
  while(!vi && *pb)
  {
    att[2] = *pb ++;
    vi = glXChooseVisual(g_dpy, 0, att);
  }
  if(!vi)
  {
    fprintf(stderr, "Cannot choose visual\n");
    XCloseDisplay(g_dpy);
    return 2;
  }
  fprintf(stderr, ">>Visual: %d bits\n", att[2]);
  swa.colormap = XCreateColormap(g_dpy, root, vi->visual, AllocNone);
  swa.event_mask = EVT_MASK;
  g_wm_quit = XInternAtom(g_dpy, "WM_DELETE_WINDOW", False);
  g_win = XCreateWindow(g_dpy, root, 0, 0, 600, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
  XSetWMProtocols(g_dpy, g_win, &g_wm_quit, 1);
  XMapWindow(g_dpy, g_win);
  XStoreName(g_dpy, g_win, title);
  g_glc = glXCreateContext(g_dpy, vi, NULL, GL_TRUE);
  if(!g_glc)
  {
    XDestroyWindow(g_dpy, g_win);
    XCloseDisplay(g_dpy);
    return 3;
  }
  glXMakeCurrent(g_dpy, g_win, g_glc);
  return 0;
}

static void wndFreeWindow()
{
  XSelectInput(g_dpy, g_win, NoEventMask);
  glXMakeCurrent(g_dpy, None, NULL);
  glXDestroyContext(g_dpy, g_glc);
  XDestroyWindow(g_dpy, g_win);
  XCloseDisplay(g_dpy);
}

////////////////////////////////////////////////////////////////////////////////
static GLvoid gllInit()
{
  static const GLfloat lightAmbient[] = {1, 1, 1, 1};
  static const GLfloat lightDiffuse[] = {1, 1, 1, 1};
  static const GLfloat lightSpecular[] = {1, 1, 1, 1};
  static const GLfloat lightPosition[] = {-2, 2, 2, 0};
  glShadeModel(GL_SMOOTH);
  glClearColor(0, 0, 0, 1);
  glClearDepth(1.0f);
  glClearStencil(0);
  glPointSize(1.0f);
  glLineWidth(1.0f);
  glDepthFunc(GL_LEQUAL);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
//  glEnableClientState(GL_VERTEX_ARRAY);
//  glEnableClientState(GL_NORMAL_ARRAY);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);//!!! Try to remove this!
}

////////////////////////////////////////////////////////////////////////////////
//main
int main(int argc, char* argv[])
{
  int res = wndInitWindow("OpenGL Test");
  if(res)
    return res;
  srand((int)time(0));
  gllInit();
  GLint iObjCount = OBJ_MIN_COUNT;
  if(argc > 1)
    sscanf(argv[1], "%d", &iObjCount);
  GLint iRunning = 1;
  while(iRunning)
  {
    GLint iObjRightArr[OBJ_MAX_COUNT];
    GLint iObjActualArr[OBJ_MAX_COUNT];
    iObjCount = iObjCount > OBJ_MAX_COUNT ? OBJ_MAX_COUNT : iObjCount;
    iObjCount = iObjCount < OBJ_MIN_COUNT ? OBJ_MIN_COUNT : iObjCount;
    scrShowScreen(&iRunning, iObjRightArr, iObjActualArr, iObjCount);
    scrCheckScreen(&iRunning, iObjRightArr, iObjActualArr, &iObjCount);
  }
  objDeleteObjects();
  wndFreeWindow();
  return 0;
}

