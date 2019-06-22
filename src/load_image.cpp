
#include    <gl\glu.h>                            // Header File For The GLu32 Library
#include    <gl\glaux.h>                          // Header File For The GLaux Library

bool        keys[256];                          // Array Used For The Keyboard Routine
bool        active=TRUE;                            // Window Active Flag
bool        fullscreen=TRUE;                        // Fullscreen Flag

GLfloat     xrot;                               // X Rotation ( NEW )
GLfloat     yrot;                               // Y Rotation ( NEW )
GLfloat     zrot;                               // Z Rotation ( NEW )








AUX_RGBImageRec *SDL_LoadBMP(char *Filename)                    // Loads A Bitmap Image
{
    FILE *File=NULL;                            // File Handle
    if (!Filename)                              // Make Sure A Filename Was Given
    {
        return NULL;                            // If Not Return NULL
    }
    File=fopen(Filename,"r");                       // Check To See If The File Exists
    if (File)                               // Does The File Exist?
    {
        fclose(File);                           // Close The Handle
        return auxDIBImageLoad(Filename);               // Load The Bitmap And Return A Pointer
    }
    return NULL;                                // If Load Failed Return NULL
}


int LoadGLTextures(char *Filename, Image image)                                // Load Bitmaps And Convert To Textures
{
    int Status=FALSE;                           // Status Indicator
    AUX_RGBImageRec *TextureImage[1];                   // Create Storage Space For The Texture
    memset(TextureImage,0,sizeof(void *)*1);                // Set The Pointer To NULL
    // Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
    if (TextureImage[0]=SDL_LoadBMP(Filename))
    {
        Status=TRUE;                            // Set The Status To TRUE
        glGenTextures(1, &image.texture);                  // Create The Texture

        // Typical Texture Generation Using Data From The Bitmap
        glBindTexture(GL_TEXTURE_2D, image.texture);
        // Generate The Texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, TextureImage[0]->image.w, TextureImage[0]->image.h, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->image.pixels);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
    }
    if (TextureImage[0])                            // If Texture Exists
    {
        if (TextureImage[0]->image.pixels)                   // If Texture Image Exists
        {
            free(TextureImage[0]->image.pixels);             // Free The Texture Image Memory
        }

        free(TextureImage[0]);                      // Free The Image Structure
    }
    return Status;                              // Return The Status
}


int InitGL(char *Filename, Image image)                              // All Setup For OpenGL Goes Here
{
    if (!LoadGLTextures(Filename, image))                          // Jump To Texture Loading Routine ( NEW )
    {
        return FALSE;                           // If Texture Didn't Load Return FALSE ( NEW )
    }

    glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping ( NEW )
    glShadeModel(GL_SMOOTH);                        // Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);                   // Black Background
    glClearDepth(1.0f);                         // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);                        // Enables Depth Testing
    glDepthFunc(GL_LEQUAL);                         // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);          // Really Nice Perspective Calculations
    return TRUE;                                // Initialization Went OK
}

int DrawGLScene(Image image)                             // Here's Where We Do All The Drawing
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear Screen And Depth Buffer
    glLoadIdentity();                           // Reset The Current Matrix
    glTranslatef(0.0f,0.0f,-5.0f);                      // Move Into The Screen 5 Units
    glRotatef(xrot,1.0f,0.0f,0.0f);                     // Rotate On The X Axis
    glRotatef(yrot,0.0f,1.0f,0.0f);                     // Rotate On The Y Axis
    glRotatef(zrot,0.0f,0.0f,1.0f);                     // Rotate On The Z Axis
    glBindTexture(GL_TEXTURE_2D, image.texture);               // Select Our Texture
    glBegin(GL_QUADS);
    // Front Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
    // Back Face
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
    // Top Face
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    // Bottom Face
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    // Right face
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
    // Left Face
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
    glEnd();
    xrot+=0.3f;                             // X Axis Rotation
    yrot+=0.2f;                             // Y Axis Rotation
    zrot+=0.4f;                             // Z Axis Rotation
    return true;                                // Keep Going
}
