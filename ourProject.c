/*
 *
 *  Key bindings:

 *  arrows Change view angle
 *  wasd to move
 *  0      Reset view angle
 *  ESC    Exit
 */
#include "CSCIx229.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <math.h>
#else
#include <GL/glut.h>
#endif

//  Globals
float grid[128][128] = {0};
float gridCoeff = 0.8;
float sini = 8;
float cosi = 6;


//  Coordinates
double x = 1;
double y = 1;
double z = 1;
//  Time step
double dt = 0.001;

double t = 0;

//gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, upX, upY, upZ);
double eyeX = 3;
double eyeY = 1;
double eyeZ = 4;
double lookX = 0;
double lookY = 1;
double lookZ = 0;
//up
double spin = 0; //how many degrees we've rotated

int th=0;       // Azimuth of view angle
int ph=20;       // Elevation of view angle
int fov=55;       //  Field of view (for perspective)
double asp=1;     //  Aspect ratio
int mode=1;     // 0 for ortho, 1 for perspective, 2 for first person
int m = 0;

double w=1;     // W variable
double dim=4;   // Dimension of orthogonal box
char* text[] = {"","2D","3D constant Z","3D","4D"};  // Dimension display text

//  Macro for sin & cos in degrees
#define Cos(th) cos(3.1415926/180*(th))
#define Sin(th) sin(3.1415926/180*(th))


// Light values
int light = 1;
int one       =   1;  // Unit value
int distance  =   1;  // Light distance

int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  20;  // Ambient intensity (%) (30)
int diffuse   =  50;  // Diffuse intensity (%) (100)
int specular  =  10;  // Specular intensity (%)
int shininess =  10;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth (90)
float ylight  =   0;  // Elevation of light
unsigned int texture[24];
float xoffset = 0;
float yoffset = 0;

int alpha = 100; // Transparency value



/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  // Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

void Project(double fov,double asp,double dim)
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Perspective transformation
   if (fov)
      gluPerspective(fov,asp,dim/16,16*dim);
   //  Orthogonal transformation
   else
      glOrtho(-asp*dim,asp*dim,-dim,+dim,-dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

static void water()
{

  glPushMatrix(); // WATER
  glBegin(GL_QUADS);
  glColor4f(0.4, 0.45, 0.7, 0.005*alpha);
  //glColor3f(0.4, 0.45, 0.7);
  //enable blending
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE);
  glNormal3f(0, 1, 0);
  glVertex3d(-6.4, -0.2, -6.4);
  glVertex3d(-6.4, -0.2,  6.4);
  glVertex3d(6.4,  -0.2,  6.4);
  glVertex3d(6.4,  -0.2, -6.4);
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  glEnd();
  glPopMatrix();
}
void heights(){

  for(int i = 0; i < 128; i ++){//Initiate grid values (y values/elevations)
    for(int j = 0; j < 128; j ++){
      // Just sine and cosine:
      // grid[i][j] = Sin((float)(i)*sini) * Cos((float)(j)*cosi) * gridCoeff;

      grid[i][j] = ((float)(rand() % 100 + -1) / 20 - 2.85); // creating noise

      if (grid[i][j] < 0){
        grid[i][j] = grid[i][j] / 2; // making terrain flatter below the water
      }
    }
  }

  for(int i = 2; i < 126; i ++){ // AVERAGING OUT THE NOISE to make things smoother
    for(int j = 2; j < 126; j ++){
      for(int k = -2; k < 3; k ++){
        for(int l = -2; l <3; l++){
          grid[i][j] = grid[i][j] + grid[i + k][j + l];
        }
      }
      grid[i][j] = grid[i][j] / 25;
    }
  }

  for(int i = 1; i < 127; i ++){ // AVERAGING OUT THE NOISE to make things smoother
    for(int j = 1; j < 127; j ++){
      grid[i][j] = grid[i-1][j-1] + grid[i-1][j] + grid[i-1][j+1]
                 + grid[i][j-1] + grid[i][j] + grid[i][j+1]
                 + grid[i+1][j-1] + grid[i+1][j] + grid[i+1][j+1];
      grid[i][j] = grid[i][j] / 9;
    }
  }

  // edges don't get averaged, so they get pushed down manually here
  for(int i = 0; i < 128; i++){
    grid[i][0] = grid[i][1];
  }
  for(int j = 0; j < 128; j++){
    grid[0][j] = grid[1][j];
  }
  for(int i = 0; i < 128; i++){
    grid[i][127] = grid[i][126];
  }
  for(int j = 0; j < 128; j++){
    grid[127][j] = grid[126][j];
  }
}

void display()
{
    //  Clear the image
    //  Reset previous transforms
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glLoadIdentity();

    //enable blending
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA,GL_ONE);

   if (mode == 1)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //  Orthogonal - set world orientation
   else if (mode == 0)
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }
   else if (mode == 2)
   {
     lookX = eyeX + Sin(spin);
     lookZ = eyeZ - Cos(spin);
     gluLookAt(eyeX, eyeY, eyeZ, lookX, lookY, lookZ, 0, 1, 0);
   }

   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.0095*ambient ,0.009*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light position
      //float Position[]  = {4*Cos(zh), 3.5*Sin(zh), 0, 1.0};
      float Position[]  = {2, 5, 0, 1.0};



      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);


    glPushMatrix();


    for(int i = 0; i < 127; i ++){
      glBegin(GL_QUAD_STRIP);
      for(int j = 0; j < 128; j ++){

        //glColor3f(((float)(i)/64), 0.5, ((float)(j)/64)); // rainbow
        if (grid[i][j] > 0.6){
          glColor3f(0.8, 0.8, 0.8);
        }
        else if (grid[i][j] >= 0){
          glColor3f((grid[i][j] + 0.2), 0.8, (grid[i][j] + 0.2));
        }
        else{
          glColor3f(0.2, 0.8, 0.2);
        }
        glNormal3f(0, 1, 0);
        glVertex3d((((float)(j)/10) - 6.4), grid[i][j],   (((float)(i) / 10) - 6.4));
        glVertex3d((((float)(j)/10) - 6.4), grid[i+1][j], ((((float)(i) + 1) / 10) - 6.4));

      }
      glEnd();
    }
    glPopMatrix();
/*
    for(float z = 0; z <= 3.2; z = z + 0.1){
      glPushMatrix();
      glBegin(GL_QUAD_STRIP);
      // change elevation HERE with relation to z
      elevation = (z*z) / 3.2;
      for(float x = 0; x <= 3.2; x = x + 0.1){
        glColor3f((x/3.2), 0.5, (z/3.2));
        // change elevation HERE with relation to x
        elevation = (elevation + x*x )/ 7;

        glVertex3d(x, elevation, (z+0));
        glVertex3d(x, elevation, (z+0.1));
      }
      glEnd();
      glPopMatrix();
    }
*/

   //  Draw axes in white
   glColor3f(1,1,1);
   glBegin(GL_LINES);
   glVertex3d(0,0,0);
   glVertex3d(1,0,0);
   glVertex3d(0,0,0);
   glVertex3d(0,1,0);
   glVertex3d(0,0,0);
   glVertex3d(0,0,1);
   glEnd();
   //  Label axes
   glRasterPos3d(1,0,0);
   Print("X");
   glRasterPos3d(0,1,0);
   Print("Y");
   glRasterPos3d(0,0,1);
   Print("Z");

  //draw water
  water();

   //  Flush and swap
   glFlush();
   glutSwapBuffers();
}
/*
void idle(){
  t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
  if (m == 0){
    zh = fmod(90*t,360.0);
  }
  //  Tell GLUT it is necessary to redisplay the scene
  glutPostRedisplay();
}*/


void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);

   //  Reset view angle
   else if (ch == 'r'){
      th = 0;
      ph = 30;
      mode = 1;
      eyeX = -3;
      eyeY = 1;
      eyeZ = 4;
      lookX = 0;
      lookY = 1;
      lookZ = 0;
      m = 0;
      //up
      spin = 0; //how many degrees we've rotated
    }

    else if (ch == '0')
      mode = 0;

    else if (ch == '1')
      mode = 1;

    else if (ch == '2')
      mode = 2;

    else if (ch == 'm'){
      m = !m;
    }
    else if (ch == ','){
      zh = zh - 2;
    }
    else if (ch == '.'){
      zh = zh + 2;
    }
    else if (ch == 'y'){
      heights();
    }


    if (mode == 2){

      if (ch == 'w'){
        lookZ = lookZ - 0.1*Cos(spin);
        eyeZ = eyeZ - 0.1*Cos(spin);
        lookX = lookX + 0.1*Sin(spin);
        eyeX = eyeX + 0.1*Sin(spin);
      }
      if (ch == 's'){
        lookZ = lookZ + 0.1*Cos(spin);
        eyeZ = eyeZ + 0.1*Cos(spin);
        lookX = lookX - 0.1*Sin(spin);
        eyeX = eyeX - 0.1*Sin(spin);
      }
      if (ch == 'd'){
        lookZ = lookZ + 0.1*Sin(spin);
        eyeZ = eyeZ + 0.1*Sin(spin);
        lookX = lookX + 0.1*Cos(spin);
        eyeX = eyeX + 0.1*Cos(spin);
      }
      if (ch == 'a'){
        lookZ = lookZ - 0.1*Sin(spin);
        eyeZ = eyeZ - 0.1*Sin(spin);
        lookX = lookX - 0.1*Cos(spin);
        eyeX = eyeX - 0.1*Cos(spin);
      }
    }



    Project(45,asp,dim);


   glutPostRedisplay();
}


void special(int key,int x,int y)
{
   if (mode == 2){
     if (key == GLUT_KEY_RIGHT){
       spin = spin + 2; // rotate this many degrees right
     }
     if (key == GLUT_KEY_LEFT){
       spin = spin - 2;
     }
   }
   else{
       if (key == GLUT_KEY_RIGHT)
          th += 5;
       else if (key == GLUT_KEY_LEFT)
          th -= 5;
       else if (key == GLUT_KEY_UP)
          ph += 5;
       else if (key == GLUT_KEY_DOWN)
          ph -= 5;
       th %= 360;
       ph %= 360;
    }

  Project(45,asp,dim);

   //  Tell GLUT it is necessary to redisplay the scene

   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(45,asp,dim);
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{

  heights();
  //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
   //  Request 500 x 500 pixel window
   glutInitWindowSize(700,700);
   glutCreateWindow("Coordinates");

   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   //glutIdleFunc(idle);

   //textures here
   ErrCheck("init");
   glutMainLoop();
   return 0;
}
