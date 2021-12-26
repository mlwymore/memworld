#include "glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...) do {} while (0)
#endif

#define VOXEL_DENSITY 1

#define WORLD_HEIGHT (16 * VOXEL_DENSITY)
#define WORLD_WIDTH (25 * VOXEL_DENSITY)
#define WORLD_DEPTH (40 * VOXEL_DENSITY)

uint32_t world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 480

uint32_t pixels[WINDOW_HEIGHT][WINDOW_WIDTH];

const double FIELD_OF_VIEW = (M_PI / 2);
double FOCAL_LENGTH;
const double MAX_ALTITUDE = (7 * M_PI / 16);

#define MAX_DRAW_DISTANCE 100
#define MAX_DRAW_COLOR 0x777777FF

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec2 aPos;\n"
                                 "layout (location = 1) in vec2 aTexCoords;\n"
                                 "out vec2 TexCoords;\n"
                                 "void main(){\n"
                                 "TexCoords = aTexCoords;\n"
                                 "gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
                                 "}\0";

const char *fragShaderSource = "#version 330 core\n"
                               "out vec4 FragColor;\n"
                               "in vec2 TexCoords;\n"
                               "uniform sampler2D screenTexture;\n"
                               "void main() {\n"
                               //"FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                               //"vec3 col = texture(screenTexture, TexCoords).rgb;\n"
                               //"FragColor = vec4(col, 1.0);\n"
                               "FragColor = texture(screenTexture, TexCoords);\n"
                               "}\0";
                            

typedef struct camera_t {
    int x;
    double x_part;
    int y;
    double y_part;
    int z;
    double z_part;
    double azimuth;
    double altitude;
} camera;

camera cam = {WORLD_WIDTH / 2, 0, WORLD_HEIGHT / 2, 0, WORLD_DEPTH / 2, 0, 0, 0};


void render_world(const uint32_t world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH], 
        uint32_t buffer[WINDOW_HEIGHT][WINDOW_WIDTH]) {

    //DEBUG_PRINTF("Rendering from (%d, %d, %d), azimuth %.2lf, altitude %.2lf\n", cam.x, cam.y, cam.z, cam.azimuth, cam.altitude);
    //DEBUG_PRINTF("-hfov %.2lf, vfov %.2lf\n", horizontal_fov, vertical_fov);

    #ifndef DEBUG_ONE_PIXEL
    for(int i_pix = -WINDOW_WIDTH / 2 * VOXEL_DENSITY; i_pix < WINDOW_WIDTH / 2 * VOXEL_DENSITY; i_pix += VOXEL_DENSITY) {
        for(int j_pix = -WINDOW_HEIGHT / 2 * VOXEL_DENSITY; j_pix < WINDOW_HEIGHT / 2 * VOXEL_DENSITY; j_pix += VOXEL_DENSITY) {
    #else
        int i_pix = WINDOW_WIDTH / 2 - 100;
        int j_pix = WINDOW_HEIGHT / 2 - 1;
    #endif
            //DEBUG_PRINTF("-pixel (%d, %d)\n", i_pix, j_pix);
            double azi_pix = cam.azimuth + atan(i_pix / FOCAL_LENGTH);
            double y1 = FOCAL_LENGTH * sin(cam.altitude);
            double d = sqrt(FOCAL_LENGTH*FOCAL_LENGTH + i_pix*i_pix);
            double beta = acos(y1 / FOCAL_LENGTH);
            double y2 = j_pix * sin(beta);
            double alt_pix = asin((y1 + y2) / sqrt(d*d + j_pix*j_pix));

            if(alt_pix > M_PI / 2) {
                azi_pix += M_PI;
                alt_pix = M_PI - alt_pix;
            } else if(alt_pix <= -M_PI / 2) {
                azi_pix += M_PI;
                alt_pix = -M_PI - alt_pix;
            }
            if(azi_pix > M_PI) {
                azi_pix -= 2 * M_PI;
            } else if(azi_pix <= -M_PI) {
                azi_pix += 2 * M_PI;
            }
 
            //DEBUG_PRINTF("--alt_pix %.4lf, azi_pix %.4lf\n", alt_pix, azi_pix);

            double uy = sin(alt_pix);

            double theta = azi_pix;
            if(azi_pix < 0 && azi_pix >= -M_PI / 2) {
                theta = -azi_pix;
            } else if(azi_pix < -M_PI / 2) {
                theta = azi_pix + M_PI;
            } else if(azi_pix > M_PI / 2) {
                theta = M_PI - azi_pix;
            }

            double h = sqrt(1 - uy*uy);

            double ux = h * sin(theta);
            if(azi_pix < 0) {
                ux = -ux;
            }

            double uz = h * cos(theta);
            if(azi_pix < -M_PI / 2 || azi_pix > M_PI / 2) {
                uz = -uz;
            }

            int rendered = 0;

            double dz, dx, dy;
            for(int i = 1; i <= MAX_DRAW_DISTANCE * VOXEL_DENSITY; i++) {

                dx = ux * i;
                dy = uy * i;
                dz = uz * i;

                //DEBUG_PRINTF("---Incrementing y\n");
                //DEBUG_PRINTF("---(%d, %d, %d)\n", cam.x + dx, cam.y + dy, cam.z + dz);
                //DEBUG_PRINTF("---value is 0x%08X\n", world[cam.x + dx][cam.y + dy][cam.z + dz]);

                uint32_t color = world[lround(cam.x + cam.x_part + dx)][lround(cam.y + cam.y_part + dy)][lround(cam.z + cam.z_part + dz)];
                if(color != 0) {
                    //DEBUG_PRINTF("---pixel assigned\n");
                    buffer[j_pix / VOXEL_DENSITY + WINDOW_HEIGHT / 2][i_pix / VOXEL_DENSITY + WINDOW_WIDTH / 2] = color;
                    rendered = 1;
                    break;
                }
            }

            if(!rendered) {
                buffer[j_pix / VOXEL_DENSITY + WINDOW_HEIGHT / 2][i_pix / VOXEL_DENSITY + WINDOW_WIDTH / 2] = MAX_DRAW_COLOR;
                //DEBUG_PRINTF("---default color\n");
            }
            #ifdef DEBUG
                uint32_t target_color = 0x000000FF;
                //if(buffer[j_pix + WINDOW_HEIGHT / 2][i_pix + WINDOW_WIDTH / 2] == target_color) exit(0);
            #endif
    #ifndef DEBUG_ONE_PIXEL
        }
    }
    #else
        exit(0);
    #endif
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window, const uint32_t world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH])
{
    const double camera_speed = 1 * VOXEL_DENSITY; // adjust accordingly
    double dx, dz;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        dx = sin(cam.azimuth);
        dz = cos(cam.azimuth);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        
    }

    double newX = cam.x + cam.x_part + dx * camera_speed;
    double newZ = cam.z + cam.z_part + dz * camera_speed;
    if(world[(int)newX][cam.y][(int)newZ] == 0) {
        cam.x = newX;
        cam.z = newZ;
        cam.x_part = newX - cam.x;
        if(cam.x_part < 0) {
            cam.x_part += 1;
        }
        cam.z_part = newZ - cam.z;
        if(cam.z_part < 0) {
            cam.z_part += 1;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = WINDOW_WIDTH / 2;
    static float lastY = WINDOW_HEIGHT / 2;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.005f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    cam.altitude += yoffset;
    cam.azimuth += xoffset;

    if(cam.altitude > MAX_ALTITUDE) {
        cam.altitude = MAX_ALTITUDE;
    } else if (cam.altitude < -MAX_ALTITUDE) {
        cam.altitude = -MAX_ALTITUDE;
    }

    if(cam.azimuth <= -M_PI) {
        cam.azimuth += 2*M_PI;
    } else if(cam.azimuth > M_PI) {
        cam.azimuth -= 2*M_PI;
    }
}

int main()
{
    FOCAL_LENGTH = (WINDOW_WIDTH * VOXEL_DENSITY / (2 * tan(FIELD_OF_VIEW / 2)));

    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < WORLD_DEPTH; z++) {
                if (x == 0) {
                    world[x][y][z] = 0xFF0000FF;
                } else if(x == WORLD_WIDTH - 1) {
                    world[x][y][z] = 0x00FF00FF;
                } else if(z == 0) {
                    world[x][y][z] = 0x0000FFFF;
                } else if (z == WORLD_DEPTH - 1) {
                    world[x][y][z] = 0x770077FF;
                } else if (y == 0) {
                    world[x][y][z] = 0xFFFFFFFF;
                } else if (y == WORLD_HEIGHT - 1) {
                    world[x][y][z] = 0x000000FF;
                } else {
                    world[x][y][z] = 0;
                }
            }
        }
    }

    for (int j = 0; j < WINDOW_HEIGHT; j++)
    {
        for (int i = 0; i < WINDOW_WIDTH; i++)
        {
            if (i == j || i == WINDOW_HEIGHT - j)
            {
                pixels[j][i] = 0xFF0000FF;
            }
            else
            {
                pixels[j][i] = 0x000000FF;
            }
        }
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "memworld", NULL, NULL);
    if (window == NULL)
    {
        DEBUG_PRINTF("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback); 

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        DEBUG_PRINTF("Failed to initialize GLAD\n");
        return -1;
    }

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        DEBUG_PRINTF("a %x\n", err);
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    unsigned int vertexShader, fragShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glShaderSource(fragShader, 1, &fragShaderSource, NULL);
    glCompileShader(fragShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);

    float quadVertices[] = {// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
                            // positions   // texCoords
                            -1.0f, 1.0f, 0.0f, 1.0f,
                            -1.0f, -1.0f, 0.0f, 0.0f,
                            1.0f, -1.0f, 1.0f, 0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f,
                            1.0f, -1.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 1.0f, 1.0f};

    // screen quad VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 WINDOW_WIDTH,
                 WINDOW_HEIGHT,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_INT_8_8_8_8,
                 (void *)pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(shaderProgram);
    //glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), texture);
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        DEBUG_PRINTF("b %x\n", err);
    }

    clock_t t = clock();
    clock_t prev_t = t;

    while (!glfwWindowShouldClose(window))
    {
        //glClearColor(1.0f, 0.5f, 1.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            DEBUG_PRINTF("c %x\n", err);
        }
        // pixels[i][10] = 0x00FF00FF;
        // i++;
        // if (i >= WINDOW_HEIGHT)
        // {
        //     break;
        // }
        process_input(window, world);

        render_world(world, pixels);

        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        0,
                        0,
                        WINDOW_WIDTH,
                        WINDOW_HEIGHT,
                        GL_RGBA,
                        GL_UNSIGNED_INT_8_8_8_8,
                        (void *)pixels);
        
        t = clock();
        printf("%lf\n", CLOCKS_PER_SEC / (double)(t - prev_t));
        prev_t = t;
    }

    glfwTerminate();
    return 0;
}
