#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <iostream>
#include <unistd.h>

typedef GLXContext (*GLXCREATECONTEXTATTRIBSARBPROC)(Display *, GLXFBConfig,
                                                     GLXContext, Bool,
                                                     const int *);

int main(int argc, char **argv) {
  Display *dpy = XOpenDisplay(0);

  int nelements;
  GLXFBConfig *fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), 0, &nelements);

  static int attributeList[] = {GLX_RGBA,
                                GLX_DOUBLEBUFFER,
                                GLX_RED_SIZE,
                                1,
                                GLX_GREEN_SIZE,
                                1,
                                GLX_BLUE_SIZE,
                                1,
                                None};
  XVisualInfo *vi = glXChooseVisual(dpy, DefaultScreen(dpy), attributeList);

  XSetWindowAttributes swa;
  swa.colormap =
      XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;
  Window win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, 400, 300,
                             0, vi->depth, InputOutput, vi->visual,
                             CWBorderPixel | CWColormap | CWEventMask, &swa);
  XMapWindow(dpy, win);

  Atom WM_DELETE_WINDOW = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, win, &WM_DELETE_WINDOW, 1);

  GLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB =
      (GLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress(
          (const GLubyte *)"glXCreateContextAttribsARB");
  std::cout << "glXCreateContextAttribsARB "
            << (void *)glXCreateContextAttribsARB << std::endl;

  // Core profile GL context
  int attribs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
                   GLX_CONTEXT_MINOR_VERSION_ARB, 2,
                   GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                   GLX_CONTEXT_FLAGS_ARB,         GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                   0};
  GLXContext ctx = glXCreateContextAttribsARB(dpy, *fbc, 0, true, attribs);
  glXMakeCurrent(dpy, win, ctx);

  float points[] = {
     0.0f,  0.5f,  0.0f,
     0.5f, -0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f
  };

  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), points, GL_STATIC_DRAW);

  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

  const char* vertex_shader =
    "#version 330\n"
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4(vp, 1.0);"
    "}";

  const char* fragment_shader =
    "#version 330\n"
    "out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = vec4(1.0, 1.0, 0.0, 1.0);"
    "}";

  char infoLog[10000];
  GLsizei length;

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  glGetShaderInfoLog(vs, sizeof(infoLog), &length, infoLog);

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  glGetShaderInfoLog(vs, sizeof(infoLog), &length, infoLog);

  GLuint shader_programme = glCreateProgram();
  glAttachShader(shader_programme, fs);
  glAttachShader(shader_programme, vs);
  glLinkProgram(shader_programme);
  glUseProgram(shader_programme);

  for(int i=0; ; i++) {
    glClearColor(i & 1 ? 0.5:0,
                 i & 2 ? 0.5:0,
                 i & 4 ? 0.5:0,
                 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glXSwapBuffers(dpy, win);

    if(XPending(dpy) > 0)
    {
      XEvent e;
      XNextEvent(dpy, &e);
      if(e.type == ClientMessage && e.xclient.data.l[0] == WM_DELETE_WINDOW)
        break;
    }

    usleep(100000);
  }

  ctx = glXGetCurrentContext();
  glXDestroyContext(dpy, ctx);
}
