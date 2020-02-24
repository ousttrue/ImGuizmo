#include "glutil.h"

static bool mInitExtensionsDone = false;
static bool mExtensionsPresent = false;

bool loadExtension(const char *extensionName, void **functionPtr)
{
#if defined(_WIN32)
    *functionPtr = glGetProcAddress(extensionName);
#else
    *functionPtr = (void *)glGetProcAddress((const GLubyte *)extensionName);
#endif
    return (*functionPtr != NULL);
}

#define LE(x) mExtensionsPresent &= loadExtension(#x, (void **)&x);

bool InitExtension()
{
    if (mInitExtensionsDone)
        return true;

    mExtensionsPresent = true;

    LE(glUniform1i);       //GLint location, GLint v0);
    LE(glUniformMatrix3fv) // GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
#ifdef _MSC_VER
    LE(glActiveTexture); //GLenum texture);
#endif
    LE(glBindFramebuffer);          //GLenum target, TextureID framebuffer);
    LE(glDeleteFramebuffers);       //GLsizei n, const TextureID* framebuffers);
    LE(glDeleteRenderbuffers);      //GLsizei n, const TextureID* renderbuffers);
    LE(glFramebufferTexture2D);     //GLenum target, GLenum attachment, GLenum textarget, TextureID texture, GLint level);
    LE(glFramebufferRenderbuffer);  //GLenum target, GLenum attachment, GLenum renderbuffertarget, TextureID renderbuffer);
    LE(glRenderbufferStorage);      //GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    LE(glGenFramebuffers);          //GLsizei n, TextureID* framebuffers);
    LE(glGenRenderbuffers);         //GLsizei n, TextureID* renderbuffers);
    LE(glBindRenderbuffer);         //GLenum target, TextureID renderbuffer);
    LE(glCheckFramebufferStatus);   //GLenum target);
    LE(glGenerateMipmap);           //GLenum target);
    LE(glBufferData);               //GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
    LE(glUseProgram);               //TextureID program);
    LE(glGetUniformLocation);       //TextureID program, const GLchar* name);
    LE(glGetAttribLocation);        //TextureID program, const GLchar* name);
    LE(glDeleteBuffers);            //GLsizei n, const TextureID* buffers);
    LE(glDeleteVertexArrays);       //GLsizei n, const TextureID* arrays);
    LE(glEnableVertexAttribArray);  //TextureID);
    LE(glVertexAttribPointer);      //TextureID index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
    LE(glGenBuffers);               //GLsizei n, TextureID* buffers);
    LE(glBindBuffer);               //GLenum target, TextureID buffer);
    LE(glCreateShader);             //GLenum type);
    LE(glShaderSource);             //TextureID shader, GLsizei count, const GLchar** strings, const GLint* lengths);
    LE(glCompileShader);            //TextureID shader);
    LE(glCreateProgram);            //void);
    LE(glAttachShader);             //TextureID program, TextureID shader);
    LE(glDeleteProgram);            //TextureID program);
    LE(glDeleteShader);             //TextureID shader);
    LE(glDisableVertexAttribArray); //TextureID);
    LE(glBindAttribLocation);       //TextureID program, TextureID index, const GLchar* name);
    LE(glVertexAttribDivisor);      //TextureID index, TextureID divisor);
    LE(glUniformMatrix4fv);         //GLint location, GLsizei count, GLboolean transpose, const float* value);
    LE(glGetShaderiv);              //TextureID shader, GLenum pname, GLint* param);
    LE(glLinkProgram);              //TextureID program);
    LE(glGetProgramiv);             //TextureID program, GLenum pname, GLint* param);
    LE(glBindVertexArray);          //TextureID array);
    LE(glUniform2fv);
    LE(glUniform3f);                       //GLint location, float v0, float v1, float v2);
    LE(glUniform3fv);                      //GLint location, GLsizei count, const float* value);
    LE(glUniform4fv);                      //GLint location, GLsizei count, const float* value);
    LE(glBufferSubData);                   //GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
    LE(glGenVertexArrays);                 //GLsizei n, const TextureID* arrays);
    LE(glGetShaderInfoLog);                //TextureID shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    LE(glGetProgramInfoLog);               //TextureID program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
    LE(glGetUniformBlockIndex);            //TextureID program, const GLchar* uniformBlockName);
    LE(glUniformBlockBinding);             //TextureID program, TextureID uniformBlockIndex, TextureID uniformBlockBinding);
    LE(glBindBufferBase);                  //GLenum target, TextureID index, TextureID buffer);
    LE(glTransformFeedbackVaryings);       //TextureID, GLsizei, const GLchar **, GLenum);
    LE(glMapBuffer);                       //GLenum target, GLenum access);
    LE(glUnmapBuffer);                     //GLenum target);
    LE(glDrawElementsInstanced);           //GLenum, GLsizei, GLenum, const GLvoid*, GLsizei);
    LE(glDrawArraysInstanced);             //GLenum, GLint, GLsizei, GLsizei);
    LE(glDrawElementsInstancedBaseVertex); //GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount, GLint basevertex);
    LE(glBeginTransformFeedback);          //GLenum);
    LE(glEndTransformFeedback);            //void);
    LE(glUniform1f);                       //GLint location, float v0);
    LE(glUniform2f);                       //GLint location, float v0, float v1);
    LE(glBlendEquationSeparate);           //GLenum, GLenum);
    LE(glBlendFuncSeparate);               //GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    LE(glGetBufferSubData);                //GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
    LE(glGetShaderSource);
    LE(glIsProgram);
    LE(glGetAttachedShaders);
    LE(glDrawBuffers);
    LE(glBlitFramebuffer);
    LE(glBlendEquation);
    LE(glBindSampler);
    LE(glDetachShader);

    return mExtensionsPresent;
}
