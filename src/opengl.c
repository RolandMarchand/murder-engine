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
GLuint cubeVAO;
GLuint lightVAO;
GLuint shaderProgram;
GLuint lightShaderProgram;
GLuint texture0;
GLuint texture1;
vec3 lightPosition = {0.0f, 0.0f, -10.0f};

uint32_t frameCount;
float lastFrameTimeSec;
float currentFrameTimeSec;
float deltaTimeSec;
float cameraSpeed = 10.0f;

static constexpr float cameraFOVMin = 0.26f;
static constexpr float cameraFOVMax = 1.75f;

float cameraFOV = GLM_PI / 2.0f;
vec3 cameraEuler;
vec3 cameraPosition = {0.0f, 0.0f, 3.0f};

void setUniformBool(GLuint shaderID, const GLchar *name, GLboolean value)
{
	glUniform1i(glGetUniformLocation(shaderID, name), (GLint)value);
}

void setUniformInt(GLuint shaderID, const GLchar *name, GLint value)
{
	glUniform1i(glGetUniformLocation(shaderID, name), value);
}

void setUniformFloat(GLuint shaderID, const GLchar *name, GLfloat value)
{
	glUniform1f(glGetUniformLocation(shaderID, name), value);
}

void setUniformVec3(GLuint shaderID, const GLchar *name, vec3 value)
{
	glUniform3fv(glGetUniformLocation(shaderID, name), 1, value);
}

void setUniformMatrix(GLuint shaderID, const GLchar *name, mat4 value)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderID, name), 1, GL_FALSE,
			   (GLfloat*)value);
}

void getCameraFront(vec3 out)
{
	out[0] = 0.0f;
	out[1] = 0.0f;
	out[2] = 1.0f;

	glm_vec3_rotate(out, cameraEuler[0], GLM_XUP);
	glm_vec3_rotate(out, cameraEuler[1], GLM_YUP);
	glm_vec3_rotate(out, cameraEuler[2], GLM_ZUP);

	out[2] = -out[2];

	glm_normalize(out);
}

void processCamera(GLFWwindow *window)
{
	vec3 velocity = {};

	velocity[0] -= (float)(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
	velocity[0] += (float)(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
	velocity[1] += (float)(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
	velocity[1] -= (float)(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
	velocity[2] += (float)(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
	velocity[2] -= (float)(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);

	glm_vec3_rotate(velocity, -cameraEuler[1], GLM_YUP);
	glm_vec3_scale(velocity, cameraSpeed * deltaTimeSec, velocity);
	glm_vec3_add(velocity, cameraPosition, cameraPosition);
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
		CLAMP(cameraEuler[0], -(float)GLM_PI / 3.0f, (float)GLM_PI / 3.0f);
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

Error windowInit(void)
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

/* Buffers data to VBO global. */
void bufferMeshData(const GLfloat *vertices, GLsizeiptr length)
{
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, length * (GLsizeiptr)sizeof(GLfloat),
		     vertices, GL_STATIC_DRAW);
}

void cubeVertexBufferInit(GLuint pVBO)
{
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pVBO);

	/* Set position attribute. */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
			      nullptr);
	glEnableVertexAttribArray(0);

	/* Set texture attribute. */
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
			      (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	/* Set normals. */
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
			      (void*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
}

void lightVertexBufferInit(GLuint pVBO)
{
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pVBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
			      nullptr);
	glEnableVertexAttribArray(0);
}

void vertexBuffersInit(void)
{
	const GLfloat vertices[] = {
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,	1.0f, 0.0f,	0.0f, 0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,	1.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,	1.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f,	0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 0.0f,	0.0f, 0.0f, -1.0f,
	
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f,	0.0f, 0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,	1.0f, 0.0f,	0.0f, 0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,	1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,	1.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f,	0.0f, 0.0f, 1.0f,
	
		-0.5f,  0.5f,  0.5f,	1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	1.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	1.0f, 0.0f,	-1.0f, 0.0f, 0.0f,
	
		0.5f,  0.5f,  0.5f,	1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		0.5f,  0.5f, -0.5f,	1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,	0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,	1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
	
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, -0.5f,	1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,	1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,	1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	0.0f, 0.0f,	0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	0.0f, -1.0f, 0.0f,
	
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,	1.0f, 1.0f,	0.0f, 1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,	1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,	1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
	};

	bufferMeshData(vertices, sizeof(vertices));
	cubeVertexBufferInit(VBO);
	lightVertexBufferInit(VBO);
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

	Error e = bindTexture(&texture0, RESOURCE_PATH "/crate.png");
	if (e != ERR_OK) {
		return e;
	}

	e = bindTexture(&texture1, RESOURCE_PATH "/crate-specular.png");
	if (e != ERR_OK) {
		return e;
	}

	glUseProgram(shaderID);
	setUniformInt(shaderID, "material.diffuse", 0);
	setUniformInt(shaderID, "material.specular", 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture1);

	return ERR_OK;
}

Error lightInit(GLuint shaderID)
{
	glUseProgram(shaderID);

	vec3 light = {1.0f, 1.0f, 1.0f};
	setUniformVec3(shaderID, "lightColor", light);

	return ERR_OK;
}

Error compileShaders(void)
{
	const GLchar vertexShaderSource[] = {
#embed "shaders/gl-vertex.glsl"
		, '\0'
	};
	const GLchar fragmentShaderSource[] = {
#embed "shaders/gl-fragment.glsl"
		, '\0'
	};
	const GLchar lightShaderSource[] = {
#embed "shaders/gl-light.glsl"
		, '\0'
	};

	Error e = compileShaderProgram(&shaderProgram, vertexShaderSource,
				       fragmentShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	e = compileShaderProgram(&lightShaderProgram, vertexShaderSource,
				       lightShaderSource);
	if (e != ERR_OK) {
		return e;
	}

	return ERR_OK;
}

Error graphicsInit(void)
{
	Error e = compileShaders();
	if (e != ERR_OK) {
		return e;
	}

	vertexBuffersInit();

	e = textureInit(shaderProgram);
	if (e != ERR_OK) {
		return e;
	}

	e = lightInit(lightShaderProgram);
	if (e != ERR_OK) {
		return e;
	}

	glEnable(GL_DEPTH_TEST);

	return ERR_OK;
}

void bindTransformMatrices(void)
{
	glUseProgram(shaderProgram);

	mat4 projection = GLM_MAT4_IDENTITY_INIT;
	glm_perspective(cameraFOV, (float)WIDTH / (float)HEIGHT, 0.1f,
			100.0f, projection);

	setUniformMatrix(shaderProgram, "projection", projection);
}

void drawCamera(void)
{
	glUseProgram(shaderProgram);

	mat4 view = GLM_MAT4_IDENTITY_INIT;
	glm_euler(cameraEuler, view);
	glm_translate_to(view, cameraPosition, view);
	setUniformMatrix(shaderProgram, "view", view);
}

void drawScene(void)
{
	glUseProgram(shaderProgram);
	glBindVertexArray(cubeVAO);

	vec3 cubePositions[] = {
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

	setUniformVec3(shaderProgram, "material.specular",
		       (vec3){1.0f, 1.0f, 1.0f});
	setUniformFloat(shaderProgram, "material.shininess", 32.0f);

	for (GLuint i = 0; i < 10; i++) {
		mat4 model = GLM_MAT4_IDENTITY_INIT;
		glm_translate(model, cubePositions[i]);
		GLfloat angle = (float)glfwGetTime() * ((GLfloat)i + 10);
		glm_rotate(model, glm_rad(angle), (vec3){1.0f, 0.3f, 0.5f});
		setUniformMatrix(shaderProgram, "model", model);
		setUniformVec3(shaderProgram, "viewPos", cameraPosition);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void drawLightCube(void)
{
	glBindVertexArray(lightVAO);
	glUseProgram(lightShaderProgram);

	mat4 model = GLM_MAT4_IDENTITY_INIT;
	glm_translate(model, lightPosition);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	setUniformMatrix(lightShaderProgram, "model", model);

	mat4 view = GLM_MAT4_IDENTITY_INIT;
	glm_euler(cameraEuler, view);
	glm_translate_to(view, cameraPosition, view);
	setUniformMatrix(lightShaderProgram, "view", view);

	mat4 projection = GLM_MAT4_IDENTITY_INIT;
	glm_perspective(cameraFOV, (float)WIDTH / (float)HEIGHT, 0.1f,
			100.0f, projection);
	setUniformMatrix(lightShaderProgram, "projection", projection);
}

void drawDirectionalLight()
{
	glUseProgram(shaderProgram);
	setUniformVec3(shaderProgram, "sunlight.color.ambient",
		       (vec3){0.5f, 0.0f, 0.0f});
	setUniformVec3(shaderProgram, "sunlight.color.diffuse",
		       (vec3){0.8f, 0.0f, 0.0f});
	setUniformVec3(shaderProgram, "sunlight.color.specular",
		       (vec3){1.0f, 0.0f, 0.0f});

	setUniformVec3(shaderProgram, "sunlight.dir", lightPosition);
}

void drawLightPoint()
{
	glUseProgram(shaderProgram);
	setUniformVec3(shaderProgram, "lightPoint.color.ambient",
		       (vec3){0.0f, 0.5f, 0.5f});
	setUniformVec3(shaderProgram, "lightPoint.color.diffuse",
		       (vec3){0.0f, 0.8f, 0.8f});
	setUniformVec3(shaderProgram, "lightPoint.color.specular",
		       (vec3){0.0f, 1.0f, 1.0f});

	setUniformVec3(shaderProgram, "lightPoint.position", lightPosition);

	setUniformFloat(shaderProgram, "lightPoint.falloff.constant", 1.0f);
	setUniformFloat(shaderProgram, "lightPoint.falloff.linear", 0.045f);
	setUniformFloat(shaderProgram, "lightPoint.falloff.quad", 0.0075f);
}

void drawSpotlight()
{
	glUseProgram(shaderProgram);
	setUniformVec3(shaderProgram, "spotlight.color.ambient",
		       (vec3){0.15f, 0.15f, 0.5f});
	setUniformVec3(shaderProgram, "spotlight.color.diffuse",
		       (vec3){0.24f, 0.24f, 0.8f});
	setUniformVec3(shaderProgram, "spotlight.color.specular",
		       (vec3){0.3f, 0.3f, 1.0f});

	setUniformVec3(shaderProgram, "spotlight.position", cameraPosition);
	setUniformFloat(shaderProgram, "spotlight.cutoff", cosf(glm_rad(12.5f)));
	setUniformFloat(shaderProgram, "spotlight.outerCutoff",
			cosf(glm_rad(17.5f)));

	setUniformFloat(shaderProgram, "spotlight.falloff.constant", 1.0f);
	setUniformFloat(shaderProgram, "spotlight.falloff.linear", 0.045f);
	setUniformFloat(shaderProgram, "spotlight.falloff.quad", 0.0075f);

	vec3 dir = {};
	getCameraFront(dir);
	setUniformVec3(shaderProgram, "spotlight.dir", dir);
}

void drawLight(void)
{
	drawLightCube();
	drawDirectionalLight();
	drawLightPoint();
	drawSpotlight();
}

Error drawFrame(void)
{
	glClearColor(0.28f, 0.16f, 0.22f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	bindTransformMatrices();

	drawCamera();

	drawLight();

	drawScene();

	glfwSwapBuffers(window);

	return ERR_OK;
}

void cleanupGraphics(void)
{
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
}

void cleanupWindow(void)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}
