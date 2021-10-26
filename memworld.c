#include "glad.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...) do {} while (0)
#endif

#define WORLD_HEIGHT 16
#define WORLD_WIDTH 25
#define WORLD_DEPTH 40

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define FIELD_OF_VIEW (M_PI / 2)
#define FOCAL_LENGTH (WINDOW_WIDTH / (2 * tan(FIELD_OF_VIEW / 2)))
#define MAX_ALTITUDE (7 * M_PI / 16)

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

// void getXYfromZ(double azi_pix, double alt_pix, int z, int * x, int * y) {

// }

void render_world(const uint32_t world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH], 
        uint32_t buffer[WINDOW_HEIGHT][WINDOW_WIDTH], camera cam) {

    DEBUG_PRINTF("Rendering from (%d, %d, %d), azimuth %.2lf, altitude %.2lf\n", cam.x, cam.y, cam.z, cam.azimuth, cam.altitude);
    DEBUG_PRINTF("-hfov %.2lf, vfov %.2lf\n", horizontal_fov, vertical_fov);

    #ifndef DEBUG_ONE_PIXEL
    for(int i_pix = -WINDOW_WIDTH / 2; i_pix < WINDOW_WIDTH / 2; i_pix++) {
        for(int j_pix = -WINDOW_HEIGHT / 2; j_pix < WINDOW_HEIGHT / 2; j_pix++) {
    #else
        int i_pix = WINDOW_WIDTH / 2 - 100;
        int j_pix = WINDOW_HEIGHT / 2 - 1;
    #endif
            DEBUG_PRINTF("-pixel (%d, %d)\n", i_pix, j_pix);
            double azi_pix = cam.azimuth + atan2(i_pix, FOCAL_LENGTH);
            double alt_pix = cam.altitude + atan2(j_pix, sqrt(i_pix*i_pix + FOCAL_LENGTH*FOCAL_LENGTH));
 
            DEBUG_PRINTF("--alt_pix %.4lf, azi_pix %.4lf\n", alt_pix, azi_pix);

            double ddz = 1;
            double ddx = fabs(tan(azi_pix) * ddz);
            double ddy = fabs(tan(alt_pix) * sqrt(ddx*ddx + ddz*ddz));

            int rendered = 0;

            int dz, dx, dy, prev_dz = 0, prev_dx = 0, prev_dy = 0;
            for(int i = 1; i <= MAX_DRAW_DISTANCE; i++) {
                // int ddz, ddx, ddy;
                // //Try z first
                // ddz = azi_pix > -M_PI / 2 && azi_pix < M_PI / 2 ? dz + 1 : dz - 1;
                // ddx = roundl(tan(azi_pix) * ddz);
                // ddy = roundl(tan(alt_pix) * sqrt(ddx*ddx + ddz*ddz));

                // if(labs(ddy - dy) > 1) {
                //     //Try y second
                //     ddy = alt_pix < 0 ? dy - 1 : dy + 1;
                //     ddx = roundl(sqrt(pow(tan(alt_pix), 2) * pow(tan(azi_pix), 2) / (ddy*ddy*(pow(tan(azi_pix), 2) + 1))));
                //     ddz = roundl(sqrt(pow(tan(alt_pix), 2) / (ddy*ddy) - ddx*ddx));
                // }

                // if(labs(ddx - dx) > 1 || labs(ddz - dz) > 1) {
                //     //Try x third
                //     ddx = azi_pix < 0 ? dx - 1 : dx + 1;
                //     ddz = roundl(ddx / tan(azi_pix));
                //     ddy = roundl(tan(alt_pix) * sqrt(ddx*ddx + ddz*ddz));
                // }

                // dz = ddz;
                // dy = ddy;
                // dx = ddx;


                // } else if(ddy >= ddx) {
                //     dy = alt_pix < 0 ? -i : i;
                //     dx = roundl(sqrt(pow(tan(alt_pix), 2) * pow(tan(azi_pix), 2) / (dy*dy*(pow(tan(azi_pix), 2) + 1))));
                //     dz = roundl(sqrt(pow(tan(alt_pix), 2) / (dy*dy) - dx*dx));
                // } else {
                //     dx = azi_pix < 0 ? -i : i;
                //     dz = roundl(dx / tan(azi_pix));
                //     dy = roundl(tan(alt_pix) * sqrt(dx*dx + dz*dz));
                // }

                if(ddz >= ddx && ddz >= ddy) {
                    dz = azi_pix > -M_PI / 2 && azi_pix < M_PI / 2 ? prev_dz + 1 : prev_dz - 1;
                    dx = roundl(tan(azi_pix) * dz);
                    dy = roundl(tan(alt_pix) * sqrt(dx*dx + dz*dz));
                    if(labs(dx - prev_dx) > 1) {
                        dx = azi_pix < 0 ? prev_dx - 1 : prev_dx + 1;
                        dz = roundl(dx / tan(azi_pix));
                        dy = roundl(tan(alt_pix) * sqrt(dx*dx + dz*dz));
                    }
                    if(labs(dy - prev_dy) > 1) {
                        dy = alt_pix < 0 ? prev_dy - 1 : prev_dy + 1;
                        dx = roundl(sqrt(pow(tan(alt_pix), 2) * pow(tan(azi_pix), 2) / (dy*dy*(pow(tan(azi_pix), 2) + 1))));
                        dz = roundl(sqrt(pow(tan(alt_pix), 2) / (dy*dy) - dx*dx));
                    }
                } else if(ddy >= ddx) {
                    dy = alt_pix < 0 ? prev_dy - 1 : prev_dy + 1;
                    dx = roundl(sqrt(pow(tan(alt_pix), 2) * pow(tan(azi_pix), 2) / (dy*dy*(pow(tan(azi_pix), 2) + 1))));
                    dz = roundl(sqrt(pow(tan(alt_pix), 2) / (dy*dy) - dx*dx));
                    if(labs(dz - prev_dz) > 1) {
                        dz = azi_pix > -M_PI / 2 && azi_pix < M_PI / 2 ? prev_dz + 1 : prev_dz - 1;
                        dx = roundl(tan(azi_pix) * dz);
                        dy = roundl(tan(alt_pix) * sqrt(dx*dx + dz*dz));                        
                    }
                    if(labs(dx - prev_dx) > 1) {
                        dx = azi_pix < 0 ? prev_dx - 1 : prev_dx + 1;
                        dz = roundl(dx / tan(azi_pix));
                        dy = roundl(tan(alt_pix) * sqrt(dx*dx + dz*dz));
                    }
                } else {
                    dx = azi_pix < 0 ? prev_dx - 1 : prev_dx + 1;
                    dz = roundl(dx / tan(azi_pix));
                    dy = roundl(tan(alt_pix) * sqrt(dx*dx + dz*dz));
                    if(labs(dz - prev_dz) > 1) {
                        dz = azi_pix > -M_PI / 2 && azi_pix < M_PI / 2 ? prev_dz + 1 : prev_dz - 1;
                        dx = roundl(tan(azi_pix) * dz);
                        dy = roundl(tan(alt_pix) * sqrt(dx*dx + dz*dz));                        
                    }
                    if(labs(dy - prev_dy) > 1) {
                        dy = alt_pix < 0 ? prev_dy - 1 : prev_dy + 1;
                        dx = roundl(sqrt(pow(tan(alt_pix), 2) * pow(tan(azi_pix), 2) / (dy*dy*(pow(tan(azi_pix), 2) + 1))));
                        dz = roundl(sqrt(pow(tan(alt_pix), 2) / (dy*dy) - dx*dx));
                    }
                }


                prev_dx = dx;
                prev_dy = dy;
                prev_dz = dz;

                DEBUG_PRINTF("---Incrementing y\n");
                DEBUG_PRINTF("---(%d, %d, %d)\n", cam.x + dx, cam.y + dy, cam.z + dz);
                DEBUG_PRINTF("---value is 0x%08X\n", world[cam.x + dx][cam.y + dy][cam.z + dz]);

                if(world[cam.x + dx][cam.y + dy][cam.z + dz] != 0) {
                    DEBUG_PRINTF("---pixel assigned\n");
                    buffer[j_pix + WINDOW_HEIGHT / 2][i_pix + WINDOW_WIDTH / 2] = world[cam.x + dx][cam.y + dy][cam.z + dz];
                    rendered = 1;
                    break;
                }
            }

            if(!rendered) {
                buffer[j_pix + WINDOW_HEIGHT / 2][i_pix + WINDOW_WIDTH / 2] = MAX_DRAW_COLOR;
                DEBUG_PRINTF("---default color\n");
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

void process_input(GLFWwindow *window, camera * cam)
{
    const double camera_speed = 0.1; // adjust accordingly
    const double rotate_speed = 0.1;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if(cam->altitude < MAX_ALTITUDE) {
            cam->altitude += rotate_speed;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if(cam->altitude > -MAX_ALTITUDE) {
            cam->altitude -= rotate_speed;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam->azimuth -= rotate_speed;
        if(cam->azimuth <= -M_PI) {
            cam->azimuth += 2*M_PI;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam->azimuth += rotate_speed;
        if(cam->azimuth > M_PI) {
            cam->azimuth -= 2*M_PI;
        }
    }
}

int main()
{
    uint32_t world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];
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

    uint32_t pixels[WINDOW_HEIGHT][WINDOW_WIDTH];
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

    int i = 0;

    camera cam = {WORLD_WIDTH / 2, 0, WORLD_HEIGHT / 2, 0, WORLD_DEPTH / 2, 0, 0, 0};

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 0.5f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
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
        process_input(window, &cam);

        render_world(world, pixels, cam);

        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        0,
                        0,
                        WINDOW_WIDTH,
                        WINDOW_HEIGHT,
                        GL_RGBA,
                        GL_UNSIGNED_INT_8_8_8_8,
                        (void *)pixels);
        //glGenerateMipmap(GL_TEXTURE_2D);
    }

    glfwTerminate();
    return 0;
}
