#include "main.h"
#include "mesh.h"
#include "model.h"

#define SCALE 0.0104f

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
float scale = 1.0f;
//glm::vec3 front(camera.Front), rright(camera.Right), up(camera.Up);
bool firstMouse = true;

bool spotEnabled = false;
bool spotKeyPressed = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void init();
void checkFailure(GLFWwindow *window);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubemap(vector<std::string> faces);
void renderScene(const Shader &shader, Model &houseModel);


float near_plane = 1.0f, far_plane = 20.5f;
int main()
{

	init();
	GLFWwindow* window= window = glfwCreateWindow(WIDTH, HEIGHT, "HouseModel", NULL, NULL);
	{
		checkFailure(window);

		glViewport(0, 0, WIDTH, HEIGHT);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

	}
	// configure global opengl state
	// -----------------------------
	
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_FRAMEBUFFER_SRGB);
	//glEnable(GL_CULL_FACE);
	Shader ourShader("shader.vs","shader.fs");
	Shader lampShader("lamp.vs", "lamp.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");
	Shader simpleDepthShader("shadow.vs", "shadow.fs");


	// load models
	// -----------
	Model houseModel("../Resources/house_skp/house.obj");
	//Model houseModel("../Resources/nanosuit/nanosuit.obj");
	//Model houseModel("../Resources/oggy/house.obj");
	//Model houseModel("../Resources/3ds/House N111111.3DS");
	//Model houseModel("../Resources/plane/plane.obj");
	//Model houseModel("../Resources/house/house.obj");
	//Model houseModel("../Resources/house3/house3.obj");
	//Model houseModel("../Resources/Old House/house.obj");

	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(1.4f,  0.4f,  4.0f),
		glm::vec3(4.6f, -6.6f, -8.0f),
		glm::vec3(-8.0f,  4.0f, -24.0f),
		glm::vec3(0.0f,  0.0f, -6.0f)
	};

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};


	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	{
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	}

	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//cubemap
	vector<std::string> faces
	{
			"../Resources/skybox/right.jpg",
			"../Resources/skybox/left.jpg",
			"../Resources/skybox/top.jpg",
			"../Resources/skybox/bottom.jpg",
			"../Resources/skybox/front.jpg",
			"../Resources/skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// configure depth map FBO
   // -----------------------
	const unsigned int SHADOW_WIDTH = 1920, SHADOW_HEIGHT = 1920;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f,  1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// lighting info
	// -------------
	glm::vec3 lightDir(5.0f, -4.0f, 1.0f);

	//shadowmap  config in our shader
	ourShader.use();
	ourShader.setInt("shadowMap", 0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);
		// render
		// ------
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		//cout << near_plane << " " << far_plane << " "<<scale<<endl;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(-lightDir, glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		// render scene from light's point of view
		simpleDepthShader.use();
		simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(simpleDepthShader,houseModel);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// 2. render scene as normal using the generated depth/shadow map  
		// --------------------------------------------------------------
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.use();
		{
			ourShader.setVec3("viewPos", camera.Position);

			/*
			   Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
			   the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
			   by defining light types as classes and set their values in there, or by using a more efficient uniform approach
			   by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
			*/
			// directional light
			ourShader.setVec3("dirLight.direction", lightDir);
			ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
			ourShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
			ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

			// point light 1
			ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
			ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[0].constant", 1.0f);
			ourShader.setFloat("pointLights[0].linear", 0.09);
			ourShader.setFloat("pointLights[0].quadratic", 0.032);
			// point light 2
			ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
			ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[1].constant", 1.0f);
			ourShader.setFloat("pointLights[1].linear", 0.09);
			ourShader.setFloat("pointLights[1].quadratic", 0.032);
			// point light 3
			ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
			ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[2].constant", 1.0f);
			ourShader.setFloat("pointLights[2].linear", 0.09);
			ourShader.setFloat("pointLights[2].quadratic", 0.032);
			// point light 4
			ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
			ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
			ourShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("pointLights[3].constant", 1.0f);
			ourShader.setFloat("pointLights[3].linear", 0.09);
			ourShader.setFloat("pointLights[3].quadratic", 0.032);
			// spotLight
			ourShader.setVec3("spotLight.position", camera.Position);
			ourShader.setVec3("spotLight.direction", camera.Front);
			//ourShader.setVec3("spotLight.ambient", 0.05f, 0.05f, 0.05f);
			ourShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
			ourShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
			ourShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
			ourShader.setFloat("spotLight.constant", 1.0f);
			ourShader.setFloat("spotLight.linear", 0.09);
			ourShader.setFloat("spotLight.quadratic", 0.032);
			ourShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
			ourShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));


		}//light uiforms

			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();
			ourShader.setMat4("projection", projection);
			ourShader.setMat4("view", view);
		
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthMap);
		// render the loaded model
			renderScene(ourShader,houseModel);
		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		//model = glm::rotate(model, glm::radians(angleZ), glm::vec3(0.0f,0.0f,-1.0f));
		//model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
		//// translate it down so it's at the center of the scene
		//model = glm::scale(model, scale*glm::vec3(SCALE, SCALE, SCALE));	// it's a bit too big for our scene, so scale it down
		//ourShader.setMat4("model", model);
		/*ourShader.setInt("spot", spotEnabled);
		
		houseModel.Draw(ourShader);*/

		// also draw the lamp object(s)
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);

		// we now draw as many light bulbs as we have point lights.
		glBindVertexArray(skyboxVAO);
		glm::mat4 model = glm::mat4(1.0f);
		//pointlights
		for (unsigned int i = 0; i < 4; i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.1f)); // Make it a smaller cube
			lampShader.setMat4("model", model);
			 glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		////directionalLight
		//model = glm::mat4(1.0f);
		//model = glm::translate(model, -lightDir);
		//model = glm::scale(model, glm::vec3(0.1f)); // Make it a smaller cube
		//lampShader.setMat4("model", model);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		////origin
		//model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(0.1f)); // Make it a smaller cube
		//lampShader.setMat4("model", model);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));// remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVAO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;

}

void renderScene(const Shader &shader,Model &houseModel)
{
	// render the loaded model
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-3.0f, -1.75f, 0.0f));
		model = glm::rotate(model, glm::radians(angleZ), glm::vec3(0.0f,0.0f,-1.0f));
		model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
		// translate it down so it's at the center of the scene
		model = glm::scale(model, scale*glm::vec3(SCALE, SCALE, SCALE));	// it's a bit too big for our scene, so scale it down
		shader.setMat4("model", model);

		shader.setInt("spot", spotEnabled);

		houseModel.Draw(shader);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}
void checkFailure(GLFWwindow *window)
{
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}


}
void processInput(GLFWwindow *window)
{
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		scale -= scale * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		scale += scale * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		angleX += deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		angleX -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
		angleY += deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
		angleY -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		angleZ -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		angleZ += deltaTime * 45;


	/*if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		near_plane -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		near_plane += deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		far_plane -= deltaTime * 45;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		far_plane += deltaTime * 45;*/


	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spotKeyPressed)
	{
		spotEnabled = !spotEnabled;
		spotKeyPressed = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		spotKeyPressed = false;
	}

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	int state2 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (state == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (firstMouse)
		{
			lastX = (float)xpos;
			lastY = (float)ypos;
			firstMouse = false;
		}

		float xoffset = (float)xpos - lastX;
		float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

		lastX = (float)xpos;
		lastY = (float)ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	else
	{
		if (firstMouse == false)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstMouse = true;
		}
	}


}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}