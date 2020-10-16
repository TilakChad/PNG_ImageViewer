#include "glad/include/glad/glad.h"

#include "glfw3/include/GLFW/glfw3.h"
#include "image.h"

float screen_height = 720;
float screen_width = 1280;

[[noreturn]] void error_callback(int code, const char *description)
{
    fputs(description, stderr);
    exit(EXIT_FAILURE);
}

void on_size_change(GLFWwindow *, int, int);
void key_callback(GLFWwindow *, int, int, int, int);

struct Point
{
    float x;
    float y;
};
typedef struct Point Point;

Point normalized_coordinate(Point image_point);

int main(int argc, char **argv)
{
    if (!glfwInit())
    {
        fputs("Error loading glfw library...", stderr);
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "PNG Image Viewer", NULL, NULL);
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    // Load glad interface
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fputs("Error loading GLAD... Set it up properly...", stderr);
        exit(EXIT_FAILURE);
    }

    glfwSetFramebufferSizeCallback(window, on_size_change);

    struct paint_info draw_data = nomain(argc, argv);
    int format = draw_data.image_color_type;
    int height = draw_data.image_height;
    int width = draw_data.image_width;

    float *vertices;
    int stride;
    if (format == 6) // True Color
    {
        vertices = malloc(6 * sizeof(float) * width * height);
        printf("True color...");
        stride = 6;
    }
    else if (format == 2)
    {
        printf("RGB color\n");
        vertices = malloc(5 * sizeof(float) * width * height);
        stride = 5;
    }
    else
    {
    }

    assert(vertices != NULL);
    int writer = 0;
    int reader = 0;
    if (format == 6)
        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                Point draw_point = normalized_coordinate((Point){col, row});
                vertices[writer++] = draw_point.x;
                vertices[writer++] = draw_point.y;
                vertices[writer++] = (float)draw_data.final_data[reader++] / 255.0f;
                vertices[writer++] = (float)draw_data.final_data[reader++] / 255.0f;
                vertices[writer++] = (float)draw_data.final_data[reader++] / 255.0f;
                vertices[writer++] = (float)draw_data.final_data[reader++] / 255.0f;
            }
        }
    else if (format == 2)
    {
        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                Point draw_point = normalized_coordinate((Point){col, row});
                vertices[writer++] = draw_point.x;
                vertices[writer++] = draw_point.y;
                vertices[writer++] = (float)draw_data.final_data[reader++] / 255.0f;
                vertices[writer++] = (float)draw_data.final_data[reader++] / 255.0f;
                vertices[writer++] = (float)draw_data.final_data[reader++] / 255.0f;
            }
        }
    }

    printf("Error type...");
    unsigned int VBO;
    unsigned int VAO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    if(format == 6)
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float) * width * height, vertices, GL_STATIC_DRAW);
    else if (format == 2)
        glBufferData(GL_ARRAY_BUFFER, 5 * sizeof(float) * width * height, vertices, GL_STATIC_DRAW);
    const char *vertexShaderSource[] =
        {
            "#version 400 core\n",
            "layout (location = 0) in vec2 aPos;\n",
            "layout (location = 1) in vec4 aColor;\n",
            "out vec4 Color;\n",
            "void main(){ \n",
            "   gl_Position = vec4(aPos,0.0f,1.0f);\n",
            "   Color = aColor;}"};

    const char *fragmentShaderSource[] =
        {
            "#version 400 core\n",
            "in vec4 Color;",
            "out vec4 fragColor;",
            "void main(){\n",
            // "   fragColor = vec4(1.0f,0.0f,1.0f,1.0f);}"
            "   fragColor = Color;}"};

    unsigned int vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, sizeof(vertexShaderSource) / sizeof(char *), vertexShaderSource, NULL);
    glShaderSource(fragmentShader, sizeof(fragmentShaderSource) / sizeof(char *), fragmentShaderSource, NULL);

    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);

    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        fputs("VERTEX SHADER COMPILATION FAILED::: ", stderr);
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        fputs(infoLog, stderr);
        exit(EXIT_FAILURE);
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        fputs("FRAGMENT SHADER COMPILATION FAILED::: ", stderr);
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        fputs(infoLog, stderr);
        exit(EXIT_FAILURE);
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        fputs("SHADER LINKING FAILED::: ", stderr);
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fputs(infoLog, stderr);
        exit(EXIT_FAILURE);
    }
    if (format == 6)
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    else if (format == 2)
    {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // glPointSize(10);
    while (!glfwWindowShouldClose(window))
    {
        glBindVertexArray(VAO);
        glUseProgram(shaderProgram);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_POINTS, 0, width * height);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

Point normalized_coordinate(Point image_point)
{
    Point temp;
    temp.x = 2 * image_point.x / screen_width - 1;
    temp.y = 1 - 2 * image_point.y / screen_height;
    // temp.y = 2*image_point.y/screen_height -1;
    return temp;
}

void key_callback(GLFWwindow *window, int key, int scanncode, int action, int mod)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void on_size_change(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}
