/*---------------------------------------------------------*/
/* ----------------  Proyecto Final -----------------------*/
/*-----------------    2023-2   ---------------------------*/
/*------------- No. Cuenta 316037837 ----------------------*/
#include <Windows.h>

#include <glad/glad.h>
#include <glfw3.h>	
#include <stdlib.h>		
#include <glm/glm.hpp>	
#include <glm/gtc/matrix_transform.hpp>	
#include <glm/gtc/type_ptr.hpp>
#include <time.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void my_input(GLFWwindow *window);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// configuracion 
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor *monitors;

void getResolution(void);

// camara
Camera camera(glm::vec3(175.0f, 80.0f, 400.0f));//posicion donde inicia la camara
float MovementSpeed = 0.1f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

//Luz
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);

// posiciones
float	movTele_x = 0.0f,
		movTele_y = 0.0f,
		movTele_z = 0.0f,
		movSofa_x = 0.0f,
		movSofa_y = 0.0f,
		movSofa_z = 0.0f;
bool	animacion_TV = false,
		animacion_Sofa = false;
bool animacion_habilitada = true,
	 animacion_Sofa_habilitada = true;
int	estado_TV = 0,
	estado_Sofa;

void animate(void)//funcion donde se generan las animciones y se le llamara en distintas partes del programa para que se ejecuten las animaciones
{
	//se gestiona la animacion de la television
	if (animacion_TV && movTele_z >= -10.0f)
	{
		if (estado_TV == 1) {//se eleva el televisor
			movTele_y += 4.0f;
			if (movTele_y >= 20.0f) {
				estado_TV = 2;
			}
		}
		if (estado_TV == 2) {//se mueve en z para acomodarse a la altura de vision
			movTele_z += 4.0f;
			if (movTele_z >= 100.0f) {
				estado_TV = 3;
			}
		}
		if (estado_TV == 3) {//avanza en x para impactar a vision
			movTele_x -= 4.0f;
			if (movTele_x <= -340.0f) {
				animacion_TV = false;//se termina la animacion
			}
		}

	}
	//se gestiona la animacion del sofa
	if (animacion_Sofa && movSofa_z >= -10.0f)
	{
		if (estado_Sofa == 1) {//se eleva el sofa
			movSofa_y += 4.0f;
			if (movSofa_y >= 20.0f) {
				estado_Sofa = 2;
			}
		}
		if (estado_Sofa == 2) {//va hacia adelante en el eje x para impactar con vision
			movSofa_x -= 4.0f;
			if (movSofa_x <= -350.0f) {
				animacion_Sofa = false;//se termina la animacion
			}
		}

	}
}


void getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}


int main()
{
	// glfw: se initializa y configura
	// ------------------------------
	glfwInit();

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// creacion de ventana con glfw 

	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CGeIHC", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// indicar a GLFW que capture nuestro mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: cargar todos los punteros de funciones de OpenGL
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configurar estado global de OpenGL
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// compilar y construir shaders
	// -------------------------
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights_mod.fs");
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

	vector<std::string> faces
	{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Configuración del shader
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// Carga de modelos
	// -----------
	Model TV("resources/objects/TV/TV.obj");
	Model CasaWandavision("resources/objects/CasaWandavision/CasaWandavision.obj");
	Model sofa_chico("resources/objects/sofa_chico/sofa_chico.obj");
	Model sofa_grande("resources/objects/sofa/sofa.obj");
	Model mesa_tele("resources/objects/mesa_tele/mesa_tele.obj");
	Model mesa_central("resources/objects/mesa_central/mesa_central.obj");
	Model barra_principal_cocina("resources/objects/barra_principal_cocina/barra_principal_cocina.obj");
	// modelos animados
	ModelAnim vision("resources/objects/vision/vision.dae");
	vision.initShaders(animShader.ID);
	ModelAnim wanda("resources/objects/wanda/wanda.dae");
	wanda.initShaders(animShader.ID);
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		skyboxShader.setInt("skybox", 0);
		
		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		animate();

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// no olvides habilitar el shader antes de establecer los uniforms
		staticShader.use();
		//Configuración de luces
		staticShader.setVec3("viewPos", camera.Position);
		staticShader.setVec3("dirLight.direction", lightDirection);
		staticShader.setVec3("dirLight.ambient", glm::vec3(0.9f, 0.9f, 0.9f));
		staticShader.setVec3("dirLight.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("dirLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));

		staticShader.setVec3("pointLight[0].position", glm::vec3(0.0, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[0].constant", 1.0f);
		staticShader.setFloat("pointLight[0].linear", 0.009f);
		staticShader.setFloat("pointLight[0].quadratic", 0.032f);

		staticShader.setVec3("pointLight[1].position", glm::vec3(-10.0, 0.0f, -150.0f));
		staticShader.setVec3("pointLight[1].ambient", glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].diffuse", glm::vec3(1.0f, 0.0f, 0.0f));  
		staticShader.setVec3("pointLight[1].specular", glm::vec3(1.0f, 0.0f, 0.0f));  
		staticShader.setFloat("pointLight[1].constant", 1.0f);
		staticShader.setFloat("pointLight[1].linear", 0.0009f);
		staticShader.setFloat("pointLight[1].quadratic", 0.0032f);


		staticShader.setVec3("spotLight[0].position", glm::vec3(0.0f, 20.0f, 10.0f));
		staticShader.setVec3("spotLight[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
		staticShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
		staticShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(60.0f)));
		staticShader.setFloat("spotLight[0].constant", 1.0f);
		staticShader.setFloat("spotLight[0].linear", 0.0009f);
		staticShader.setFloat("spotLight[0].quadratic", 0.0005f);

		staticShader.setFloat("material_shininess", 32.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// transformaciones de vista/proyección
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//// Luz
		glm::vec3 lightColor = glm::vec3(0.6f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);
		

		// -------------------------------------------------------------------------------------------------------------------------
		// Personajes Animados
		// -------------------------------------------------------------------------------------------------------------------------
		animShader.use();
		animShader.setMat4("projection", projection);
		animShader.setMat4("view", view);

		animShader.setVec3("material.specular", glm::vec3(0.5f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);

		
		// -------------------------------------------------------------------------------------------------------------------------
		// Wanda animada
		// -------------------------------------------------------------------------------------------------------------------------

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 0.0f, -150.0f));
		model = glm::rotate(model, glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(50.0f));//cambia tamaño del modelo
		animShader.setMat4("model", model);
		wanda.Draw(staticShader);
		// -------------------------------------------------------------------------------------------------------------------------
		// Vision animado
		//-------------------------------------------------------------------------------------------------------------------------

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 0.0f, 10.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(50.0f));//cambia tamaño del modelo
		animShader.setMat4("model", model);
		vision.Draw(staticShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//Se dibuja Casa de Wandavision y dentro de la casa ya vienen colocados los objetos siguientes:
		// alacena, buro de cocina, fregadero y refrigerador ademas de la fachada con sus elementos (2 arboles, 2 arbustos) 
		//y la escalera como se muestra en las imagenes de referencia en la sala aunque de esta ultima no se halla hecho enfoque en su recreacion 
		//en las imagenes de referencia
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 92.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));//cambia tamaño del modelo
		staticShader.setMat4("model", model);
		CasaWandavision.Draw(staticShader);

		//sofa_chico que esta al lado del televisor como en las imagenes de referencia
		model = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f + movSofa_x, 0.0f + movSofa_y, 10.0f + movSofa_z));// se define la posicion inicial del sofa y variables auxiliares para mover el sofa
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(1.0f));//cambia tamaño del modelo
		staticShader.setMat4("model", model);
		sofa_chico.Draw(staticShader);

		//sofa
		model = glm::translate(glm::mat4(1.0f), glm::vec3(160.0f, 0.0f, -60.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(0.9f));//cambia tamaño del modelo
		staticShader.setMat4("model", model);
		sofa_grande.Draw(staticShader);
		
		//se dibuja televisor
		model = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f + movTele_x, 41.0f + movTele_y, -100.0f + movTele_z));//se define la posicion inicial de la tele y variables auxiliares para mover la tele
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(1.0f));//cambia tamaño del modelo
		staticShader.setMat4("model", model);
		TV.Draw(staticShader);

		//mesa de la television o soporte de la television 
		model = glm::translate(glm::mat4(1.0f), glm::vec3(300.0f, 0.0f, -100.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(1.0f));//cambia tamaño del modelo
		staticShader.setMat4("model", model);
		mesa_tele.Draw(staticShader);
		//mesa principal (donde se aprecia una esfera en las imagenes de referencia) esta enfrente del televisor
		model = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, -10.0f, -100.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(1.0f));//cambia tamaño del modelo
		staticShader.setMat4("model", model);
		mesa_central.Draw(staticShader);
		//barra principal de la cocina (se eligio dibujar aqui y no colocar en la casa directamente por el peso del obj y la restriccion del peso de 100 mb de github)
		model = glm::translate(glm::mat4(1.0f), glm::vec3(40.0f, 55.0f, -350.0f));
		model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f));//rotacion 
		model = glm::scale(model, glm::vec3(1.0f));//cambia tamaño del modelo
		staticShader.setMat4("model", model);
		barra_principal_cocina.Draw(staticShader);
		// -------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario
		// -------------------------------------------------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------------
		// Dibuja el skybox al final
		// -------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, view, projection, camera);

		// Limita el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // Tiempo para completar un ciclo completo.
		
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		// glfw: intercambia los buffers y procesa los eventos de entrada (teclas presionadas/liberadas, movimiento del mouse, etc.).
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.Terminate();

	glfwTerminate();
	return 0;
}

// Procesa toda la entrada: consulta a GLFW si las teclas relevantes se han presionado/liberado en este frame y reacciona en consecuencia.
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);


	//televisor animacion donde se presiona una vez la tecla 1 para iniciar la animacion y presionar mas veces la tecla 1 no interrumpe la animacion para que termine adecuadamente
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		if (animacion_habilitada) { 
			estado_TV = 1;
			animacion_TV ^= true;
			animacion_habilitada = false;
		}
	}
	//la tecla 2 regresa a su posicion original al televisor y espera a que presionando la tecla 1 se inicie de nuevo la animacion
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		if (!animacion_TV) {
			movTele_x = 0.0f;
			movTele_z = 0.0f;
			movTele_y = 0.0f;
			animacion_TV = false;
			animacion_habilitada = true;
		}
	}


	//Sofa chico animacion donde se presiona una vez la tecla 3 para iniciar la animacion y presionar mas veces la tecla 3 no interrumpe la animacion para que termine adecuadamente 
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		if (animacion_Sofa_habilitada) {
			estado_Sofa = 1;
			animacion_Sofa ^= true;
			animacion_Sofa_habilitada = false;
		}
	}
	//la tecla 4 regresa a su posicion original al televisor y espera a que presionando la tecla 3 se inicie de nuevo la animacion
	if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
		if (!animacion_Sofa) {
			movSofa_x = 0.0f;
			movSofa_z = 0.0f;
			movSofa_y = 0.0f;
			animacion_Sofa = false;
			animacion_Sofa_habilitada = true;
		}
	}

}

// glfw: cada vez que cambia el tamaño de la ventana (ya sea por el sistema operativo o por el redimensionamiento del usuario), se ejecuta esta función de devolución de llamada.
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// Nos Aseguramos de que el área de visualización (viewport) coincida con las nuevas dimensiones de la ventana
	// ten en cuenta que el ancho y alto serán significativamente mayores que los especificados en pantallas Retina
	glViewport(0, 0, width, height);
}

// glfw: cada vez que se mueve el mouse, se llama a esta función de devolución de llamada.
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // glfw: al estar invertidas las coordenadas y, que van de abajo hacia arriba, esta función de devolución de llamada se ejecuta cada vez que el mouse se mueve

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: cada vez que se desplaza la rueda del ratón, se llama a esta función de devolución de llamada.
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}