#pragma once
#include <glad/glad.h>
#include <stdexcept>
#include <string>

struct Shader
{
    GLuint program;

    Shader(char const * const vsSource, char const * const fsSource);
    ~Shader() {glDeleteProgram(program);}

    operator GLuint() const noexcept {return program;}
};

inline Shader::Shader( char const * const vsSource
                     , char const * const fsSource
                     )
    : program(glCreateProgram())
{
    auto const compileShader = [](GLenum const kind, char const * const src)
    {
        GLuint shader = glCreateShader(kind);
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint isCreated;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCreated);
        if(isCreated != GL_TRUE)
        {
            GLint size = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &size);
            std::string err(std::size_t(size), '\0');
            glGetShaderInfoLog(shader, size, nullptr, err.data());
            throw std::runtime_error("failed to compile shader\n" + err);
        }
        return shader;
    };

    GLuint const   vertexShader = compileShader(GL_VERTEX_SHADER  , vsSource);
    GLuint const fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program,   vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(fragmentShader);
    glDeleteShader(  vertexShader);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if(linked != GL_TRUE)
    {
        GLint size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &size);
        std::string err(std::size_t(size), '\0');
        glGetProgramInfoLog(program, size, nullptr, err.data());
        throw std::runtime_error("failed to link shader: " + err);
    }
}
