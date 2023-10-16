// GLEW
//#define GLEW_STATIC
#include <GL/glew.h>
#include <stb_image.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <shader.h>
#include <arcball.h>
#include <Model.h>
#include <keyframe.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <text.h>
#include <plane.h>
#include <cube.h>

// Std. Includes
#include <string>
#include <ctime>

static const int START_TIME = 10;


static const int SUSTAIN_TIME = 20;

bool record = false;
bool gameStart = false;

int len = 4;
int tenten = 10;
int state = 0;
int rotState = 0;
clock_t Cstart, Cend;
bool GAMERUN = false;
float deltaT = 1.0f / 200.0f;
int accT = 0;
float AccStore[SUSTAIN_TIME][2];
float AccDirStore[SUSTAIN_TIME][2];


enum WriteLR { NO, L,R };
enum turnLR{STAY,TL,TR};
WriteLR flag = NO;
turnLR turnFlag = STAY;

// Globals
unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 960;
GLFWwindow *mainWindow = NULL;
glm::mat4 projection;

// for arcball
float arcballSpeed = 0.05f;
static Arcball camArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true );
static Arcball modelArcBall(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
bool arcballCamRot = true;

// for camera
glm::vec3 cameraOrigPos(0.0f, -5.0f, 2.0f);
glm::vec3 cameraPos;
glm::vec3 modelPan(0.0f, 0.0f, 0.0f);

// Function prototypes
unsigned int loadTexture(const char* texFileName);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action , int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double x, double y);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void Euler();
void initKeyframes();
void updateAnimData(float timeT);

GLFWwindow *glAllInit();
void render();

//Shader
Shader* Duckshader;
Shader* textshader;
Shader* SeaShader;

// For model
Model* Duck;
Model* Balloon;


// KeyFraming
float xTrans, yTrans, zTrans;           // current translation factors
float xAngle, yAngle, zAngle;           // current rotation factors
KeyFraming xTKF(4), yTKF(4), zTKF(4);   // translation keyframes
KeyFraming xRKF(4), yRKF(4), zRKF(4);   // rotation keyframes

glm::vec2 Position(0.0f);
glm::vec2 Velocity(0.0f);
glm::vec2 Accel(0.0f);

glm::vec2 angleVelocity(0.0f);
glm::vec2 angleAccel(0.0f);

glm::vec2 AccelDir(glm::radians(0.0f));

Text* timer;
Plane* Sea;
static unsigned int SeaTex, block;

Cube* left_cube;
Cube* right_cube;

bool RaceFinish = false;
string myrecord;

int main( )
{
    mainWindow = glAllInit();

    for (int i = 0; i < 15; i++) {
        AccStore[i][0] = 0;
        AccStore[i][1] = 0;
        AccDirStore[i][0] = 0;
        AccDirStore[i][1] = 0;
    }

    // Projection initialization
    projection = glm::perspective(glm::radians(45.0f),
        (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    //DUCK
    Duckshader = new Shader( "res/shaders/modelLoading.vs", "res/shaders/modelLoading.frag" );
    Duck = new Model((GLchar*)"res/models/Duck/Duck.obj");
    Balloon = new Model((GLchar*)"res/models/Balloon/Balloon.obj");
    Duckshader->use();
    Duckshader->setMat4("projection", projection);

    // Timer
    textshader = new Shader("text.vs", "text.frag");
    timer = new Text((char*)"fonts/arial.ttf", textshader, SCR_WIDTH, SCR_HEIGHT);

    //Sea
    SeaShader = new Shader("ground.vs", "ground.fs");
    SeaShader->use();
    SeaShader->setMat4("projection", projection);
    Sea = new Plane(0.0f, 0.0f, 0.0f, 10000.0f);
    Sea->texCoords[0] = 0;
    Sea->texCoords[1] = 100;
    Sea->texCoords[2] = 0;
    Sea->texCoords[3] = 0;
    Sea->texCoords[4] = 100;
    Sea->texCoords[5] = 0;
    Sea->texCoords[6] = 100;
    Sea->texCoords[7] = 100;
    Sea->initBuffers();
    SeaTex = loadTexture("sea.png");
    SeaShader->setInt("SeaTex", 0);

    //Light
    //light
    // directional light
    
    SeaShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    SeaShader->setVec3("dirLight.ambient", 0.7f, 0.7f, 0.7f);
    SeaShader->setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.3f);
    SeaShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    Duckshader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    Duckshader->setVec3("dirLight.ambient", 0.7f, 0.7f, 0.7f);
    Duckshader->setVec3("dirLight.diffuse", 0.3f, 0.3f, 0.3f);
    Duckshader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    

    cameraPos = glm::vec3(-100.0f, -100.0f, 30.0f);
    initKeyframes();

    left_cube = new Cube(-5, 0, 0, 0.5f, 0.5f, 1.0f);
    right_cube = new Cube(5, 0, 0, 0.5f, 0.5f, 1.0f);

    block = loadTexture("block.jpg");
    SeaShader->setInt("Block", 1);
    // Game loop
    while( !glfwWindowShouldClose( mainWindow ) )
    {
        glfwPollEvents( );
        render();

        glfwSwapBuffers(mainWindow);
    }
    
    glfwTerminate( );
    return 0;
}


unsigned int loadTexture(const char* texFileName) {
    unsigned int texture;

    // Create texture ids.
    glGenTextures(1, &texture);

    // All upcomming GL_TEXTURE_2D operations now on "texture" object
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set texture parameters for wrapping.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    // Set texture parameters for filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);   // vertical flip the texture
    unsigned char* image = stbi_load(texFileName, &width, &height, &nrChannels, 0);
    if (!image) {
        printf("texture %s loading error ... \n", texFileName);
    }
    else printf("texture %s loaded\n", texFileName);

    GLenum format;
    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

void render()
{
    //cout<<Velocity[1]<<"\n";
    //cout << accT << "\n";
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // Drawing texts
    //IF FINISH

    //Start Text
    float start = (float)glfwGetTime();
    if (start < START_TIME) {
        timer->RenderText("READY?", 350,650, 3.0f, glm::vec3(0.8, 0.3f, 0.0f));
    }
    else if (start < START_TIME+1) {
        timer->RenderText("3", 565, 650, 3.0f, glm::vec3(0.8, 0.3f, 0.0f));
    }
    else if (start < START_TIME+2) {
        timer->RenderText("2", 565, 650, 3.0f, glm::vec3(0.8, 0.3f, 0.0f));

    }
    else if (start < START_TIME+3) {
        timer->RenderText("1", 565, 650, 3.0f, glm::vec3(0.8, 0.3f, 0.0f));

    }
    else if (start < START_TIME+4) {
        timer->RenderText("START!", 350, 650, 3.0f, glm::vec3(0.8, 0.3f, 0.0f));
        GAMERUN = true;
    }

    //Timer
    float t = (float)glfwGetTime()-(START_TIME+4);
    if (RaceFinish == false) {
        string time = to_string(t);
        myrecord = time;
        if (t >= tenten) {
            len++;
            tenten *= 10;
        }
        if (t > 0) {
            timer->RenderText(time.substr(0, len), 1100.0f - (len - 4) * 18, 920.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        }
        //EULER
        if (GAMERUN) {
            Euler();
        }
    }

    //Draw Duck
    Duckshader->use();
    
    glm::mat4 view;
    //camera
    if (start < START_TIME-3) {
        view = glm::lookAt(cameraPos, glm::vec3(Position, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        view = view * camArcBall.createRotationMatrix();
        Duckshader->setMat4("view", view);
    }
    else if (start < START_TIME) {
        //linear Keyframe
        updateAnimData(start - (START_TIME-3));
        cameraPos.x = xTrans; cameraPos.y = yTrans; cameraPos.z = zTrans;
        view = glm::lookAt(cameraPos, glm::vec3(Position, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        view = view * camArcBall.createRotationMatrix();
        Duckshader->setMat4("view", view);
    }
    else  {
        
        view = glm::lookAt(cameraPos, glm::vec3(Position, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        view = view * camArcBall.createRotationMatrix();
        cameraPos = glm::vec3(Position, 0.0f) + glm::vec3(5.0f * glm::sin(AccelDir[0] + glm::radians(180.0f)), 5.0f * glm::cos(AccelDir[0] + glm::radians(180.0f)), 2.0f);
        Duckshader->setMat4("view", view);
    }
    Duckshader->setVec3("viewPos", cameraPos);

    // Draw the loaded model
    glm::mat4 model(1.0);

    // Rotate model by arcball and panning
    model = glm::translate( model, glm::vec3(Position,0.0f));
    model = glm::rotate(model, -AccelDir[0], glm::vec3(0.0f,0.0f, 1.0f));
    model = glm::rotate(model,glm::radians(180.0f) , glm::vec3(0, 0, 1));
    model = model * modelArcBall.createRotationMatrix();
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    
    Duckshader->setMat4("model", model);
    
    Duck->Draw( Duckshader );

    //Balloon
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0, 0.0, 0.0));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 0, 1));
    model = model * modelArcBall.createRotationMatrix();
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));

    Duckshader->setMat4("model", model);
    Balloon->Draw(Duckshader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(5.0, 0.0, 0.0));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 0, 1));
    model = model * modelArcBall.createRotationMatrix();
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    Duckshader->setMat4("model", model);
    Balloon->Draw(Duckshader);


    //Draw Sea
    SeaShader->use();
    SeaShader->setMat4("view", view);
    SeaShader->setVec3("viewPos", cameraPos);
    model = glm::mat4(1.0);
    //model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    SeaShader->setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, SeaTex);
    SeaShader->setInt("Tex", 0);
    Sea->draw(SeaShader);
    model = glm::rotate(model, glm::radians(180.f), glm::vec3(1.0, 0.0, 0.0));
    SeaShader->setMat4("model", model);
    Sea->draw(SeaShader);

    //blocks
    model = glm::mat4(1.0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, block);
    SeaShader->setInt("Tex", 1);

    //Course
    //straight
    for (int i = 0; i < 20; i++) {
        model = glm::translate(model, glm::vec3(0.0f,10.0f,0.0f));
        SeaShader->setMat4("model", model);
        left_cube->draw(SeaShader);
        right_cube->draw(SeaShader);
    }
    model = glm::mat4(1.0f);
    //curve
    for (int i = 1; i <= 20; i++) {
        model = glm::translate(model, glm::vec3(-100.0f, 200.0f, 0.0f));
        model = glm::rotate(model, glm::radians(9.0f*i), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(100.0f, 0.0f, 0.0f));
        SeaShader->setMat4("model", model);
        left_cube->draw(SeaShader);
        right_cube->draw(SeaShader);
        model = glm::mat4(1.0f);
    }
    model = glm::translate(model, glm::vec3(-200.0, 200.0, 0.0));
    //straight
    for (int i = 0; i < 20; i++) {
        model = glm::translate(model, glm::vec3(0.0f, -10.0f, 0.0f));
        SeaShader->setMat4("model", model);
        left_cube->draw(SeaShader);
        right_cube->draw(SeaShader);
    }
    model = glm::mat4(1.0f);
    //curve
    for (int i = 1; i <= 20; i++) {
        model = glm::translate(model, glm::vec3(-100.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(9.0f * i), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::translate(model, glm::vec3(-100.0f, 0.0f, 0.0f));
        SeaShader->setMat4("model", model);
        left_cube->draw(SeaShader);
        right_cube->draw(SeaShader);
        model = glm::mat4(1.0f);
    }

    //FINISH Message
    if (RaceFinish) {
        timer->RenderText("FINISH!!", 350, 650, 3.0f, glm::vec3(0.8, 0.3f, 0.0f));
        timer->RenderText(myrecord.substr(0, len), 1100.0f - (len - 4) * 18, 920.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
    }

    //Text for Key inputs
    if (flag == L) {
        timer->RenderText("L", 30, 30, 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        if (accT % 50 == 1) flag = NO;
    }
    else if (flag == R) {
        timer->RenderText("R", 1100, 30, 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        if (accT % 50 == 1) flag = NO;
    }
    if (turnFlag == TL) {
        timer->RenderText("Turn L", 130, 30, 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        if (accT % 50 == 1) turnFlag = STAY;
    }
    else if (turnFlag == TR) {
        timer->RenderText("Turn R", 800, 30, 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        if (accT % 50 == 1) turnFlag = STAY;
    }
}

void Euler() {
    float prevP, nowP;
    prevP = Position[1];
    for (int i = 0; i < 2; i++) {

        Position[i] = Position[i] + (Velocity[i] * deltaT);
        Velocity[i] = Velocity[i] + Accel[i] * deltaT;
        Accel[i] = 0;

        AccStore[(accT + 1) % SUSTAIN_TIME][i] = 0;
        for (int j = 0; j < SUSTAIN_TIME; j++) {
            Accel[i] += AccStore[j][i];
        }
        Accel[i] -= 0.2 * Velocity[i];

        AccelDir[i] = AccelDir[i] + (angleVelocity[i] * deltaT);
        angleVelocity[i] = angleVelocity[i] + angleAccel[i] * deltaT;
        angleAccel[i] = 0;

        AccDirStore[(accT + 1) % SUSTAIN_TIME][i] = 0;
        for (int j = 0; j < SUSTAIN_TIME; j++) {
            angleAccel[i] += AccDirStore[j][i];
        }
        angleAccel[i] -= 0.2 * angleVelocity[i];

    }
    nowP = Position[1];
    if (prevP < 0 && 0 < nowP) {
        RaceFinish = true;
    }
    accT++;
}

GLFWwindow *glAllInit()
{
    // Init GLFW
    glfwInit( );
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow *window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "Model Loading", nullptr, nullptr );
    
    if ( nullptr == window )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate( );
        exit(-1);
    }
    
    glfwMakeContextCurrent( window );
    
    // Set the required callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if ( GLEW_OK != glewInit( ) )
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        exit(-1);
    }
    
    // Define the viewport dimensions
    glViewport( 0, 0, SCR_WIDTH, SCR_HEIGHT );
    
    // OpenGL options
    glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
    glEnable( GL_DEPTH_TEST );
    
    // Set OpenGL options
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    projection = glm::perspective(glm::radians(45.0f),
                                  (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    Duckshader->use();
    Duckshader->setMat4("projection", projection); 
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
        modelArcBall.init(SCR_WIDTH, SCR_HEIGHT, arcballSpeed, true, true);
        cameraPos = cameraOrigPos;

    }
    if (state == 0) {
        if (key == GLFW_KEY_A && action == GLFW_PRESS) {
            state = 1;
            flag = L;
        }
    }
    else if (state == 1) {
        if (key == GLFW_KEY_L && action == GLFW_PRESS) {
            AccStore[accT % SUSTAIN_TIME][0] = 20 * glm::sin(AccelDir[0]);
            AccStore[accT % SUSTAIN_TIME][1] = 20 * glm::cos(AccelDir[0]);
            state = 0;
            flag = R;
            //cout << 0.5;
        }
        if (key == GLFW_KEY_A && action == GLFW_PRESS) {
            state = 0;
            flag = NO;
        }
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        turnFlag = TL;
        AccDirStore[accT % SUSTAIN_TIME][0] = glm::radians(-18.0f);
        AccDirStore[accT % SUSTAIN_TIME][1] = glm::radians(-18.0f);
    }
    else if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        turnFlag = TR;
        AccDirStore[accT % SUSTAIN_TIME][0] = glm::radians(18.0f);
        AccDirStore[accT % SUSTAIN_TIME][1] = glm::radians(18.0f);
    }
   
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (arcballCamRot)
        camArcBall.mouseButtonCallback( window, button, action, mods );
    else
        modelArcBall.mouseButtonCallback( window, button, action, mods );
}

void cursor_position_callback(GLFWwindow *window, double x, double y) {
    if (arcballCamRot)
        camArcBall.cursorCallback( window, x, y );
    else
        modelArcBall.cursorCallback( window, x, y );
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    cameraPos[2] -= (yoffset * 0.5);
}

void initKeyframes() {

    // x-translation keyframes
    xTKF.setKey(0, 0, -100.0);
    xTKF.setKey(1, 1.0, -66.0);
    xTKF.setKey(2, 2.0, -33.0);
    xTKF.setKey(3, 3.0, 0.0);

    // y-translation keyframes
    yTKF.setKey(0, 0, -100.0);
    yTKF.setKey(1, 1.0, -66.0);
    yTKF.setKey(2, 2.0, -33.0);
    yTKF.setKey(3, 3.0, -5.0);

    // z-translation keyframes
    zTKF.setKey(0, 0, 30.0);
    zTKF.setKey(1, 1.0, 20.0);
    zTKF.setKey(2, 2.0, 10.0);
    zTKF.setKey(3, 3.0, 2.0);
    /*
    // x-rotation keyframes
    xRKF.setKey(0, 0, 0.0);
    xRKF.setKey(1, 1.5, 20.0);
    xRKF.setKey(2, 3.0, 80.0);
    xRKF.setKey(3, 3.0, 0.0);

    // y-rotation keyframes
    yRKF.setKey(0, 0, 0.0);
    yRKF.setKey(1, 1.5, -30.0);
    yRKF.setKey(2, 3.0, 50.0);
    yRKF.setKey(3, 3.0, 0.0);

    // z-rotation keyframes
    zRKF.setKey(0, 0, 0.0);
    zRKF.setKey(1, 1.5, 90.0);
    zRKF.setKey(2, 3.0, 180.0);
    zRKF.setKey(3, 3.0, 200.0);
    */
}

void updateAnimData(float timeT) {

    xTrans = xTKF.getValLinear(timeT);
    yTrans = yTKF.getValLinear(timeT);
    zTrans = zTKF.getValLinear(timeT);
    //xAngle = xRKF.getValLinear(timeT);
    //yAngle = yRKF.getValLinear(timeT);
    //zAngle = zRKF.getValLinear(timeT);
}
