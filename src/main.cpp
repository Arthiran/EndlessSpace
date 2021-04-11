//Just a simple handler for simple initialization stuffs
#include "Utilities/BackendHandler.h"

#include <filesystem>
#include <json.hpp>
#include <fstream>

//TODO: New for this tutorial
#include <DirectionalLight.h>
#include <PointLight.h>
#include <UniformBuffer.h>
/////////////////////////////

#include <Texture2D.h>
#include <Texture2DData.h>
#include <MeshBuilder.h>
#include <MeshFactory.h>
#include <NotObjLoader.h>
#include <ObjLoader.h>
#include <VertexTypes.h>
#include <ShaderMaterial.h>
#include <RendererComponent.h>
#include <TextureCubeMap.h>
#include <TextureCubeMapData.h>

#include <Timing.h>
#include <GameObjectTag.h>
#include <InputHelpers.h>

#include <IBehaviour.h>
#include <Behaviours/CameraControlBehaviour.h>
#include <Behaviours/PlaneBehaviour.h>
#include <Behaviours/FollowPathBehaviour.h>
#include <Behaviours/HideBigFloor.h>
#include <SimpleMoveBehaviour.h>
#include <Behaviours/RotateObjectBehaviour.h>

#include <Physics/PhysicsEngine.h>
#include <Physics/PhysicsObject.h>

#include <Utilities/EffectHandler.h>

int main() {
	int frameIx = 0;
	float fpsBuffer[128];
	float minFps, maxFps, avgFps;
	int selectedVao = 0; // select cube by default
	std::vector<GameObject> controllables;

	bool drawGBuffer = false;
	bool drawIllumBuffer = false;

	BackendHandler::InitAll();

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(BackendHandler::GlDebugMessage, nullptr);

	// Enable texturing
	glEnable(GL_TEXTURE_2D);

	// Push another scope so most memory should be freed *before* we exit the app
	{
		#pragma region Shader and ImGui
		Shader::sptr passthroughShader = Shader::Create();
		passthroughShader->LoadShaderPartFromFile("shaders/passthrough_vert.glsl", GL_VERTEX_SHADER);
		passthroughShader->LoadShaderPartFromFile("shaders/passthrough_frag.glsl", GL_FRAGMENT_SHADER);
		passthroughShader->Link();

		Shader::sptr simpleDepthShader = Shader::Create();
		simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_vert.glsl", GL_VERTEX_SHADER);
		simpleDepthShader->LoadShaderPartFromFile("shaders/simple_depth_frag.glsl", GL_FRAGMENT_SHADER);
		simpleDepthShader->Link();

		//Init gBuffer shader
		Shader::sptr gBufferShader = Shader::Create();
		gBufferShader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		gBufferShader->LoadShaderPartFromFile("shaders/gBuffer_pass_frag.glsl", GL_FRAGMENT_SHADER);
		gBufferShader->Link();

		// Load our shaders
		Shader::sptr shader = Shader::Create();
		shader->LoadShaderPartFromFile("shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
		//Directional Light Shader
		shader->LoadShaderPartFromFile("shaders/directional_blinn_phong_frag.glsl", GL_FRAGMENT_SHADER);
		shader->Link();

		//Physics Stuff
		PhysicsEngine* physicsEngine;

		//Basic effect for drawing to
		PostEffect* basicEffect;
		Framebuffer* shadowBuffer;
		GBuffer* gBuffer;
		IlluminationBuffer* illumBuffer;

		//Post Processing Effects
		int activeEffect = 0;
		int activeColourGradingEffect = 0;
		std::vector<PostEffect*> effects;
		SepiaEffect* sepiaEffect;
		GreyscaleEffect* greyscaleEffect;
		ColorCorrectEffect* colorCorrectEffect;
		BloomEffect* bloomEffect;
		FilmGrainEffect* filmGrainEffect;
		PixelatedEffect* pixelatedEffect;

		EffectHandler* effectHandler = new EffectHandler();
		effectHandler->m_activeEffect = 0;

		bool showOnlyOneDeferredLightSource = false;
		bool drawPositionBufferOnly = false;
		bool drawNormalBufferOnly = false;
		bool drawColourBufferOnly = false;
		bool showLightAccumulationBuffer = false;

		LUT3D warmCube("cubes/WarmCorrection.cube");
		LUT3D coolCube("cubes/CoolCorrection.cube");
		LUT3D customCube("cubes/CustomCorrection.cube");
		LUT3D neutralCube("cubes/Neutral.cube");
		
		// We'll add some ImGui controls to control our shader
		BackendHandler::imGuiCallbacks.push_back([&]() {
			/*if (ImGui::Button("Scene with only one deferred light source"))
			{
				showOnlyOneDeferredLightSource = true;
				drawPositionBufferOnly = false;
				drawNormalBufferOnly = false;
				drawColourBufferOnly = false;
				showLightAccumulationBuffer = false;
			}
			if (ImGui::Button("Draw the Position/Depth Buffer only"))
			{
				drawPositionBufferOnly = true;
				showOnlyOneDeferredLightSource = false;
				drawNormalBufferOnly = false;
				drawColourBufferOnly = false;
				showLightAccumulationBuffer = false;
			}
			if (ImGui::Button("Draw the Normal Buffer only"))
			{
				drawNormalBufferOnly = true;
				showOnlyOneDeferredLightSource = false;
				drawPositionBufferOnly = false;
				drawColourBufferOnly = false;
				showLightAccumulationBuffer = false;
			}
			if (ImGui::Button("Draw the Colour/Material buffer only"))
			{
				drawColourBufferOnly = true;
				showOnlyOneDeferredLightSource = false;
				drawPositionBufferOnly = false;
				drawNormalBufferOnly = false;
				showLightAccumulationBuffer = false;
			}
			if (ImGui::Button("Show light accumulation buffer"))
			{
				showLightAccumulationBuffer = true;
				showOnlyOneDeferredLightSource = false;
				drawPositionBufferOnly = false;
				drawNormalBufferOnly = false;
				drawColourBufferOnly = false;
			}*/
			ImGui::SliderInt("Chosen Effect", &effectHandler->m_activeEffect, 0, effects.size() - 1);

			if (effectHandler->m_activeEffect == 0)
			{
				ImGui::Text("Active Effect: Bloom Effect");

				BloomEffect* temp = (BloomEffect*)effects[effectHandler->m_activeEffect];
				float threshold = temp->GetThreshold();
				int passes = temp->GetPasses();

				if (ImGui::SliderFloat("Bloom Threshold", &threshold, 0.01f, 1.0f))
				{
					temp->SetThreshold(threshold);
				}

				if (ImGui::SliderInt("Blur Passes", &passes, 1, 10))
				{
					temp->SetPasses(passes);
				}
			}
			if (effectHandler->m_activeEffect == 1)
			{
				ImGui::Text("Active Effect: Film Grain Effect");

				FilmGrainEffect* temp = (FilmGrainEffect*)effects[effectHandler->m_activeEffect];
				float strength = temp->GetStrength();

				if (ImGui::SliderFloat("Strength", &strength, 0.0f, 20.0f))
				{
					filmGrainEffect->SetStrength(strength);
				}
			}
			if (effectHandler->m_activeEffect == 2)
			{
				ImGui::Text("Active Effect: Pixelation Effect");

				PixelatedEffect* temp = (PixelatedEffect*)effects[effectHandler->m_activeEffect];
				float pixelation = temp->GetPixels();

				if (ImGui::SliderFloat("Pixelation", &pixelation, 256.0f, 1024.0f))
				{
					temp->SetPixels(pixelation);
				}
			}
			if (effectHandler->m_activeEffect == 3)
			{
				ImGui::Text("Active Effect: Greyscale Effect");

				GreyscaleEffect* temp = (GreyscaleEffect*)effects[effectHandler->m_activeEffect];
			}
			if (effectHandler->m_activeEffect == 4)
			{
				ImGui::Text("Active Colour Correction: Sepia Effect");

				SepiaEffect* temp = (SepiaEffect*)effects[effectHandler->m_activeEffect];
			}
			if (effectHandler->m_activeEffect == 5)
			{
				ImGui::Text("Active Effect: Colour Correction Effect");

				ColorCorrectEffect* temp = (ColorCorrectEffect*)effects[effectHandler->m_activeEffect];
				if (ImGui::SliderInt("Colour Correction Effect", &activeColourGradingEffect, 0, 2))
				{
					if (activeColourGradingEffect == 0)
					{
						ImGui::Text("Active Colour Correction: Warm Colour Correction");
						effectHandler->m_activeEffect = 5;
						temp->SetLUT(warmCube);
					}
					if (activeColourGradingEffect == 1)
					{
						ImGui::Text("Active Colour Correction: Cool Colour Correction");
						effectHandler->m_activeEffect = 5;
						temp->SetLUT(coolCube);
					}
					if (activeColourGradingEffect == 2)
					{
						ImGui::Text("Active Colour Correction: Custom Colour Correction");
						effectHandler->m_activeEffect = 5;
						temp->SetLUT(customCube);
					}
				}
			}
			if (ImGui::CollapsingHeader("Light Level Lighting Settings"))
			{
				if (ImGui::DragFloat3("Light Direction/Position", glm::value_ptr(illumBuffer->GetSunRef()._lightDirection), 0.01f, -10.0f, 10.0f)) 
				{
				}
			}
			});

		#pragma endregion 

		// GL states
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LEQUAL); // New 

		///////////////////////////////////// Texture Loading //////////////////////////////////////////////////
		#pragma region Texture

		// Load some textures from files
		Texture2D::sptr diffuse = Texture2D::LoadFromFile("images/Stone_001_Diffuse.png");
		Texture2D::sptr diffuse2 = Texture2D::LoadFromFile("images/box.bmp");
		Texture2D::sptr specular = Texture2D::LoadFromFile("images/Stone_001_Specular.png");
		Texture2D::sptr reflectivity = Texture2D::LoadFromFile("images/box-reflections.bmp");

		//Specular Textures
		Texture2D::sptr nospecular = Texture2D::LoadFromFile("images/nospec.png");
		Texture2D::sptr darkspecular = Texture2D::LoadFromFile("images/DarkGrey.png");
		Texture2D::sptr offwhitespecular = Texture2D::LoadFromFile("images/offwhite.png");
		Texture2D::sptr grassTex = Texture2D::LoadFromFile("images/grass.jpg");

		//Colour Textures
		Texture2D::sptr redTexture = Texture2D::LoadFromFile("images/meteor.png");
		Texture2D::sptr blueTexture = Texture2D::LoadFromFile("images/Shuttle.png");

		// Load the cube map
		//TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/sample.jpg");
		TextureCubeMap::sptr environmentMap = TextureCubeMap::LoadFromImages("images/cubemaps/skybox/space.jpg"); 

		// Creating an empty texture
		Texture2DDescription desc = Texture2DDescription();  
		desc.Width = 1;
		desc.Height = 1;
		desc.Format = InternalFormat::RGB8;
		Texture2D::sptr texture2 = Texture2D::Create(desc);
		// Clear it with a white colour
		texture2->Clear();

		#pragma endregion
		//////////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////// Scene Generation //////////////////////////////////////////////////
		#pragma region Scene Generation
		
		// We need to tell our scene system what extra component types we want to support
		GameScene::RegisterComponentType<RendererComponent>();
		GameScene::RegisterComponentType<BehaviourBinding>();
		GameScene::RegisterComponentType<Camera>();

		// Create a scene, and set it to be the active scene in the application
		GameScene::sptr scene = GameScene::Create("test");
		Application::Instance().ActiveScene = scene;

		// We can create a group ahead of time to make iterating on the group faster
		entt::basic_group<entt::entity, entt::exclude_t<>, entt::get_t<Transform>, RendererComponent> renderGroup =
			scene->Registry().group<RendererComponent>(entt::get_t<Transform>());

		// Create a material and set some properties for it
		ShaderMaterial::sptr material0 = ShaderMaterial::Create();
		material0->Shader = gBufferShader;
		material0->Set("s_Diffuse", diffuse);
		material0->Set("s_Diffuse2", diffuse2);
		material0->Set("s_Specular", specular);
		material0->Set("u_Shininess", 8.0f);
		material0->Set("u_TextureMix", 0.0f);

		// Lego Block Materials
		ShaderMaterial::sptr redMat = ShaderMaterial::Create();
		redMat->Shader = gBufferShader;
		redMat->Set("s_Diffuse", redTexture);
		redMat->Set("s_Specular", offwhitespecular);
		redMat->Set("u_Shininess", 8.0f);
		redMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr blueMat = ShaderMaterial::Create();
		blueMat->Shader = gBufferShader;
		blueMat->Set("s_Diffuse", blueTexture);
		blueMat->Set("s_Specular", offwhitespecular);
		blueMat->Set("u_Shininess", 8.0f);
		blueMat->Set("u_TextureMix", 0.0f);

		ShaderMaterial::sptr offwhiteMat = ShaderMaterial::Create();
		offwhiteMat->Shader = gBufferShader;
		offwhiteMat->Set("s_Diffuse", grassTex);
		offwhiteMat->Set("s_Specular", offwhitespecular);
		offwhiteMat->Set("u_Shininess", 8.0f);
		offwhiteMat->Set("u_TextureMix", 0.0f);

		std::shared_ptr<PhysicsEngine> PhysicsEngineWorld;
		GameObject PhysicsWorld = scene->CreateEntity("Physics World");
		{
			PhysicsEngineWorld = BehaviourBinding::Bind<PhysicsEngine>(PhysicsWorld);
			PhysicsEngineWorld->SetEffectHandler(effectHandler);
		}

		// Create an object to be our camera
		GameObject cameraObject = scene->CreateEntity("Camera");
		{
			cameraObject.get<Transform>().SetLocalPosition(0, 0.0f, 5.0f).LookAt(glm::vec3(0, -3, 5.0f));
			// We'll make our camera a component of the camera object
			Camera& camera = cameraObject.emplace<Camera>();// Camera::Create();
			camera.SetPosition(glm::vec3(0, 0.0f, 5.0f));
			camera.SetUp(glm::vec3(0, 0, 1));
			camera.LookAt(glm::vec3(0, 0, 5.0f));
			camera.SetFovDegrees(90.0f); // Set an initial FOV
			camera.SetOrthoHeight(3.0f);
			BehaviourBinding::Bind<PlaneBehaviour>(cameraObject);
			std::shared_ptr<PhysicsObject> newObject = BehaviourBinding::Bind<PhysicsObject>(cameraObject);
			newObject->SetPositionAndRadius(glm::vec3(0, 3.0f, 5.0f), glm::vec3(0, -8.0f, -12.0f), 0.5f);
			newObject->SetIsPlayer(true);
			PhysicsEngineWorld->AddObject(newObject);
		}

		GameObject Plane = scene->CreateEntity("plane");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Shuttle.obj");
			Plane.emplace<RendererComponent>().SetMesh(vao).SetMaterial(blueMat);
			Plane.get<Transform>().SetLocalPosition(0.0f, -5.0f, -5.0f);
			Plane.get<Transform>().SetLocalRotation(0, 90, 0);
			Plane.get<Transform>().SetLocalScale(0.5f, 0.5f, 0.5f);
			Plane.get<Transform>().SetParent(cameraObject);
		}

		for (int i = 0; i < 50; i++)
		{
			int randNumX = 101;
			int randNumY = 11;
			while (randNumX > 100)
			{
				randNumX = 1 + std::rand() / ((RAND_MAX + 1u) / 100);
			}
			while (randNumY > 10)
			{
				randNumY = 1 + std::rand() / ((RAND_MAX + 1u) / 10);
			}
			GameObject Spikes = scene->CreateEntity("spikes" + i);
			{
				VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/Meteor.obj");
				Spikes.emplace<RendererComponent>().SetMesh(vao).SetMaterial(redMat);
				Spikes.get<Transform>().SetLocalPosition(randNumX - 50, -i - 20, 2);

				std::shared_ptr<PhysicsObject> newObject = BehaviourBinding::Bind<PhysicsObject>(Spikes);
				newObject->SetPositionAndRadius(glm::vec3(randNumX - 50, -i - 20, 2), glm::vec3(0), 0.5f);
				newObject->SetIsPlayer(false);
				PhysicsEngineWorld->AddObject(newObject);
				// Bind returns a smart pointer to the behaviour that was added
				auto pathing = BehaviourBinding::Bind<FollowPathBehaviour>(Spikes);
				// Set up a path for the object to follow
				pathing->Points.push_back({ randNumX - 50, -i - 20 + 100, 2 });
				pathing->Points.push_back({ randNumX - 50, -i - 20 + 101, 2 });
				pathing->Speed = 8.0f;
			}
		}

		GameObject BigFloor = scene->CreateEntity("big_floor");
		{
			VertexArrayObject::sptr vao = ObjLoader::LoadFromFile("models/BigFloor.obj");
			BigFloor.emplace<RendererComponent>().SetMesh(vao).SetMaterial(offwhiteMat);
			BigFloor.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
			BehaviourBinding::Bind<HideBigFloor>(BigFloor);
		}

		int width, height;
		glfwGetWindowSize(BackendHandler::window, &width, &height);

		GameObject gBufferObject = scene->CreateEntity("G Buffer");
		{
			gBuffer = &gBufferObject.emplace<GBuffer>();
			gBuffer->Init(width, height);
		}

		GameObject illumBufferObject = scene->CreateEntity("Illumination Buffer");
		{
			illumBuffer = &illumBufferObject.emplace<IlluminationBuffer>();
			illumBuffer->Init(width, height);
			illumBuffer->GetSunRef()._ambientPow = 0.3f;
		}

		int shadowWidth = 4096;
		int shadowHeight = 4096;

		GameObject shadowBufferObject = scene->CreateEntity("Shadow Buffer");
		{
			shadowBuffer = &shadowBufferObject.emplace<Framebuffer>();
			shadowBuffer->AddDepthTarget();
			shadowBuffer->Init(shadowWidth, shadowHeight);
		}

		GameObject framebufferObject = scene->CreateEntity("Basic Effect");
		{
			basicEffect = &framebufferObject.emplace<PostEffect>();
			basicEffect->Init(width, height);
		}

		GameObject bloomEffectObject = scene->CreateEntity("Bloom Effect");
		{
			bloomEffect = &bloomEffectObject.emplace<BloomEffect>();
			bloomEffect->Init(width, height);
		}
		effects.push_back(bloomEffect);

		GameObject filmGrainEffectObject = scene->CreateEntity("Film Grain Effect");
		{
			filmGrainEffect = &filmGrainEffectObject.emplace<FilmGrainEffect>();
			filmGrainEffect->Init(width, height);
		}
		effects.push_back(filmGrainEffect);

		GameObject pixelatedEffectObject = scene->CreateEntity("Pixelated Effect");
		{
			pixelatedEffect = &pixelatedEffectObject.emplace<PixelatedEffect>();
			pixelatedEffect->Init(width, height);
		}
		effects.push_back(pixelatedEffect);

		GameObject greyscaleEffectObject = scene->CreateEntity("Greyscale Effect");
		{
			greyscaleEffect = &greyscaleEffectObject.emplace<GreyscaleEffect>();
			greyscaleEffect->Init(width, height);
		}
		effects.push_back(greyscaleEffect);

		GameObject SepiaEffectObject = scene->CreateEntity("Sepia Effect");
		{
			sepiaEffect = &SepiaEffectObject.emplace<SepiaEffect>();
			sepiaEffect->Init(width, height);
		}
		effects.push_back(sepiaEffect);

		GameObject ColourCorrectionEffectObject = scene->CreateEntity("Colour Correction Effect");
		{
			colorCorrectEffect = &ColourCorrectionEffectObject.emplace<ColorCorrectEffect>();
			colorCorrectEffect->Init(width, height);
		}
		effects.push_back(colorCorrectEffect);

		#pragma endregion 
		//////////////////////////////////////////////////////////////////////////////////////////

		/////////////////////////////////// SKYBOX ///////////////////////////////////////////////

		// Load our shaders
		Shader::sptr skybox = std::make_shared<Shader>();
		skybox->LoadShaderPartFromFile("shaders/skybox-shader.vert.glsl", GL_VERTEX_SHADER);
		skybox->LoadShaderPartFromFile("shaders/skybox-shader.frag.glsl", GL_FRAGMENT_SHADER);
		skybox->Link();

		ShaderMaterial::sptr skyboxMat = ShaderMaterial::Create();
		skyboxMat->Shader = skybox;
		skyboxMat->Set("s_Environment", environmentMap);
		skyboxMat->Set("u_EnvironmentRotation", glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0))));
		skyboxMat->RenderLayer = 100;

		MeshBuilder<VertexPosNormTexCol> mesh;
		MeshFactory::AddIcoSphere(mesh, glm::vec3(0.0f), 1.0f);
		MeshFactory::InvertFaces(mesh);
		VertexArrayObject::sptr meshVao = mesh.Bake();

		GameObject skyboxObj = scene->CreateEntity("skybox");
		skyboxObj.get<Transform>().SetLocalPosition(0.0f, 0.0f, 0.0f);
		//skyboxObj.get_or_emplace<RendererComponent>().SetMesh(meshVao).SetMaterial(skyboxMat).SetCastShadow(false);

		////////////////////////////////////////////////////////////////////////////////////////


		// We'll use a vector to store all our key press events for now (this should probably be a behaviour eventually)
		std::vector<KeyPressWatcher> keyToggles;
		{
			// This is an example of a key press handling helper. Look at InputHelpers.h an .cpp to see
			// how this is implemented. Note that the ampersand here is capturing the variables within
			// the scope. If you wanted to do some method on the class, your best bet would be to give it a method and
			// use std::bind
			keyToggles.emplace_back(GLFW_KEY_T, [&]() { cameraObject.get<Camera>().ToggleOrtho(); });

			//Toggles drawing specific buffer
			keyToggles.emplace_back(GLFW_KEY_O, [&]() { drawGBuffer = !drawGBuffer; });
			keyToggles.emplace_back(GLFW_KEY_P, [&]() { drawIllumBuffer = !drawIllumBuffer; });
		}

		// Initialize our timing instance and grab a reference for our use
		Timing& time = Timing::Instance();
		time.LastFrame = glfwGetTime();

		///// Game loop /////
		while (!glfwWindowShouldClose(BackendHandler::window)) {
			glfwPollEvents();

			// Update the timing
			time.CurrentFrame = glfwGetTime();
			time.DeltaTime = static_cast<float>(time.CurrentFrame - time.LastFrame);

			time.DeltaTime = time.DeltaTime > 1.0f ? 1.0f : time.DeltaTime;

			// Update our FPS tracker data
			fpsBuffer[frameIx] = 1.0f / time.DeltaTime;
			frameIx++;
			if (frameIx >= 128)
				frameIx = 0;

			// We'll make sure our UI isn't focused before we start handling input for our game
			if (!ImGui::IsAnyWindowFocused()) {
				// We need to poll our key watchers so they can do their logic with the GLFW state
				// Note that since we want to make sure we don't copy our key handlers, we need a const
				// reference!
				for (const KeyPressWatcher& watcher : keyToggles) {
					watcher.Poll(BackendHandler::window);
				}
			}

			// Iterate over all the behaviour binding components
			scene->Registry().view<BehaviourBinding>().each([&](entt::entity entity, BehaviourBinding& binding) {
				// Iterate over all the behaviour scripts attached to the entity, and update them in sequence (if enabled)
				for (const auto& behaviour : binding.Behaviours) {
					if (behaviour->Enabled) {
						behaviour->Update(entt::handle(scene->Registry(), entity));
					}
				}
			});

			// Clear the screen
			basicEffect->Clear();
			greyscaleEffect->Clear();
			sepiaEffect->Clear();
			for (int i = 0; i < effects.size(); i++)
			{
				effects[i]->Clear();
			}
			shadowBuffer->Clear();
			gBuffer->Clear();
			illumBuffer->Clear();


			glClearColor(1.0f, 1.0f, 1.0f, 0.3f);
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Update all world matrices for this frame
			scene->Registry().view<Transform>().each([](entt::entity entity, Transform& t) {
				t.UpdateWorldMatrix();
			});

			// Grab out camera info from the camera object
			Transform& camTransform = cameraObject.get<Transform>();
			glm::mat4 view = glm::inverse(camTransform.LocalTransform());
			glm::mat4 projection = cameraObject.get<Camera>().GetProjection();
			glm::mat4 viewProjection = projection * view;

			//Set up light space matrix
			glm::mat4 lightProjectionMatrix = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, -200.0f, 200.0f);
			glm::mat4 lightViewMatrix = glm::lookAt(glm::vec3(-illumBuffer->GetSunRef()._lightDirection), glm::vec3(), glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 lightSpaceViewProj = lightProjectionMatrix * lightViewMatrix;

			//Set shadow stuff
			illumBuffer->SetLightSpaceViewProj(lightSpaceViewProj);
			glm::vec3 camPos = glm::inverse(view) * glm::vec4(0, 0, 0, 1);
			illumBuffer->SetCamPos(camPos);

			// Sort the renderers by shader and material, we will go for a minimizing context switches approach here,
			// but you could for instance sort front to back to optimize for fill rate if you have intensive fragment shaders
			renderGroup.sort<RendererComponent>([](const RendererComponent& l, const RendererComponent& r) {
				// Sort by render layer first, higher numbers get drawn last
				if (l.Material->RenderLayer < r.Material->RenderLayer) return true;
				if (l.Material->RenderLayer > r.Material->RenderLayer) return false;

				// Sort by shader pointer next (so materials using the same shader run sequentially where possible)
				if (l.Material->Shader < r.Material->Shader) return true;
				if (l.Material->Shader > r.Material->Shader) return false;

				// Sort by material pointer last (so we can minimize switching between materials)
				if (l.Material < r.Material) return true;
				if (l.Material > r.Material) return false;

				return false;
			});

			// Start by assuming no shader or material is applied
			Shader::sptr current = nullptr;
			ShaderMaterial::sptr currentMat = nullptr;

			glViewport(0, 0, shadowWidth, shadowHeight);
			shadowBuffer->Bind();

			renderGroup.each([&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// Render the mesh
				if (renderer.CastShadows)
				{
					BackendHandler::RenderVAO(simpleDepthShader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);
				}
			});

			shadowBuffer->Unbind();

			glfwGetWindowSize(BackendHandler::window, &width, &height);

			glViewport(0, 0, width, height);
			gBuffer->Bind();
			// Iterate over the render group components and draw them
			renderGroup.each([&](entt::entity e, RendererComponent& renderer, Transform& transform) {
				// If the shader has changed, set up it's uniforms
				if (current != renderer.Material->Shader) {
					current = renderer.Material->Shader;
					current->Bind();
					BackendHandler::SetupShaderForFrame(current, view, projection);
				}
				// If the material has changed, apply it
				if (currentMat != renderer.Material) {
					currentMat = renderer.Material;
					currentMat->Apply();
				}


				shadowBuffer->BindDepthAsTexture(30);
				// Render the mesh
				BackendHandler::RenderVAO(renderer.Material->Shader, renderer.Mesh, viewProjection, transform, lightSpaceViewProj);

				skybox->Bind();
				BackendHandler::SetupShaderForFrame(skybox, view, projection);
				skyboxMat->Apply();
				BackendHandler::RenderVAO(skybox, meshVao, viewProjection, skyboxObj.get<Transform>(), lightSpaceViewProj);
				skybox->UnBind();
			});

			gBuffer->Unbind();

			illumBuffer->BindBuffer(0);

			illumBuffer->UnbindBuffer();

			shadowBuffer->BindDepthAsTexture(30);

			illumBuffer->ApplyEffect(gBuffer);

			shadowBuffer->UnbindTexture(30);

			if (drawGBuffer)
			{
				gBuffer->DrawBuffersToScreen(0);
			}
			else if (drawIllumBuffer)
			{
				illumBuffer->DrawIllumBuffer();
			}
			else
			{
				illumBuffer->DrawToScreen();
			}

			effects[effectHandler->m_activeEffect]->ApplyEffect(illumBuffer);

			effects[effectHandler->m_activeEffect]->DrawToScreen();

			// Draw our ImGui content
			BackendHandler::RenderImGui();

			scene->Poll();
			glfwSwapBuffers(BackendHandler::window);
			time.LastFrame = time.CurrentFrame;
		}

		// Nullify scene so that we can release references
		Application::Instance().ActiveScene = nullptr;
		//Clean up the environment generator so we can release references
		EnvironmentGenerator::CleanUpPointers();
		BackendHandler::ShutdownImGui();
	}	



	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}