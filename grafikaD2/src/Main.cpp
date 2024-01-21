
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.hpp"
#include "Types.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 5000.0f

GLFWwindow* _window;
Shader _terrainShader;
Shader _skyboxShader;
Transform _cameraTransform;
Texture _normalMapTexture;
CubemapTexutre _skyboxTexture;
HeightMap _heightMap;
GLuint _cubeVao = 0;
double _mouseX = 0;
double _mouseY = 0;	
double _mouseOffsetX = 0;
double _mouseOffsetY = 0;
float _mouseSensitivity = 0.002f;
float _walkSpeed = 1.1f;

void Init(int width, int height, std::string title) {

	// OpenGL init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// preventuje backwards compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	if (_window == NULL) {
		std::cout << "GLFW window failed creation\n";
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(_window);
	// glad managuje pointere ka funkcijama u opengl-u, ovde ga initujemo
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "GLAD failed init\n";
		return;
	}

	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Load shaders
	_terrainShader.Load("terrain.vert", "terrain.frag");
	_skyboxShader.Load("skybox.vert", "skybox.frag");
	
	// Load textures
	_normalMapTexture.Load("res/textures/NormalMap.png");

	// Load height map
	_heightMap.Load("res/textures/HeightMap.png");

	// Load cubemap textures
	// za cubemap 6 - 2d i napravimo kockicu
	std::vector<std::string> skyboxTextureFilePaths;
	skyboxTextureFilePaths.push_back("res/textures/right.png");
	skyboxTextureFilePaths.push_back("res/textures/left.png");
	skyboxTextureFilePaths.push_back("res/textures/top.png");
	skyboxTextureFilePaths.push_back("res/textures/bottom.png");
	skyboxTextureFilePaths.push_back("res/textures/front.png");
	skyboxTextureFilePaths.push_back("res/textures/back.png");
	_skyboxTexture.Create(skyboxTextureFilePaths);

	// Init cube vertices (for skybox)	
	std::vector<glm::vec3> cubeVertices;
	std::vector<unsigned int> cubeIndices;
	// velicina cuba 1
	float d = 0.5f;
	// dodajemo sve moguce trouglove koji postoje u cube mapi
	cubeVertices.push_back(glm::vec3(-d, d, d));	// Gore
	cubeVertices.push_back(glm::vec3(-d, d, -d));
	cubeVertices.push_back(glm::vec3(d, d, -d));
	cubeVertices.push_back(glm::vec3(d, d, d));	
	cubeVertices.push_back(glm::vec3(-d, -d, d));	// Dole
	cubeVertices.push_back(glm::vec3(-d, -d, -d));
	cubeVertices.push_back(glm::vec3(d, -d, -d));
	cubeVertices.push_back(glm::vec3(d, -d, d));
	cubeVertices.push_back(glm::vec3(-d, d, d));	// Z napred
	cubeVertices.push_back(glm::vec3(-d, -d, d));
	cubeVertices.push_back(glm::vec3(d, -d, d));
	cubeVertices.push_back(glm::vec3(d, d, d));
	cubeVertices.push_back(glm::vec3(-d, d, -d));	// Z pozadi
	cubeVertices.push_back(glm::vec3(-d, -d, -d));
	cubeVertices.push_back(glm::vec3(d, -d, -d));
	cubeVertices.push_back(glm::vec3(d, d, -d));
	cubeVertices.push_back(glm::vec3(d, d, -d));	// X napred
	cubeVertices.push_back(glm::vec3(d, -d, -d));
	cubeVertices.push_back(glm::vec3(d, -d, d));
	cubeVertices.push_back(glm::vec3(d, d, d));
	cubeVertices.push_back(glm::vec3(-d, d, -d));	// X pozadi
	cubeVertices.push_back(glm::vec3(-d, -d, -d));
	cubeVertices.push_back(glm::vec3(-d, -d, d));
	cubeVertices.push_back(glm::vec3(-d, d, d));
	cubeIndices = { 0, 1, 3, 1, 2, 3, 7, 5, 4, 7, 6, 5, 11, 9, 8, 11, 10, 9, 12, 13, 15, 13, 14, 15, 16, 17, 19, 17, 18, 19, 23, 21, 20, 23, 22, 21 };
	unsigned int vbo;
	unsigned int ebo;
	// samo mu dajemo ID i stavljamo buffere
	glGenVertexArrays(1, &_cubeVao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	// radimo sa njim
	glBindVertexArray(_cubeVao);
	// povezujemo GL_ARRAY_BUFFER i vbo i onda ucitavamo vector po vector iz cubeVertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(glm::vec3), &cubeVertices[0], GL_STATIC_DRAW);
	// isto sve za indexe kao za vertices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), &cubeIndices[0], GL_STATIC_DRAW);
	// layout nula pos vector3
	glEnableVertexAttribArray(0);
	// ovde kazemo kako da cita datu i odakle sve kao pre
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

	// unbindujemo vbo
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// unbindujemo vbo
	glBindVertexArray(0);
	
	// na poziciji position i da gleda malo zarotiran (test)
	_cameraTransform.position = glm::vec3(0, 36.0f, 0);
	_cameraTransform.rotation = glm::vec3(-0.25f, 0.0f, 0.0f);

	// Init mouse state
	double x, y;
	glfwGetCursorPos(_window, &x, &y);
	_mouseOffsetX = x;
	_mouseOffsetY = y;
	_mouseX = x;
	_mouseY = y;
}

void Update() {

	// Quit?
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(_window, true);
	}
	
	// za test nebitno za kod
	if (glfwGetKey(_window, GLFW_KEY_H) == GLFW_PRESS) {
		_terrainShader.Load("terrain.vert", "terrain.frag");
	}

	// Mouselook	
	double x, y;
	glfwGetCursorPos(_window, &x, &y);
	// pos i offset
	_mouseOffsetX = x - _mouseX;
	_mouseOffsetY = y - _mouseY;
	_mouseX = x;
	_mouseY = y;

	// jasno je (posle sat vremena XD) da treba ovako da se odradi (minusi su logicni kao u 2d) ali ako rotiramo oko y dobijamo x view,
	// zato za rotaciju kazemo da se offsetY gleda
	_cameraTransform.rotation.x += -_mouseOffsetY * _mouseSensitivity;
	_cameraTransform.rotation.y += -_mouseOffsetX * _mouseSensitivity;
	// malo manje od pola PI-a da se ne bih usinuo
	_cameraTransform.rotation.x = std::min(_cameraTransform.rotation.x, 1.5f);
	_cameraTransform.rotation.x = std::max(_cameraTransform.rotation.x, -1.5f);


	// uzimam desnu i prednju stranu, kao od desnog ramena i kao iz ociju da pucam vektore, i samo stavljam da je 0 y da ne idem gore dole
	glm::vec3 camRight = glm::vec3(_cameraTransform.to_mat4()[0]);
	glm::vec3 camForward = glm::vec3(_cameraTransform.to_mat4()[2]);
	glm::vec3 movementForwardVector = glm::normalize(glm::vec3(camForward.x, 0, camForward.z));

	// kretnja
	// ...
	glm::vec3 displacement = glm::vec3(0);
	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS) {
		displacement -= camRight;
	}
	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS) {
		displacement += camRight;
	}
	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS) {
		displacement -= movementForwardVector;
	}
	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS) {
		displacement += movementForwardVector;
	}
	// menjam poziciju kamere na osnovu displacementa
	displacement *= _walkSpeed;
	_cameraTransform.position += displacement;
}


void Render() {

	// std::cout << "PROBAAAAA";
	// klasika za clearovanje
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// MORA DA SE ENABLEUJE OVO INACE NE ZNA KAKO DA CRTA
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);


	// projection matrica, projektujemo 3d na 2d plane 1f je tipa radijani (ugao gledanja)
	glm::mat4 projection = glm::perspective(1.0f, 1920.0f / 1080.0f, NEAR_PLANE, FAR_PLANE);

	// inverse kad se pozove
	glm::mat4 view = glm::inverse(_cameraTransform.to_mat4());

	// Render skybox
	// stavljamo ga static da bi radila ova mini rotacijica (kao pomeraju se oblaci itd)
	static Transform skyBoxTransform;
	// -10 je jer je tekstura za skybox malo zajebana pa zato
	skyBoxTransform.position = _cameraTransform.position + glm::vec3(0, -10, 0);
	skyBoxTransform.scale = glm::vec3(100.f);
	skyBoxTransform.rotation.y -= 0.00015f;

	// bind je smao gl use program od id shadera, ove funkcije imamo dole setMat4 koja samo uzima svaku od ovih mat4 i ubacuje ih u shadere
	_skyboxShader.Bind();
	_skyboxShader.SetMat4("projection", projection);
	_skyboxShader.SetMat4("view", view);
	_skyboxShader.SetMat4("model", skyBoxTransform.to_mat4());
	glActiveTexture(GL_TEXTURE0);
	// bindujem skyboxTexture na gl_texture_cube_map 
	glBindTexture(GL_TEXTURE_CUBE_MAP, _skyboxTexture.ID);
	// bindujemo sta crtamo vao
	glBindVertexArray(_cubeVao);
	// 36 indexa imamo u onom nizu sto smo pravili i crtamo
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Render heightmap
	// veoma slicno sve dole kao za 
	Transform heightMapTransform;
	// ovo je samo da bih malo naglasio height mapu
	heightMapTransform.scale = glm::vec3(2.0f);
	_terrainShader.Bind();	
	_terrainShader.SetMat4("projection", projection);
	_terrainShader.SetMat4("view", view);
	_terrainShader.SetMat4("model", heightMapTransform.to_mat4());
	// uzimam poziciju viewa da bih znao gde da iscrtam fog
	_terrainShader.SetVec3("viewPos", glm::vec3(_cameraTransform.to_mat4()[3]));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _normalMapTexture.ID);
	glBindVertexArray(_heightMap.vao);
	glDrawElements(GL_TRIANGLE_STRIP, _heightMap.indexCount, GL_UNSIGNED_INT, 0);

	glClear(GL_DEPTH_BUFFER_BIT);

	glfwSwapBuffers(_window);
	glfwPollEvents();
}

int main() {

	Init(1920 * 1.5f, 1080 * 1.5f, "grafikaD2");
	
	// zove update i render dok ne setujemo flag
	while (!glfwWindowShouldClose(_window)) {
		Update();
		Render();
	}

	glfwTerminate();
	return 0;
}
