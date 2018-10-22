#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <fstream>
#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"

#include "Shader.h"

class App
{
public:
   App( float width, float height ) :
      m_window( nullptr ), m_shader( nullptr ),
      m_width( width ), m_height( height )
   {
   }

   ~App( )
   {
      delete m_shader;
   }

   static void FrameBufferSizeCallBcak( GLFWwindow* window, int width, int height )
   {
      glViewport( 0, 0, width, height );
   }

   void ProcessInput( GLFWwindow* window )
   {
      if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) )
      {
         glfwSetWindowShouldClose( window, true );
      }
   }

   int Run( )
   {
      if ( !Init( ) )
      {
         return -1;
      }

      glm::vec3 cubePositions[ ] = {
         glm::vec3( 0.0f,  0.0f,  0.0f ),
         glm::vec3( 2.0f,  5.0f, -15.0f ),
         glm::vec3( -1.5f, -2.2f, -2.5f ),
         glm::vec3( -3.8f, -2.0f, -12.3f ),
         glm::vec3( 2.4f, -0.4f, -3.5f ),
         glm::vec3( -1.7f,  3.0f, -7.5f ),
         glm::vec3( 1.3f, -2.0f, -2.5f ),
         glm::vec3( 1.5f,  2.0f, -2.5f ),
         glm::vec3( 1.5f,  0.2f, -1.5f ),
         glm::vec3( -1.3f,  1.0f, -1.5f )
      };

      while ( !glfwWindowShouldClose( m_window ) )
      {
         ProcessInput( m_window );

         glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
         glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

         glActiveTexture( GL_TEXTURE0 );
         glBindTexture( GL_TEXTURE_2D, m_textures[ 0 ] );
         glActiveTexture( GL_TEXTURE1 );
         glBindTexture( GL_TEXTURE_2D, m_textures[ 1 ] );
         m_shader->SetInt( "texture1", 0 );
         m_shader->SetInt( "texture2", 1 );


         float radius = 10.0f;
         float camX = sin( glfwGetTime( ) ) * radius;
         float camZ = cos( glfwGetTime( ) ) * radius;
         m_view = glm::lookAt( glm::vec3( camX, 0.0f, camZ ),
                               glm::vec3( 0.0f, 0.0f, 0.0f ),
                               glm::vec3( 0.0f, 1.0f, 0.0f ) );
         m_shader->SetMat4f( "view", m_view );
         m_shader->SetMat4f( "proj", m_proj );

         m_shader->Use( );

         glBindVertexArray( m_vao );
         for ( int idx = 0; idx < 10; ++idx )
         {
            glm::mat4 model;
            model = glm::translate( model, cubePositions[ idx ] );
            float angle = 20.0f * idx;
            model = glm::rotate( model, ( float ) glfwGetTime( ) * glm::radians( angle ), glm::vec3( 1.0f, 0.3f, 0.5f ) );

            m_shader->SetMat4f( "model", model );
            glDrawArrays( GL_TRIANGLES, 0, 36 );
         }

         glfwSwapBuffers( m_window );
         glfwPollEvents( );
      }

      glfwTerminate( );
      return 0;
   }

   bool Init( )
   {
      glfwInit( );
      glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
      glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
      glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

      m_window = glfwCreateWindow( ( int ) m_width, ( int ) m_height, "TEST", nullptr, nullptr );
      if ( m_window == nullptr )
      {
         std::cout << "Failed to init GLFW" << std::endl;
         glfwTerminate( );
         return false;
      }

      glfwMakeContextCurrent( m_window );

      if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
      {
         std::cout << "Failed to init GLAD" << std::endl;
         return false;
      }

      glViewport( 0, 0, m_width, m_height );
      glfwSetFramebufferSizeCallback( m_window, App::FrameBufferSizeCallBcak );

      // Pos TexCoord
      float vertices[ ] = {
         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

         -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
      };

      glGenVertexArrays( 1, &m_vao );
      glBindVertexArray( m_vao );

      glGenBuffers( 1, &m_vbo );
      glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
      glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), &vertices, GL_STATIC_DRAW );

      // location, Number of elements, type, normalize?, stride, offset
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 5, ( void* ) 0 );
      glEnableVertexAttribArray( 0 );

      glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 5, ( void* ) ( 3 * sizeof( float ) ) );
      glEnableVertexAttribArray( 1 );

      // Load Shader program
      m_shader = new Shader( "../Resources/Shaders/BasicVS.glsl", "../Resources/Shaders/BasicPS.glsl" );

      glGenTextures( 2, m_textures );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, m_textures[ 0 ] );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); // Up scailing => Use Linear (Mag)
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ); // Down scailing => Use Nearest filter (Min)

                                                                           // Texture Down Scailing => Use Mipmaps! (effective memory usage!)
                                                                           // glGenerateMipmaps..
                                                                           // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR..NEAREST )


                                                                           // Load Textures
      int width, height, nrChannels;
      stbi_set_flip_vertically_on_load( true );
      unsigned char* data = stbi_load( "../Resources/Textures/wooden_container.jpg", &width, &height, &nrChannels, 0 );
      if ( data != nullptr )
      {
         glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
         glGenerateMipmap( GL_TEXTURE_2D );
      }
      else
      {
         std::cout << "Failed to load texture" << std::endl;
      }
      stbi_image_free( data );

      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, m_textures[ 1 ] );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

      data = stbi_load( "../Resources/Textures/awesomeface.png", &width, &height, &nrChannels, 0 );
      if ( data != nullptr )
      {
         glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
         glGenerateMipmap( GL_TEXTURE_2D );
      }
      else
      {
         std::cout << "Failed to load awesomeface.png" << std::endl;
      }
      stbi_image_free( data );

      m_view = glm::translate( m_view, glm::vec3( 0.0f, 0.0f, -3.0f ) );
      m_proj = glm::perspective( glm::radians( 45.0f ), m_width / m_height, 1.0f, 1000.0f );

      glEnable( GL_DEPTH_TEST );

      return true;
   }

private:
   float m_width;
   float m_height;

   unsigned int m_textures[ 2 ];

   GLFWwindow* m_window;
   Shader*     m_shader;

   unsigned int m_vao;
   unsigned int m_vbo;

   glm::mat4   m_view;
   glm::mat4   m_proj;

};

int main( )
{
   App app{ 800, 600 };
   return app.Run( );
}