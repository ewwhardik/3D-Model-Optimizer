#ifndef __glad_h_
#define __glad_h_

#ifdef __gl_h_
#error OpenGL header already included
#endif
#define __gl_h_

#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#define APIENTRY __stdcall
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPIENTRY
#define GLAPIENTRY APIENTRY
#endif

#include "KHR/khrplatform.h"

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef khronos_int8_t GLbyte;
typedef khronos_uint8_t GLubyte;
typedef khronos_int16_t GLshort;
typedef khronos_uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef khronos_int32_t GLclampx;
typedef int GLsizei;
typedef khronos_float_t GLfloat;
typedef khronos_float_t GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void* GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
#ifdef __APPLE__
typedef void *GLhandleARB;
#else
typedef unsigned int GLhandleARB;
#endif
typedef unsigned short GLhalfARB;
typedef unsigned short GLhalf;
typedef GLint GLfixed;
typedef khronos_intptr_t GLintptr;
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_int64_t GLint64;
typedef khronos_uint64_t GLuint64;
typedef khronos_intptr_t GLintptrARB;
typedef khronos_ssize_t GLsizeiptrARB;
typedef khronos_int64_t GLint64EXT;
typedef khronos_uint64_t GLuint64EXT;
typedef struct __GLsync *GLsync;
struct _cl_context;
struct _cl_event;
typedef void (APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCAMD)(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, void *userParam);
typedef unsigned short GLhalfNV;
typedef GLintptr GLvdpauSurfaceNV;
typedef void (APIENTRY *GLVULKANPROCNV)(void);

/* ─── GL Constants ─────────────────────────────────────────────────────────── */
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_NO_ERROR                       0
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_NONE                           0

#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006

#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308

#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408

#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506

#define GL_CW                             0x0900
#define GL_CCW                            0x0901

#define GL_POINT_SIZE                     0x0B11
#define GL_LINE_WIDTH                     0x0B21
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_TEST                   0x0B90
#define GL_BLEND                          0x0BE2
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND_DST                      0x0BE0
#define GL_DITHER                         0x0BD0

#define GL_TEXTURE_2D                     0x0DE1
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_DOUBLE                         0x140A

#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_REPEAT                         0x2901

#define GL_TEXTURE0                       0x84C0

/* Framebuffer */
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_UNSIGNED_INT_24_8              0x84FA

/* Shaders */
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84

/* Buffers */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

/* Texture formats */
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_RGBA8                          0x8058
#define GL_RGB8                           0x8051

/* Polygon mode */
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02

/* Multisample */
#define GL_MULTISAMPLE                    0x809D

/* VAO */
/* (OpenGL 3.3 core) */

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Function pointer types ────────────────────────────────────────────────── */
typedef void   (APIENTRYP PFNGLCULLFACEPROC)(GLenum mode);
typedef void   (APIENTRYP PFNGLFRONTFACEPROC)(GLenum mode);
typedef void   (APIENTRYP PFNGLLINEWIDTHPROC)(GLfloat width);
typedef void   (APIENTRYP PFNGLVIEWPORTPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void   (APIENTRYP PFNGLCLEARPROC)(GLbitfield mask);
typedef void   (APIENTRYP PFNGLCLEARCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void   (APIENTRYP PFNGLCLEARDEPTHPROC)(GLdouble depth);
typedef void   (APIENTRYP PFNGLDEPTHFUNCPROC)(GLenum func);
typedef void   (APIENTRYP PFNGLBLENDEQUATIONPROC)(GLenum mode);
typedef void   (APIENTRYP PFNGLBLENDFUNCPROC)(GLenum sfactor, GLenum dfactor);
typedef void   (APIENTRYP PFNGLENABLEPROC)(GLenum cap);
typedef void   (APIENTRYP PFNGLDISABLEPROC)(GLenum cap);
typedef GLenum (APIENTRYP PFNGLGETERRORPROC)(void);
typedef const GLubyte* (APIENTRYP PFNGLGETSTRINGPROC)(GLenum name);
typedef void   (APIENTRYP PFNGLPOLYGONMODEPROC)(GLenum face, GLenum mode);
typedef void   (APIENTRYP PFNGLSCISSORPROC)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void   (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum mode, GLint first, GLsizei count);
typedef void   (APIENTRYP PFNGLDRAWELEMENTSPROC)(GLenum mode, GLsizei count, GLenum type, const void* indices);
typedef void   (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei n, GLuint* textures);
typedef void   (APIENTRYP PFNGLDELETETEXTURESPROC)(GLsizei n, const GLuint* textures);
typedef void   (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum target, GLuint texture);
typedef void   (APIENTRYP PFNGLTEXIMAGE2DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
typedef void   (APIENTRYP PFNGLTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
typedef void   (APIENTRYP PFNGLTEXPARAMETERIPROC)(GLenum target, GLenum pname, GLint param);
typedef void   (APIENTRYP PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void   (APIENTRYP PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void   (APIENTRYP PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void   (APIENTRYP PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void   (APIENTRYP PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void   (APIENTRYP PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC)(GLenum type);
typedef void   (APIENTRYP PFNGLDELETESHADERPROC)(GLuint shader);
typedef void   (APIENTRYP PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
typedef void   (APIENTRYP PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void   (APIENTRYP PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void   (APIENTRYP PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC)(void);
typedef void   (APIENTRYP PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRYP PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void   (APIENTRYP PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void   (APIENTRYP PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRYP PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void   (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLint  (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
typedef void   (APIENTRYP PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void   (APIENTRYP PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void   (APIENTRYP PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);
typedef void   (APIENTRYP PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void   (APIENTRYP PFNGLUNIFORM4FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void   (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void   (APIENTRYP PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void   (APIENTRYP PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void   (APIENTRYP PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void   (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void   (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void   (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void   (APIENTRYP PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint* framebuffers);
typedef void   (APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint* framebuffers);
typedef void   (APIENTRYP PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void   (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
typedef void   (APIENTRYP PFNGLGENRENDERBUFFERSPROC)(GLsizei n, GLuint* renderbuffers);
typedef void   (APIENTRYP PFNGLDELETERENDERBUFFERSPROC)(GLsizei n, const GLuint* renderbuffers);
typedef void   (APIENTRYP PFNGLBINDRENDERBUFFERPROC)(GLenum target, GLuint renderbuffer);
typedef void   (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void   (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

/* ─── External declarations ─────────────────────────────────────────────────── */
extern PFNGLCULLFACEPROC                glad_glCullFace;
extern PFNGLFRONTFACEPROC               glad_glFrontFace;
extern PFNGLLINEWIDTHPROC               glad_glLineWidth;
extern PFNGLVIEWPORTPROC                glad_glViewport;
extern PFNGLCLEARPROC                   glad_glClear;
extern PFNGLCLEARCOLORPROC              glad_glClearColor;
extern PFNGLDEPTHFUNCPROC               glad_glDepthFunc;
extern PFNGLBLENDFUNCPROC               glad_glBlendFunc;
extern PFNGLENABLEPROC                  glad_glEnable;
extern PFNGLDISABLEPROC                 glad_glDisable;
extern PFNGLGETERRORPROC                glad_glGetError;
extern PFNGLGETSTRINGPROC               glad_glGetString;
extern PFNGLPOLYGONMODEPROC             glad_glPolygonMode;
extern PFNGLDRAWARRAYSPROC              glad_glDrawArrays;
extern PFNGLDRAWELEMENTSPROC            glad_glDrawElements;
extern PFNGLGENTEXTURESPROC             glad_glGenTextures;
extern PFNGLDELETETEXTURESPROC          glad_glDeleteTextures;
extern PFNGLBINDTEXTUREPROC             glad_glBindTexture;
extern PFNGLTEXIMAGE2DPROC              glad_glTexImage2D;
extern PFNGLTEXSUBIMAGE2DPROC           glad_glTexSubImage2D;
extern PFNGLTEXPARAMETERIPROC           glad_glTexParameteri;
extern PFNGLACTIVETEXTUREPROC           glad_glActiveTexture;
extern PFNGLGENBUFFERSPROC              glad_glGenBuffers;
extern PFNGLDELETEBUFFERSPROC           glad_glDeleteBuffers;
extern PFNGLBINDBUFFERPROC              glad_glBindBuffer;
extern PFNGLBUFFERDATAPROC              glad_glBufferData;
extern PFNGLBUFFERSUBDATAPROC           glad_glBufferSubData;
extern PFNGLCREATESHADERPROC            glad_glCreateShader;
extern PFNGLDELETESHADERPROC            glad_glDeleteShader;
extern PFNGLSHADERSOURCEPROC            glad_glShaderSource;
extern PFNGLCOMPILESHADERPROC           glad_glCompileShader;
extern PFNGLGETSHADERIVPROC             glad_glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC        glad_glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC           glad_glCreateProgram;
extern PFNGLDELETEPROGRAMPROC           glad_glDeleteProgram;
extern PFNGLATTACHSHADERPROC            glad_glAttachShader;
extern PFNGLLINKPROGRAMPROC             glad_glLinkProgram;
extern PFNGLUSEPROGRAMPROC              glad_glUseProgram;
extern PFNGLGETPROGRAMIVPROC            glad_glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC       glad_glGetProgramInfoLog;
extern PFNGLGETUNIFORMLOCATIONPROC      glad_glGetUniformLocation;
extern PFNGLUNIFORM1IPROC               glad_glUniform1i;
extern PFNGLUNIFORM1FPROC               glad_glUniform1f;
extern PFNGLUNIFORM2FPROC               glad_glUniform2f;
extern PFNGLUNIFORM3FPROC               glad_glUniform3f;
extern PFNGLUNIFORM4FPROC               glad_glUniform4f;
extern PFNGLUNIFORMMATRIX4FVPROC        glad_glUniformMatrix4fv;
extern PFNGLGENVERTEXARRAYSPROC         glad_glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC      glad_glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC         glad_glBindVertexArray;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC     glad_glVertexAttribPointer;
extern PFNGLGENFRAMEBUFFERSPROC         glad_glGenFramebuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC      glad_glDeleteFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC         glad_glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC    glad_glFramebufferTexture2D;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC  glad_glCheckFramebufferStatus;
extern PFNGLGENRENDERBUFFERSPROC        glad_glGenRenderbuffers;
extern PFNGLDELETERENDERBUFFERSPROC     glad_glDeleteRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC        glad_glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC     glad_glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer;

/* Macros */
#define glCullFace                  glad_glCullFace
#define glFrontFace                 glad_glFrontFace
#define glLineWidth                 glad_glLineWidth
#define glViewport                  glad_glViewport
#define glClear                     glad_glClear
#define glClearColor                glad_glClearColor
#define glDepthFunc                 glad_glDepthFunc
#define glBlendFunc                 glad_glBlendFunc
#define glEnable                    glad_glEnable
#define glDisable                   glad_glDisable
#define glGetError                  glad_glGetError
#define glGetString                 glad_glGetString
#define glPolygonMode               glad_glPolygonMode
#define glDrawArrays                glad_glDrawArrays
#define glDrawElements              glad_glDrawElements
#define glGenTextures               glad_glGenTextures
#define glDeleteTextures            glad_glDeleteTextures
#define glBindTexture               glad_glBindTexture
#define glTexImage2D                glad_glTexImage2D
#define glTexSubImage2D             glad_glTexSubImage2D
#define glTexParameteri             glad_glTexParameteri
#define glActiveTexture             glad_glActiveTexture
#define glGenBuffers                glad_glGenBuffers
#define glDeleteBuffers             glad_glDeleteBuffers
#define glBindBuffer                glad_glBindBuffer
#define glBufferData                glad_glBufferData
#define glBufferSubData             glad_glBufferSubData
#define glCreateShader              glad_glCreateShader
#define glDeleteShader              glad_glDeleteShader
#define glShaderSource              glad_glShaderSource
#define glCompileShader             glad_glCompileShader
#define glGetShaderiv               glad_glGetShaderiv
#define glGetShaderInfoLog          glad_glGetShaderInfoLog
#define glCreateProgram             glad_glCreateProgram
#define glDeleteProgram             glad_glDeleteProgram
#define glAttachShader              glad_glAttachShader
#define glLinkProgram               glad_glLinkProgram
#define glUseProgram                glad_glUseProgram
#define glGetProgramiv              glad_glGetProgramiv
#define glGetProgramInfoLog         glad_glGetProgramInfoLog
#define glGetUniformLocation        glad_glGetUniformLocation
#define glUniform1i                 glad_glUniform1i
#define glUniform1f                 glad_glUniform1f
#define glUniform2f                 glad_glUniform2f
#define glUniform3f                 glad_glUniform3f
#define glUniform4f                 glad_glUniform4f
#define glUniformMatrix4fv          glad_glUniformMatrix4fv
#define glGenVertexArrays           glad_glGenVertexArrays
#define glDeleteVertexArrays        glad_glDeleteVertexArrays
#define glBindVertexArray           glad_glBindVertexArray
#define glEnableVertexAttribArray   glad_glEnableVertexAttribArray
#define glDisableVertexAttribArray  glad_glDisableVertexAttribArray
#define glVertexAttribPointer       glad_glVertexAttribPointer
#define glGenFramebuffers           glad_glGenFramebuffers
#define glDeleteFramebuffers        glad_glDeleteFramebuffers
#define glBindFramebuffer           glad_glBindFramebuffer
#define glFramebufferTexture2D      glad_glFramebufferTexture2D
#define glCheckFramebufferStatus    glad_glCheckFramebufferStatus
#define glGenRenderbuffers          glad_glGenRenderbuffers
#define glDeleteRenderbuffers       glad_glDeleteRenderbuffers
#define glBindRenderbuffer          glad_glBindRenderbuffer
#define glRenderbufferStorage       glad_glRenderbufferStorage
#define glFramebufferRenderbuffer   glad_glFramebufferRenderbuffer

int gladLoadGL(void);
typedef void* (*GLADloadproc)(const char* name);
int gladLoadGLLoader(GLADloadproc);

#ifdef __cplusplus
}
#endif

#endif /* __glad_h_ */
