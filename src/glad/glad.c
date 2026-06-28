#include "glad.h"
#include <string.h>
#include <stdlib.h>

/* ─── Function pointers ─────────────────────────────────────────────────────── */
PFNGLCULLFACEPROC                glad_glCullFace               = NULL;
PFNGLFRONTFACEPROC               glad_glFrontFace              = NULL;
PFNGLLINEWIDTHPROC               glad_glLineWidth              = NULL;
PFNGLVIEWPORTPROC                glad_glViewport               = NULL;
PFNGLCLEARPROC                   glad_glClear                  = NULL;
PFNGLCLEARCOLORPROC              glad_glClearColor             = NULL;
PFNGLDEPTHFUNCPROC               glad_glDepthFunc              = NULL;
PFNGLBLENDFUNCPROC               glad_glBlendFunc              = NULL;
PFNGLENABLEPROC                  glad_glEnable                 = NULL;
PFNGLDISABLEPROC                 glad_glDisable                = NULL;
PFNGLGETERRORPROC                glad_glGetError               = NULL;
PFNGLGETSTRINGPROC               glad_glGetString              = NULL;
PFNGLPOLYGONMODEPROC             glad_glPolygonMode            = NULL;
PFNGLDRAWARRAYSPROC              glad_glDrawArrays             = NULL;
PFNGLDRAWELEMENTSPROC            glad_glDrawElements           = NULL;
PFNGLGENTEXTURESPROC             glad_glGenTextures            = NULL;
PFNGLDELETETEXTURESPROC          glad_glDeleteTextures         = NULL;
PFNGLBINDTEXTUREPROC             glad_glBindTexture            = NULL;
PFNGLTEXIMAGE2DPROC              glad_glTexImage2D             = NULL;
PFNGLTEXSUBIMAGE2DPROC           glad_glTexSubImage2D          = NULL;
PFNGLTEXPARAMETERIPROC           glad_glTexParameteri          = NULL;
PFNGLACTIVETEXTUREPROC           glad_glActiveTexture          = NULL;
PFNGLGENBUFFERSPROC              glad_glGenBuffers             = NULL;
PFNGLDELETEBUFFERSPROC           glad_glDeleteBuffers          = NULL;
PFNGLBINDBUFFERPROC              glad_glBindBuffer             = NULL;
PFNGLBUFFERDATAPROC              glad_glBufferData             = NULL;
PFNGLBUFFERSUBDATAPROC           glad_glBufferSubData          = NULL;
PFNGLCREATESHADERPROC            glad_glCreateShader           = NULL;
PFNGLDELETESHADERPROC            glad_glDeleteShader           = NULL;
PFNGLSHADERSOURCEPROC            glad_glShaderSource           = NULL;
PFNGLCOMPILESHADERPROC           glad_glCompileShader          = NULL;
PFNGLGETSHADERIVPROC             glad_glGetShaderiv            = NULL;
PFNGLGETSHADERINFOLOGPROC        glad_glGetShaderInfoLog       = NULL;
PFNGLCREATEPROGRAMPROC           glad_glCreateProgram          = NULL;
PFNGLDELETEPROGRAMPROC           glad_glDeleteProgram          = NULL;
PFNGLATTACHSHADERPROC            glad_glAttachShader           = NULL;
PFNGLLINKPROGRAMPROC             glad_glLinkProgram            = NULL;
PFNGLUSEPROGRAMPROC              glad_glUseProgram             = NULL;
PFNGLGETPROGRAMIVPROC            glad_glGetProgramiv           = NULL;
PFNGLGETPROGRAMINFOLOGPROC       glad_glGetProgramInfoLog      = NULL;
PFNGLGETUNIFORMLOCATIONPROC      glad_glGetUniformLocation     = NULL;
PFNGLUNIFORM1IPROC               glad_glUniform1i              = NULL;
PFNGLUNIFORM1FPROC               glad_glUniform1f              = NULL;
PFNGLUNIFORM2FPROC               glad_glUniform2f              = NULL;
PFNGLUNIFORM3FPROC               glad_glUniform3f              = NULL;
PFNGLUNIFORM4FPROC               glad_glUniform4f              = NULL;
PFNGLUNIFORMMATRIX4FVPROC        glad_glUniformMatrix4fv       = NULL;
PFNGLGENVERTEXARRAYSPROC         glad_glGenVertexArrays        = NULL;
PFNGLDELETEVERTEXARRAYSPROC      glad_glDeleteVertexArrays     = NULL;
PFNGLBINDVERTEXARRAYPROC         glad_glBindVertexArray        = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray= NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_glDisableVertexAttribArray= NULL;
PFNGLVERTEXATTRIBPOINTERPROC     glad_glVertexAttribPointer    = NULL;
PFNGLGENFRAMEBUFFERSPROC         glad_glGenFramebuffers        = NULL;
PFNGLDELETEFRAMEBUFFERSPROC      glad_glDeleteFramebuffers     = NULL;
PFNGLBINDFRAMEBUFFERPROC         glad_glBindFramebuffer        = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC    glad_glFramebufferTexture2D   = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC  glad_glCheckFramebufferStatus = NULL;
PFNGLGENRENDERBUFFERSPROC        glad_glGenRenderbuffers       = NULL;
PFNGLDELETERENDERBUFFERSPROC     glad_glDeleteRenderbuffers    = NULL;
PFNGLBINDRENDERBUFFERPROC        glad_glBindRenderbuffer       = NULL;
PFNGLRENDERBUFFERSTORAGEPROC     glad_glRenderbufferStorage    = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer= NULL;

/* ─── Loader ─────────────────────────────────────────────────────────────────── */
static GLADloadproc s_loader = NULL;

static void* load(const char* name) {
    return s_loader ? s_loader(name) : NULL;
}

int gladLoadGLLoader(GLADloadproc loader) {
    s_loader = loader;

#define LOAD(fn, type) glad_##fn = (type)load(#fn); if (!glad_##fn) return 0;

    /* Core 1.0 / 1.1 */
    LOAD(glCullFace,              PFNGLCULLFACEPROC)
    LOAD(glFrontFace,             PFNGLFRONTFACEPROC)
    LOAD(glLineWidth,             PFNGLLINEWIDTHPROC)
    LOAD(glViewport,              PFNGLVIEWPORTPROC)
    LOAD(glClear,                 PFNGLCLEARPROC)
    LOAD(glClearColor,            PFNGLCLEARCOLORPROC)
    LOAD(glDepthFunc,             PFNGLDEPTHFUNCPROC)
    LOAD(glBlendFunc,             PFNGLBLENDFUNCPROC)
    LOAD(glEnable,                PFNGLENABLEPROC)
    LOAD(glDisable,               PFNGLDISABLEPROC)
    LOAD(glGetError,              PFNGLGETERRORPROC)
    LOAD(glGetString,             PFNGLGETSTRINGPROC)
    LOAD(glPolygonMode,           PFNGLPOLYGONMODEPROC)
    LOAD(glDrawArrays,            PFNGLDRAWARRAYSPROC)
    LOAD(glDrawElements,          PFNGLDRAWELEMENTSPROC)
    LOAD(glGenTextures,           PFNGLGENTEXTURESPROC)
    LOAD(glDeleteTextures,        PFNGLDELETETEXTURESPROC)
    LOAD(glBindTexture,           PFNGLBINDTEXTUREPROC)
    LOAD(glTexImage2D,            PFNGLTEXIMAGE2DPROC)
    LOAD(glTexSubImage2D,         PFNGLTEXSUBIMAGE2DPROC)
    LOAD(glTexParameteri,         PFNGLTEXPARAMETERIPROC)

    /* Core 1.3 */
    LOAD(glActiveTexture,         PFNGLACTIVETEXTUREPROC)

    /* Core 1.5 / 2.0 */
    LOAD(glGenBuffers,            PFNGLGENBUFFERSPROC)
    LOAD(glDeleteBuffers,         PFNGLDELETEBUFFERSPROC)
    LOAD(glBindBuffer,            PFNGLBINDBUFFERPROC)
    LOAD(glBufferData,            PFNGLBUFFERDATAPROC)
    LOAD(glBufferSubData,         PFNGLBUFFERSUBDATAPROC)
    LOAD(glCreateShader,          PFNGLCREATESHADERPROC)
    LOAD(glDeleteShader,          PFNGLDELETESHADERPROC)
    LOAD(glShaderSource,          PFNGLSHADERSOURCEPROC)
    LOAD(glCompileShader,         PFNGLCOMPILESHADERPROC)
    LOAD(glGetShaderiv,           PFNGLGETSHADERIVPROC)
    LOAD(glGetShaderInfoLog,      PFNGLGETSHADERINFOLOGPROC)
    LOAD(glCreateProgram,         PFNGLCREATEPROGRAMPROC)
    LOAD(glDeleteProgram,         PFNGLDELETEPROGRAMPROC)
    LOAD(glAttachShader,          PFNGLATTACHSHADERPROC)
    LOAD(glLinkProgram,           PFNGLLINKPROGRAMPROC)
    LOAD(glUseProgram,            PFNGLUSEPROGRAMPROC)
    LOAD(glGetProgramiv,          PFNGLGETPROGRAMIVPROC)
    LOAD(glGetProgramInfoLog,     PFNGLGETPROGRAMINFOLOGPROC)
    LOAD(glGetUniformLocation,    PFNGLGETUNIFORMLOCATIONPROC)
    LOAD(glUniform1i,             PFNGLUNIFORM1IPROC)
    LOAD(glUniform1f,             PFNGLUNIFORM1FPROC)
    LOAD(glUniform2f,             PFNGLUNIFORM2FPROC)
    LOAD(glUniform3f,             PFNGLUNIFORM3FPROC)
    LOAD(glUniform4f,             PFNGLUNIFORM4FPROC)
    LOAD(glUniformMatrix4fv,      PFNGLUNIFORMMATRIX4FVPROC)
    LOAD(glEnableVertexAttribArray,  PFNGLENABLEVERTEXATTRIBARRAYPROC)
    LOAD(glDisableVertexAttribArray, PFNGLDISABLEVERTEXATTRIBARRAYPROC)
    LOAD(glVertexAttribPointer,   PFNGLVERTEXATTRIBPOINTERPROC)

    /* Core 3.0 / 3.3 */
    LOAD(glGenVertexArrays,       PFNGLGENVERTEXARRAYSPROC)
    LOAD(glDeleteVertexArrays,    PFNGLDELETEVERTEXARRAYSPROC)
    LOAD(glBindVertexArray,       PFNGLBINDVERTEXARRAYPROC)
    LOAD(glGenFramebuffers,       PFNGLGENFRAMEBUFFERSPROC)
    LOAD(glDeleteFramebuffers,    PFNGLDELETEFRAMEBUFFERSPROC)
    LOAD(glBindFramebuffer,       PFNGLBINDFRAMEBUFFERPROC)
    LOAD(glFramebufferTexture2D,  PFNGLFRAMEBUFFERTEXTURE2DPROC)
    LOAD(glCheckFramebufferStatus,PFNGLCHECKFRAMEBUFFERSTATUSPROC)
    LOAD(glGenRenderbuffers,      PFNGLGENRENDERBUFFERSPROC)
    LOAD(glDeleteRenderbuffers,   PFNGLDELETERENDERBUFFERSPROC)
    LOAD(glBindRenderbuffer,      PFNGLBINDRENDERBUFFERPROC)
    LOAD(glRenderbufferStorage,   PFNGLRENDERBUFFERSTORAGEPROC)
    LOAD(glFramebufferRenderbuffer,PFNGLFRAMEBUFFERRENDERBUFFERPROC)

#undef LOAD

    return 1;
}

int gladLoadGL(void) {
    return 0; /* Use gladLoadGLLoader instead */
}
