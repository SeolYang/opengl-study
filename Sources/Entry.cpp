#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <fstream>
#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"

void FrameBufferSizeCallBcak( GLFWwindow* window, int width, int height )
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

int main( )
{
   glfwInit( );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

   GLFWwindow* window = glfwCreateWindow( 800, 600, "TEST", nullptr, nullptr );
   if ( window == nullptr )
   {
      std::cout << "Failed to init GLFW" << std::endl;
      glfwTerminate( );
      return -1;
   }

   glfwMakeContextCurrent( window );

   if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
   {
      std::cout << "Failed to init GLAD" << std::endl;
      return -1;
   }

   glViewport( 0, 0, 800, 600 );
   glfwSetFramebufferSizeCallback( window, FrameBufferSizeCallBcak );

   // Pos / Color / TexCoord
   float vertices[ ] = {
      0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
      0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
      -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
   };

   unsigned int indices[ ] = {
      0, 3, 1,
      1, 2, 3
   };

   unsigned int VAO{ 0 };
   glGenVertexArrays( 1, &VAO );
   glBindVertexArray( VAO );

   unsigned int VBO{ 0 };
   glGenBuffers( 1, &VBO );
   glBindBuffer( GL_ARRAY_BUFFER, VBO );
   glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), &vertices, GL_STATIC_DRAW );

   // location, Number of elements, type, normalize?, stride, offset
   glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 5, ( void* ) 0 );
   glEnableVertexAttribArray( 0 );

   glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( GL_FLOAT ) * 5, ( void* ) ( 3 * sizeof( float ) ) );
   glEnableVertexAttribArray( 1 );

   unsigned int EBO{ 0 };
   glGenBuffers( 1, &EBO );
   glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
   glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

   // Load Shader program
   Shader simpleShader{ "../Resources/Shaders/BasicVS.glsl", "../Resources/Shaders/BasicPS.glsl" };

   unsigned int textures[ ]{ 0, 0 };
   glGenTextures( 2, textures );
   glActiveTexture( GL_TEXTURE0 );
   glBindTexture( GL_TEXTURE_2D, textures[0] );

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
   glBindTexture( GL_TEXTURE_2D, textures[ 1 ] );

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

   // transform with matrix
   glm::mat4 trans;
   trans = glm::rotate( trans, glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
   trans = glm::scale( trans, glm::vec3( 0.5f, 0.5f, 0.5f ) );

   while ( !glfwWindowShouldClose( window ) )
   {
      ProcessInput( window );

      glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT );

      float timeValue = glfwGetTime( );
      float redValue = ( cos( timeValue ) / 2.0f ) + 0.5f;
      float greenValue = ( sin( timeValue ) / 2.0f ) + 0.5f;
      float blueValue = ( redValue + greenValue ) / 2.0f;
      simpleShader.SetVec3f( "ourColor", redValue, greenValue, blueValue );

      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, textures[ 0 ] );
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, textures[ 1 ] );
      simpleShader.SetInt( "texture1", 0 );
      simpleShader.SetInt( "texture2", 1 );

      unsigned int transformLoc = glGetUniformLocation( simpleShader.GetID( ), "transform" );
      glUniformMatrix4fv( transformLoc, 1, GL_FALSE, glm::value_ptr( trans ) );

      simpleShader.Use( );

      glBindVertexArray( VAO );
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
      glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

      glfwSwapBuffers( window );
      glfwPollEvents( );
   }

   glfwTerminate( );
   return 0;
}