#include <stdio.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#define PI 3.14159265

/*
struct DOT
{
  GLfloat x, y, z, distance;
};

struct ACTIVE_OBJ_ARRAY
{
  struct DOT dotArr[DOT_COUNT];
  GLushort dotIdxArr[DOT_COUNT];
};*/

#define BRICK_HCOUNT 10
#define BRICK_VCOUNT 10

struct SCENE
{
  GLint bricks[BRICK_HCOUNT][BRICK_VCOUNT];
  GLfloat ball_x, ball_y, ball_vx, ball_vy;
  GLfloat paddle_x;
};

////////////////////////////////////////////////////////////////////////////////
//Precompiled object ids
enum
{
  OBJ_FIRST = 1,
  OBJ_RECT = OBJ_FIRST,
  OBJ_BRICK,
  OBJ_BRICK_RED,
  OBJ_BRICK_YELLOW,
  OBJ_BRICK_GREEN,
  OBJ_BRICK_BLUE,
  OBJ_PADDLE,
  OBJ_FRAME,
  OBJ_LAST
};
/*
static GLvoid objUpdateDotList(struct ACTIVE_OBJ_ARRAY* objArr)
{
  glNewList(OBJ_DOTS, GL_COMPILE);
  glPushMatrix();
    glScalef(0.8, 0.8, 0.8);
    glVertexPointer(3, GL_FLOAT, sizeof(struct DOT), objArr->dotArr);
    glDrawElements(GL_POINTS, DOT_COUNT, GL_UNSIGNED_SHORT, objArr->dotIdxArr);
  glPopMatrix();
  glEndList();
}*/

static GLvoid objDoMove()
{
}

static GLvoid objCheckCollision()
{
}

static GLvoid scrDrawPaddle(GLfloat* paddle_x)
{
  int x, y;
  GLint viewport[4];
  SDL_GetMouseState(&x, &y);
  glGetIntegerv(GL_VIEWPORT, viewport);
//  printf("<=>%g\n", (2.0 * x / viewport[2] - 1) * 1.2 * viewport[2] / viewport[3]);
  *paddle_x = (2.0 * x / viewport[2] - 1) * 1.2 * viewport[2] / viewport[3];
  glPushMatrix();
    glTranslatef(*paddle_x, 0, 0);
    glCallList(OBJ_PADDLE);
  glPopMatrix();
}

static GLvoid scrDrawBall()
{
}

static GLvoid scrDrawBricks(GLint bricks[BRICK_HCOUNT][BRICK_VCOUNT])
{
  int i, j;
  glPushMatrix();
    glTranslatef(-1, 1, 0);
    glScalef(0.1, 0.05, 1);
    glTranslatef(1, -1, 0);
    for(j = 0; j < 10; j ++)
    {
      for(i = 0; i < 10; i++)
      {
        glCallList(bricks[i][j]);
        glTranslatef(2, 0, 0);
      }
      glTranslatef(-20, -2, 0);
    }
  glPopMatrix();
}

static GLvoid scrDrawAll(struct SCENE* scene)
{
  glPushMatrix();
  glTranslatef(0, 0, -20);
  glCallList(OBJ_FRAME);
  objCheckCollision();
  objDoMove();
  scrDrawBricks(scene->bricks);
  scrDrawPaddle(&scene->paddle_x);
  scrDrawBall();
/*    glTranslatef(0, 0, -20);
    glRotatef(rotX, 0, 1, 0);
    glRotatef(rotY, 1, 0, 0);
    glPushMatrix();
      glTranslatef(-0.8, -0.8, -0.8);
      glScalef(0.05, 0.05, 0.05);
      glCallList(OBJ_XYZARROWS);
    glPopMatrix();
    glColor4f(1, 1, 1, 1);
    glCallList(OBJ_DOTS);
    glColor4f(1, 1, 0.5, 0.7);
    glCallList(OBJ_AXES);*/
  glPopMatrix();
//  objUpdateDotList();
}
/*
static GLvoid objInitRandomDots(struct ACTIVE_OBJ_ARRAY* objArr)
{
  int i;
  srand48(SDL_GetTicks());
  for(i = 0; i < DOT_COUNT; i ++)
  {
    objArr->dotIdxArr[i] = (GLushort)i;
    objArr->dotArr[i].x = 2 * drand48() - 1;
    objArr->dotArr[i].y = 2 * drand48() - 1;
    objArr->dotArr[i].z = 2 * drand48() - 1;
  }
  objUpdateDotList(objArr);
}*/

static GLvoid objInitRandomBricks(struct SCENE* scene)
{
  srandom(SDL_GetTicks());
  GLint typeArr[] = {
    OBJ_BRICK_RED,
    OBJ_BRICK_YELLOW,
    OBJ_BRICK_GREEN,
    OBJ_BRICK_BLUE
  };
  int i, j;
  for(j = 0; j < BRICK_VCOUNT; j++)
  {
    for(i = 0; i < BRICK_HCOUNT; i ++)
      scene->bricks[i][j] = typeArr[random() % (sizeof(typeArr) / sizeof(typeArr[0]))];
  }
}

static GLvoid objInitObjects(struct SCENE* scene)
{
  objInitRandomBricks(scene);
  glNewList(OBJ_RECT, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
      glVertex2f(-1, -1);
      glVertex2f(-1, 1);
      glVertex2f(1, 1);
      glVertex2f(1, -1);
    glEnd();
  glEndList();
  glNewList(OBJ_BRICK, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
      glVertex2f(-1, -0.8);
      glVertex2f(-0.8, -1);
      glVertex2f(0.8,  -1);
      glVertex2f(1,  -0.8);
      glVertex2f(1,  0.8);
      glVertex2f(0.8,  1);
      glVertex2f(-0.8, 1);
      glVertex2f(-1, 0.8);
    glEnd();
  glEndList();
  glNewList(OBJ_BRICK_RED, GL_COMPILE);
    glColor4f(1, 0.25, 0.25, 1);
    glCallList(OBJ_BRICK);
  glEndList();
  glNewList(OBJ_BRICK_YELLOW, GL_COMPILE);
    glColor4f(1, 1, 0.25, 1);
    glCallList(OBJ_BRICK);
  glEndList();
  glNewList(OBJ_BRICK_GREEN, GL_COMPILE);
    glColor4f(0.25, 1, 0.25, 1);
    glCallList(OBJ_BRICK);
  glEndList();
  glNewList(OBJ_BRICK_BLUE, GL_COMPILE);
    glColor4f(0.25, 0.25, 1, 1);
    glCallList(OBJ_BRICK);
  glEndList();
  glNewList(OBJ_PADDLE, GL_COMPILE);
    glPushMatrix();
      glTranslatef(0, -1, 0);
      glScalef(0.2, 0.025, 1);
      glColor4f(1.0, 1.0, 1.0, 1);
      glCallList(OBJ_BRICK);
    glPopMatrix();
  glEndList();
  glNewList(OBJ_FRAME, GL_COMPILE);
    glColor4f(0.5, 0.5, 0.5, 1);
    glCallList(OBJ_RECT);
  glEndList();
/*  int i;
  glNewList(OBJ_ARROW, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
      glVertex2f(-1, 0.5);
      glVertex2f(0, 0.5);
      glVertex2f(0, 1);
      glVertex2f(1, 0);
      glVertex2f(0, -1);
      glVertex2f(0, -0.5);
      glVertex2f(-1, -0.5);
    glEnd();
  glEndList();
  glNewList(OBJ_AXES, GL_COMPILE);
    glPushMatrix();
      for(i = 0; i < 4; i ++)
      {
        glBegin(GL_LINE_LOOP);
          glVertex3f(-1, -1, 1);
          glVertex3f(1, -1, 1);
          glVertex3f(1, 1, 1);
          glVertex3f(-1, 1, 1);
        glEnd();
        glRotatef(90, 0, 1, 0);
        glRotatef(90, 1, 0, 0);
        glRotatef(90, 0, 0, 1);
      }
    glPopMatrix();
  glEndList();
  glNewList(OBJ_BRICK, GL_COMPILE);
    glBegin(GL_LINE_LOOP);
      glVertex2f(-1, -0.8);
      glVertex2f(-0.8, -1);
      glVertex2f(0.8,  -1);
      glVertex2f(1,  -0.8);
      glVertex2f(1,  0.8);
      glVertex2f(0.8,  1);
      glVertex2f(-0.8, 1);
      glVertex2f(-1, 0.8);
    glEnd();
  glEndList();
  glNewList(OBJ_XYZARROWS, GL_COMPILE);
    glColor4f(1, 0.25, 0.25, 1);
    glCallList(OBJ_ARROW);
    glRotatef(90.0, 0, 0, 1);
    glTranslatef(4, 0, 0);
    glColor4f(0.25, 1, 0.25, 1);
    glCallList(OBJ_ARROW);
    glRotatef(-90.0, 0, 1, 0);
    glTranslatef(4, 0, 2);
    glColor4f(0.25, 0.25, 1, 1);
    glCallList(OBJ_ARROW);
  glEndList();
  objInitRandomDots(objArr);*/
}

////////////////////////////////////////////////////////////////////////////////
//Precompiled object deletion
static GLvoid objDeleteObjects()
{
  glDeleteLists(OBJ_FIRST, OBJ_LAST - 1);
}

////////////////////////////////////////////////////////////////////////////////
//Window event handlers
static GLvoid fWndResize(GLint winWidth, GLint winHeight)
{
  glViewport(0, 0, winWidth, winHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(7, (GLdouble)winWidth / winHeight, 1, 100);
  glMatrixMode(GL_MODELVIEW);
}

static GLvoid fWndDraw(struct SCENE* scene)
{
  static GLint t0 = 0;
  static GLint frm = 0;
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  scrDrawAll(scene);
  SDL_GL_SwapBuffers();
  frm++;
  GLint t1 = SDL_GetTicks();
  if(t1 - t0 >= 5000)
  {
    GLfloat sec = (t1 - t0) / 1000.0;
    GLfloat fps = frm / sec;
    fprintf(stderr, "%d frames in %g seconds = %g FPS\n", frm, sec, fps);
    t0 = t1;
    frm = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
static GLvoid scrInit(struct SCENE* scene)
{
  fprintf(stderr, "GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
  fprintf(stderr, "GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
  fprintf(stderr, "GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
  fprintf(stderr, "GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));
  glClearColor(0, 0, 0, 1);
  glLineWidth(1);
  glPointSize(1);
  glEnable(GL_DEPTH_TEST);
//  glEnable(GL_BLEND);
//  glEnable(GL_LINE_SMOOTH);
//  glEnable(GL_POINT_SMOOTH);
//  glShadeModel(GL_SMOOTH);
//  glEnable (GL_BLEND);
//  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glEnable(GL_ALPHA_TEST);
  glEnableClientState(GL_VERTEX_ARRAY);
//  fntInitFonts();
  objInitObjects(scene);
  fprintf(stderr, "OpenGL: %s\n", gluErrorString(glGetError()));
}

int main()
{
  struct SCENE scene;
  unsigned char autoRotate = 0;
  int res = 0;
  SDL_Surface *screen;
  SDL_Init(SDL_INIT_VIDEO);
  screen = SDL_SetVideoMode(600, 600, 24, SDL_OPENGL|SDL_RESIZABLE);
  if(!screen)
  {
    fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  SDL_WM_SetCaption("Evol test", "Evol");
  scrInit(&scene);
//  SDL_ShowCursor(0);
  fWndResize(screen->w, screen->h);
  int done = 0;
  while(!done)
  {
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      switch(event.type) {
        case SDL_MOUSEBUTTONDOWN:
          printf("=>MOUSEBUTTONDOWN\n");
          break;
        case SDL_MOUSEMOTION:
//          printf("=>%d, %d\n", event.motion.xrel, event.motion.yrel);
//          printf("+>%d, %d\n", event.motion.x, event.motion.y);
          break;
        case SDL_KEYDOWN:
          if(event.key.keysym.sym == SDLK_p) autoRotate = !autoRotate;
          break;
        case SDL_VIDEORESIZE:
          screen = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_OPENGL|SDL_RESIZABLE);
          if(screen)
          {
            fWndResize(screen->w, screen->h);
          }
          else
          {
            fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
            res = 2;
            goto end;
          }
          break;
        case SDL_QUIT:
          goto end;
      }
    }
    Uint8* keys = SDL_GetKeyState(NULL);
//    rotY -= keys[SDLK_UP];
//    rotY += keys[SDLK_DOWN];
//    rotX -= keys[SDLK_LEFT];
//    rotX += keys[SDLK_RIGHT];

    done = keys[SDLK_ESCAPE];
    fWndDraw(&scene);
  }
end:
  objDeleteObjects();
  SDL_Quit();
  return res;
}

