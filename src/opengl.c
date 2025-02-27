#include <math.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "cglm/cglm.h"
#include "common.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CLAMP(X, MIN, MAX)					\
	((X) >= (MAX) ? (MAX) : ((X) <= (MIN) ? (MIN) : (X)))

enum {
	INFO_LOG_SIZE = 512,
};

GLchar infoLog[INFO_LOG_SIZE];
GLFWwindow *window;
GLuint VBO;
GLuint VAO;
GLuint EBO;
GLuint shaderProgram;
GLuint texture0;
GLuint texture1;

uint32_t frameCount;
float lastFrameTimeSec;
float currentFrameTimeSec;
float deltaTimeSec;
float cameraSpeed = 10.0f;

static constexpr float cameraFOVMin = 0.26f;
static constexpr float cameraFOVMax = 1.75f;

float cameraFOV = M_PI / 2.0f;
vec3 cameraEuler;
vec4 cameraPos = {0.0f, 0.0f, 3.0f};

GLvoid setUniformBool(GLuint shaderID, const GLchar *name, GLboolean value)
{
	glUniform1i(glGetUniformLocation(shaderID, name), (GLint)value);
}

GLvoid setUniformInt(GLuint shaderID, const GLchar *name, GLint value)
{
	glUniform1i(glGetUniformLocation(shaderID, name), value);
}

GLvoid setUniformFloat(GLuint shaderID, const GLchar *name, GLfloat value)
{
	glUniform1f(glGetUniformLocation(shaderID, name), value);
}

GLvoid setUniformMatrix(GLuint shaderID, const GLchar *name, mat4 value)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderID, name), 1, GL_FALSE,
			   (GLfloat*)value);
}

void processCamera(GLFWwindow *window)
{
	vec3 velocity = {0};

	velocity[0] -= (float)(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
	velocity[0] += (float)(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
	velocity[2] += (float)(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
	velocity[2] -= (float)(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);

	glm_vec3_rotate(velocity, -cameraEuler[1], GLM_YUP);
	glm_vec3_scale(velocity, cameraSpeed * deltaTimeSec, velocity);
	glm_vec3_add(velocity, cameraPos, cameraPos);
}

void processInput(GLFWwindow *window)
{
	(void)window;
	glfwPollEvents();
	processCamera(window);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
		 int mods)
{
	(void)scancode;
	(void)mods;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	(void)window;

	static float lastX = NAN;
	static float lastY = NAN;

	if (isnan(lastX) || isnan(lastY)) {
		lastX = (float)xpos;
		lastY = (float)ypos;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = -lastY + (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

	constexpr float sensitivity = 0.01f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	cameraEuler[0] += yoffset;
	cameraEuler[1] += xoffset;

	cameraEuler[0] =
		CLAMP(cameraEuler[0], -(float)M_PI / 3.0f, (float)M_PI / 3.0f);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	(void)window;
	(void)xoffset;

	cameraFOV -= (float)yoffset * 0.25f;
	cameraFOV = CLAMP(cameraFOV, cameraFOVMin, cameraFOVMax);
}

void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

Error initWindow(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WIDTH, HEIGHT, ENGINE_NAME, nullptr, nullptr);

	if (window == nullptr) {
		glfwTerminate();
		return ERR_WINDOW_CREATION_FAILED;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return ERR_GLAD_INITIALIZATION_FAILED;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	glfwSwapInterval(0);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);  
	glfwSetScrollCallback(window, scrollCallback);

	return ERR_OK;
}

Error compileVertexShader(GLuint *vertexShaderOut, const GLchar *code)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &code, nullptr);
	glCompileShader(vertexShader);

	/* Shader compilation results. */
	GLint success = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, INFO_LOG_SIZE, nullptr,
				   infoLog);
		(void)fprintf(stderr, "%s\n", infoLog);

	}

	*vertexShaderOut = vertexShader;

	return ERR_OK;
}

Error compileFragmentShader(GLuint *fragmentShaderOut, const GLchar *code)
{
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &code, nullptr);
	glCompileShader(fragmentShader);

	/* Shader compilation results. */
	GLint success = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, INFO_LOG_SIZE, nullptr,
				   infoLog);
		(void)fprintf(stderr, "%s\n", infoLog);
		return ERR_SHADER_CREATION_FAILED;
	}

	*fragmentShaderOut = fragmentShader;

	return ERR_OK;
}

Error linkShaderProgram(GLuint *shaderProgramOut, GLuint vertexShader,
			    GLuint fragmentShader)
{
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	/* Shader program linking results. */
	GLint success = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shaderProgram, INFO_LOG_SIZE, nullptr,
				   infoLog);
		(void)fprintf(stderr, "%s\n", infoLog);
		return ERR_SHADER_CREATION_FAILED;
	}

	*shaderProgramOut = shaderProgram;
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ERR_OK;
}

Error compileShaderProgram(GLuint *shaderIDOut,
			   const GLchar *vertexShaderSource,
			   const GLchar *fragmentShaderSource)
{
	/* Create vertex shader. */
	GLuint vertexShader = 0;
	Error e = compileVertexShader(&vertexShader, vertexShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	/* Create fragment shader. */
	GLuint fragmentShader = 0;
	e = compileFragmentShader(&fragmentShader, fragmentShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	GLuint shaderProgram = 0;
	e = linkShaderProgram(&shaderProgram, vertexShader, fragmentShader);
	if (e != ERR_OK) {
		return e;
	}

	*shaderIDOut = shaderProgram;

	return ERR_OK;
}

GLvoid vertexBufferInit(void)
{
	static const GLfloat vertices[] = {
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

	/* VAO */
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	/* VBO */
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
		     GL_STATIC_DRAW);

	/* Set position attribute. */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
			      nullptr);
	glEnableVertexAttribArray(0);

	/* Set texture attribute. */
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
			      (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(VAO);
}

Error bindTexture(GLuint *idOut, const char *path)
{
	int width = 0;
	int height = 0;
	int nrChannels = 0;
	unsigned char *data = stbi_load(path, &width,
					&height, &nrChannels, 0);
	if (data == nullptr) {
		return ERR_TEXTURE_LOADING_FAILED;
	}

	GLuint id = 0;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
		     GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);

	*idOut = id;

	return ERR_OK;
}

Error textureInit(GLuint shaderID)
{
	stbi_set_flip_vertically_on_load(true);

	Error e = bindTexture(&texture0, RESOURCE_PATH "/laz.png");
	if (e != ERR_OK) {
		return e;
	}

	e = bindTexture(&texture1, RESOURCE_PATH "/blue.png");
	if (e != ERR_OK) {
		return e;
	}

	glUseProgram(shaderID);
	setUniformInt(shaderID, "laz", 0);
	setUniformInt(shaderID, "blue", 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	return ERR_OK;
}

Error initGraphics(void)
{
	static const GLchar vertexShaderSource[] = {
#embed "shaders/gl-vertex.glsl"
		, '\0'
	};
	static const GLchar fragmentShaderSource[] = {
#embed "shaders/gl-fragment.glsl"
		, '\0'
	};

	Error e = compileShaderProgram(&shaderProgram, vertexShaderSource,
				       fragmentShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	vertexBufferInit();

	e = textureInit(shaderProgram);
	if (e != ERR_OK) {
		return e;
	}

	glEnable(GL_DEPTH_TEST);

	return ERR_OK;
}

void bindTransformMatrices(void)
{
	mat4 projection = {0};
	glm_perspective(cameraFOV, (float)WIDTH / (float)HEIGHT, 0.1f,
			100.0f, projection);

	setUniformMatrix(shaderProgram, "projection", projection);
}

void drawCamera(void)
{
	mat4 view = {0};
	glm_euler(cameraEuler, view);
	glm_translate_to(view, cameraPos, view);
	setUniformMatrix(shaderProgram, "view", view);
}

void drawScene(void)
{
	static vec3 cubePositions[] = {
		{ 0.0f,  0.0f,  0.0f},
		{ 2.0f,  5.0f, -15.0f},
		{-1.5f, -2.2f, -2.5f},
		{-3.8f, -2.0f, -12.3f},
		{ 2.4f, -0.4f, -3.5f},
		{-1.7f,  3.0f, -7.5f},
		{ 1.3f, -2.0f, -2.5f},
		{ 1.5f,  2.0f, -2.5f},
		{ 1.5f,  0.2f, -1.5f},
		{-1.3f,  1.0f, -1.5f},
	};

	for (GLuint i = 0; i < 10; i++) {
		mat4 model = GLM_MAT4_IDENTITY_INIT;
		glm_translate(model, cubePositions[i]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		GLfloat angle = (float)glfwGetTime() * ((GLfloat)i + 1);
		glm_rotate(model, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});
		setUniformMatrix(shaderProgram, "model", model);
	}
}

Error drawFrame(void)
{
	glClearColor(0.28f, 0.16f, 0.22f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);

	bindTransformMatrices();

	drawCamera();

	drawScene();

	glfwSwapBuffers(window);

	return ERR_OK;
}

void cleanupGraphics(void)
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void cleanupWindow(void)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}
