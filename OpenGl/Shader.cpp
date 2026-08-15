#include "Shader.h"
#include "Renderer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cerrno>
#include <system_error>

enum class ShaderType
{
    NONE = -1, VERTEX = 0, FRAGMENT = 1

};

Shader::Shader(const std::string& filepath)
    : m_FilePath(filepath), m_RendererID(0), m_vertexPath(""), m_fragmentPath("")
{
    ShaderProgramSource source = ParseShader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
     
}

Shader::Shader(const std::string& vertexPath,const std::string& fragmentPath)
    : m_FilePath(""), m_RendererID(0), m_vertexPath(vertexPath), m_fragmentPath(fragmentPath)
{
    ShaderProgramSource source = ParseShader(vertexPath, fragmentPath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
     
}

Shader::~Shader()
{ 
    GLCall(glDeleteProgram(m_RendererID));
}

ShaderProgramSource Shader::ParseShader(const std::string& filepath) 
{
    std::ifstream stream(filepath);
    if (!stream.is_open())
    {
        std::string _errMsg;
#if defined(_MSC_VER)
        char _errbuf[256];
        strerror_s(_errbuf, sizeof(_errbuf), errno);
        _errMsg = _errbuf;
#else
        _errMsg = std::error_code(errno, std::generic_category()).message();
#endif
        std::cerr << "I/O Error encountered: " << filepath << '\n';
        std::cerr << "System message: " << _errMsg << '\n';

    }
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }


    return { ss[(int)ShaderType::VERTEX].str(), ss[(int)ShaderType::FRAGMENT].str()};

}

ShaderProgramSource Shader::ParseShader(const std::string& vertexFilePath, const std::string& fragmentFilePath) 
{
    return { ReadFile(vertexFilePath), ReadFile(fragmentFilePath) };


}

static std::string ReadFile(const std::string& filepath) 
{
    std::ifstream stream(filepath);

    if (!stream.is_open())
    {
        std::string _errMsg;
#if defined(_MSC_VER)
        char _errbuf[256];
        strerror_s(_errbuf, sizeof(_errbuf), errno);
        _errMsg = _errbuf;
#else
        _errMsg = std::error_code(errnom, std::generic_category()).message();
#endif
        std::cerr << "I/O Error encountered: " << filepath << '\n';
        std::cerr << "System message: " << _errMsg <<  '\n';
    
        return "";
    }
    
    std::stringstream ss;
    ss << stream.rdbuf();
    stream.close();
    return ss.str();


}


unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    GLCall(const char* src = source.c_str());
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" <<
            std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;

    }

    return id;

}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader) 
{
    GLCall(unsigned int program = glCreateProgram());
    GLCall(unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader));
    GLCall(unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader));

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));

}
void Shader::Unbind() const
{
    GLCall(glUseProgram(0));

}
void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

unsigned int Shader::GetUniformLocation(const std::string& name)
{
    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1)
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    return location;
}
