#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include <Camera.h>
#include <Model.h>

#include <iostream>
const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

void processInput( GLFWwindow *window );
void framebuffer_size_callback( GLFWwindow* window, int width, int height );
void mouse_callback( GLFWwindow* window, double xpos, double ypos );
void scroll_callback( GLFWwindow* window, double xoffset, double yoffset );
unsigned int LoadTexture( const std::string& path );
unsigned int LoadCubeMap( const std::vector<std::string>& faces );

void renderScene( const Shader& shader );
void renderCube( );
void renderQuad( );

Camera camera{ glm::vec3( 0.0f, 0.0f, 3.0f ) };
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastTime = 0.0f;

// Meshes
unsigned int planeVAO;

enum RenderPass
{
   Shadow = 0,
   Render,
   EnumMax
};

int main( )
{
   unsigned int frameCount = 0;
   float elasedTime = 0.0;
   constexpr float UnitTime = 1.0f;

   constexpr unsigned int Samples = 4;

   glfwInit( );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
   glfwWindowHint( GLFW_SAMPLES, Samples );

   GLFWwindow* window = glfwCreateWindow( SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", nullptr, nullptr );
   if ( window == NULL )
   {
      std::cout << "Failed to create window" << std::endl;
   }
   glfwMakeContextCurrent( window );
   glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
   glfwSetCursorPosCallback( window, mouse_callback );
   glfwSetScrollCallback( window, scroll_callback );

   glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

   if ( !gladLoadGLLoader( ( GLADloadproc ) glfwGetProcAddress ) )
   {
      std::cout << "Failed to init glad" << std::endl;
      return -1;
   }

#pragma region Initialize Region
   glEnable( GL_DEPTH_TEST );
   //glEnable( GL_FRAMEBUFFER_SRGB );

   Shader shader{ "../Resources/Shaders/ShadowMapping.vs", "../Resources/Shaders/ShadowMapping.fs" };
   Shader simpleDepthShader{ "../Resources/Shaders/ShadowMappingDepth.vs", "../Resources/Shaders/ShadowMappingDepth.fs" };
   Shader debugDepthQuad{ "../Resources/Shaders/DebugQuad.vs", "../Resources/Shaders/DebugQuad.fs" };
   float planeVertices[ ] = {
      // positions            // normals         // texcoords
      25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
      -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
      -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

      25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
      -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
      25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 10.0f
   };

   // plane VAO
   unsigned int planeVBO;
   glGenVertexArrays( 1, &planeVAO );
   glGenBuffers( 1, &planeVBO );
   glBindVertexArray( planeVAO );
   glBindBuffer( GL_ARRAY_BUFFER, planeVBO );
   glBufferData( GL_ARRAY_BUFFER, sizeof( planeVertices ), planeVertices, GL_STATIC_DRAW );
   glEnableVertexAttribArray( 0 );
   glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) 0 );
   glEnableVertexAttribArray( 1 );
   glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) ( 3 * sizeof( float ) ) );
   glEnableVertexAttribArray( 2 );
   glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) ( 6 * sizeof( float ) ) );
   glBindVertexArray( 0 );

   unsigned int woodTexture = LoadTexture( "../Resources/Textures/wood.png" );

   unsigned int depthMapFBO{ 0 };
   glGenFramebuffers( 1, &depthMapFBO );

   constexpr unsigned int SHADOW_WIDTH = 1024;
   constexpr unsigned int SHADOW_HEIGHT = 1024;

   unsigned int depthMap{ 0 };
   glGenTextures( 1, &depthMap );
   glBindTexture( GL_TEXTURE_2D, depthMap );
   glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, nullptr );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
   float borderColor[ ]{ 1.0f, 1.0f, 1.0f, 1.0f };
   glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

   glBindFramebuffer( GL_FRAMEBUFFER, depthMapFBO );
   glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0 );
   glDrawBuffer( GL_NONE );
   glReadBuffer( GL_NONE );
   glBindFramebuffer( GL_FRAMEBUFFER, 0 );

   debugDepthQuad.Use( );
   // Bind DepthMap to GL_TEXTURE0
   debugDepthQuad.SetInt( "depthMap", 0 );

   shader.Use( );
   // Bind Diffuse Texture to GL_TEXTURE0
   // Bind Shadow Map to GL_TEXTURE1
   shader.SetInt( "diffuseTexture", 0 );
   shader.SetInt( "shadowMap", 1 );

   glm::vec3 lightPos( -2.0f, 4.0f, -1.0f );

#pragma endregion

   while ( !glfwWindowShouldClose( window ) )
   {
      float currentFrame = glfwGetTime( );
      deltaTime = currentFrame - lastTime;
      lastTime = currentFrame;
      ++frameCount;
      elasedTime += deltaTime;
      if ( elasedTime >= UnitTime )
      {
         printf( "FPS: %u\n", frameCount );
         frameCount = 0;
         elasedTime = 0.0f;
      }

      processInput( window );

#pragma region Render
      glClearColor( 0.1f, 0.1f, 0.1f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      glm::mat4 lightProjection, lightView;
      glm::mat4 lightSpaceMatrix;
      float near_plane = 1.0f;
      float far_plane = 7.5f;
      lightProjection = glm::ortho( -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane );
      lightView = glm::lookAt( lightPos, glm::vec3( 0.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
      lightSpaceMatrix = lightProjection * lightView;
      simpleDepthShader.Use( );
      simpleDepthShader.SetMat4f( "lightSpaceMatrix", lightSpaceMatrix );

      glCullFace( GL_FRONT );
      glViewport( 0, 0, SHADOW_WIDTH, SHADOW_HEIGHT );
      glBindFramebuffer( GL_FRAMEBUFFER, depthMapFBO );
      glClear( GL_DEPTH_BUFFER_BIT );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, woodTexture );
      renderScene( simpleDepthShader );
      // to default fbo
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );
      glCullFace( GL_BACK );

      glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      shader.Use( );
      glm::mat4 projection = glm::perspective( glm::radians( camera.Zoom ), ( float ) SCREEN_WIDTH / ( float ) SCREEN_HEIGHT, 0.1f, 100.0f );
      glm::mat4 view = camera.GetViewMatrix( );
      shader.SetMat4f( "projection", projection );
      shader.SetMat4f( "view", view );

      shader.SetVec3f( "viewPos", camera.Position );
      shader.SetVec3f( "lightPos", lightPos );
      shader.SetMat4f( "lightSpaceMatrix", lightSpaceMatrix );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, woodTexture );
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, depthMap );
      renderScene( shader );

      debugDepthQuad.Use( );
      debugDepthQuad.SetFloat( "near_plane", near_plane );
      debugDepthQuad.SetFloat( "far_plane", far_plane );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, depthMap );
      //renderQuad( );
#pragma endregion

      glfwSwapBuffers( window );
      glfwPollEvents( );
   }

   glDeleteVertexArrays( 1, &planeVAO );
   glDeleteBuffers( 1, &planeVBO );

   glfwTerminate( );
   return 0;
}

void processInput( GLFWwindow *window )
{
   if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
      glfwSetWindowShouldClose( window, true );

   if ( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
      camera.ProcessKeyboard( FORWARD, deltaTime );
   if ( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
      camera.ProcessKeyboard( BACKWARD, deltaTime );
   if ( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
      camera.ProcessKeyboard( LEFT, deltaTime );
   if ( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
      camera.ProcessKeyboard( RIGHT, deltaTime );
}

void framebuffer_size_callback( GLFWwindow* window, int width, int height )
{
   glViewport( 0, 0, width, height );
}

void mouse_callback( GLFWwindow* window, double xpos, double ypos )
{
   if ( firstMouse )
   {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
   }

   float xoffset = xpos - lastX;
   float yoffset = lastY - ypos;

   lastX = xpos;
   lastY = ypos;

   camera.ProcessMouseMovement( xoffset, yoffset );
}

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
   camera.ProcessMouseScroll( yoffset );
}

unsigned int LoadTexture( const std::string& path )
{
   unsigned int texture;
   glGenTextures( 1, &texture );
   int imageWidth, imageHeight;
   int imageChannels;
   stbi_set_flip_vertically_on_load( true );
   unsigned char* imageData = stbi_load( path.c_str( ), &imageWidth, &imageHeight, &imageChannels, 0 );
   stbi_set_flip_vertically_on_load( false );
   if ( imageData != nullptr )
   {
      GLenum format;
      switch ( imageChannels )
      {
      case 1:
         format = GL_RED;
         break;

      case 3:
         format = GL_RGB;
         break;

      case 4:
         format = GL_RGBA;
         break;
      }

      glBindTexture( GL_TEXTURE_2D, texture );
      glTexImage2D( GL_TEXTURE_2D, 0, format, imageWidth, imageHeight, 0, format, GL_UNSIGNED_BYTE, imageData );
      glGenerateMipmap( GL_TEXTURE_2D );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT ); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   }
   else
   {
      std::cout << "Failed to load " << path << std::endl;
   }
   stbi_image_free( imageData );

   return texture;
}

unsigned int LoadCubeMap( const std::vector<std::string>& faces )
{
   unsigned int textureID;
   glGenTextures( 1, &textureID );
   glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );

   int width;
   int height;
   int nrChannels;
   for ( unsigned int idx = 0; idx < faces.size( ); ++idx )
   {
      unsigned char* data = stbi_load( faces[ idx ].c_str( ), &width, &height, &nrChannels, 0 );
      if ( data != nullptr )
      {
         glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx,
                       0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
      }
      else
      {
         std::cout << "Faeild to load cubemap texture : " << faces[ idx ] << std::endl;
      }
      stbi_image_free( data );
   }

   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
   glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   return textureID;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad( )
{
   if ( quadVAO == 0 )
   {
      float quadVertices[ ] = {
         // positions        // texture Coords
         -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
      };
      // setup plane VAO
      glGenVertexArrays( 1, &quadVAO );
      glGenBuffers( 1, &quadVBO );
      glBindVertexArray( quadVAO );
      glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
      glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), &quadVertices, GL_STATIC_DRAW );
      glEnableVertexAttribArray( 0 );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* ) 0 );
      glEnableVertexAttribArray( 1 );
      glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* ) ( 3 * sizeof( float ) ) );
   }
   glBindVertexArray( quadVAO );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   glBindVertexArray( 0 );
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube( )
{
   // initialize (if necessary)
   if ( cubeVAO == 0 )
   {
      float vertices[ ] = {
         // back face
         -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
         -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
         -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                                                               // front face
                                                               -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                                                               1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                                                               1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                                                               1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                                                               -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                                                               -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                                                                                                                     // left face
                                                                                                                     -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                                                                                                                     -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                                                                                                                     -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                                                                                                                     -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                                                                                                                     -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                                                                                                                     -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                                                                                                                                                                           // right face
                                                                                                                                                                           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                                                                                                                                                                           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                                                                                                                                                                           1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
                                                                                                                                                                           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                                                                                                                                                                           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                                                                                                                                                                           1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
                                                                                                                                                                                                                                // bottom face
                                                                                                                                                                                                                                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                                                                                                                                                                                                                                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                                                                                                                                                                                                                                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                                                                                                                                                                                                                                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                                                                                                                                                                                                                                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                                                                                                                                                                                                                                                                                      // top face
                                                                                                                                                                                                                                                                                      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                                                                                                                                                                                                                                                                                      1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                                                                      1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
                                                                                                                                                                                                                                                                                      1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                                                                                                                                                                                                                                                                                      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                                                                                                                                                                                                                                                                                      -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
      };
      glGenVertexArrays( 1, &cubeVAO );
      glGenBuffers( 1, &cubeVBO );
      // fill buffer
      glBindBuffer( GL_ARRAY_BUFFER, cubeVBO );
      glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
      // link vertex attributes
      glBindVertexArray( cubeVAO );
      glEnableVertexAttribArray( 0 );
      glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) 0 );
      glEnableVertexAttribArray( 1 );
      glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) ( 3 * sizeof( float ) ) );
      glEnableVertexAttribArray( 2 );
      glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* ) ( 6 * sizeof( float ) ) );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );
      glBindVertexArray( 0 );
   }
   // render Cube
   glBindVertexArray( cubeVAO );
   glDrawArrays( GL_TRIANGLES, 0, 36 );
   glBindVertexArray( 0 );
}

void renderScene( const Shader& shader )
{
   glm::mat4 model;
   shader.SetMat4f( "model", model );
   glBindVertexArray( planeVAO );
   glDrawArrays( GL_TRIANGLES, 0, 6 );

   model = glm::mat4( );
   model = glm::translate( model, glm::vec3( 0.0f, 1.5f, 0.0f ) );
   model = glm::scale( model, glm::vec3( 0.5f ) );
   shader.SetMat4f( "model", model );
   renderCube( );
   model = glm::mat4( );
   model = glm::translate( model, glm::vec3( glm::cos(glm::radians(lastTime * 100.0f))*2.0f, 0.0f, glm::sin( glm::radians( lastTime  * 100.0f ) )*1.0f ) );
   model = glm::scale( model, glm::vec3( 0.5f ) );
   shader.SetMat4f( "model", model );
   renderCube( );
   model = glm::mat4( );
   model = glm::translate( model, glm::vec3( -1.0f, 0.0f, 2.0f ) );
   model = glm::rotate( model, glm::radians( 60.0f ), glm::normalize( glm::vec3( 1.0f, 0.0f, 1.0f ) ) );
   model = glm::scale( model, glm::vec3( 0.25f ) );
   shader.SetMat4f( "model", model );
   renderCube( );
}