#include <fstream>
#include <random>
#define NOMINMAX

#include "SceneManager.h"
#include "StdInc.h"
#include "LogManager.h"
#include "Input.h"
#include "Timer.h"
#include "TextureManager.h"
#include "BufferManager.h"
#include "DBconnectivity.h"

TextureManager * gTextureManager;
BufferManager * gBufferManager;
DBconnectivity gConnectDB;

extern LogManager * gLogManager;
extern Input * gInput;
extern Timer * gTimer;
extern GUIManager * gGUIManager;

#define DISTANSE_TO_PICKUP_ITEMS 2.0

SceneManager::SceneManager()
{
	lastGameState = currentGameState = GAME_STATE_UNINITIALIZED;
	physics = NULL;
	camera = NULL;
	sunlight = NULL;
	pipelineManager = NULL;

	initCommandBuffer = NULL;
	deferredCommandBuffer = NULL;

	renderDummy = NULL;
	skydome = NULL;
	shadowMaps = NULL;
	frustumCuller = NULL;

	idleAnim = NULL;
	walkAnim = NULL;
	fallAnim = NULL;
	jumpAnim = NULL;
	runAnim = NULL;

	player = NULL;

	splashScreen = NULL;
	showSplashScreen = true;
	gProgramRunning = true;
}

SceneManager::~SceneManager()
{
	SAFE_DELETE(player);

	SAFE_DELETE(runAnim);
	SAFE_DELETE(jumpAnim);
	SAFE_DELETE(fallAnim);
	SAFE_DELETE(walkAnim);
	SAFE_DELETE(idleAnim);

	SAFE_DELETE(sunlight);
	SAFE_DELETE(camera);
	SAFE_DELETE(frustumCuller);
	SAFE_DELETE(timeCycle);
	SAFE_DELETE(physics);
	SAFE_DELETE(gBufferManager);
	SAFE_DELETE(gTextureManager);
}

bool SceneManager::Init(VulkanInterface * vulkan)
{
	if (gConnectDB.logIn())
		loggedin = true;

	// Init resource managers
	gTextureManager = new TextureManager();
	gBufferManager = new BufferManager();

	// Init command buffers
	initCommandBuffer = new VulkanCommandBuffer();
	if (!initCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (initCommandBuffer)");
		return false;
	}

	for(size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		VulkanCommandBuffer * cmdBuffer = new VulkanCommandBuffer();
		if (!cmdBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
		{
			gLogManager->AddMessage("ERROR: Failed to create a command buffer! (renderCommandBuffers)");
			return false;
		}
		renderCommandBuffers.push_back(cmdBuffer);
	}

	deferredCommandBuffer = new VulkanCommandBuffer();
	if (!deferredCommandBuffer->Init(vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool(), true))
	{
		gLogManager->AddMessage("ERROR: Failed to create a command buffer! (deferredCommandBuffer)");
		return false;
	}

	// Init pipeline manager
	pipelineManager = new PipelineManager();
	if (!pipelineManager->InitUIPipelines(vulkan))
	{
		gLogManager->AddMessage("ERROR: Failed to init UI pipelines!");
		return false;
	}

	// Init GUI manager
	guiManager = new GUIManager();
	if (!guiManager->Init(vulkan, initCommandBuffer, gConnectDB.getInventoryDB()))
		return false;

	// Camera setup
	camera = new Camera();
	camera->Init();
	camera->SetPosition(0.0f, 0.0f, 0.0f);
	camera->SetDirection(0.0f, 0.5f, 0.5f);
	camera->SetCameraState(CAMERA_STATE_ORBIT_PLAYER);

	// Splash screen logo
	splashScreen = new GUIElement();
	if (!splashScreen->Init(vulkan, initCommandBuffer, "data/textures/logo.rct"))
	{
		gLogManager->AddMessage("ERROR: Failed to init splash screen logo!");
		return false;
	}
	splashScreen->SetDimensions(0.3f, 0.4f);
	splashScreen->SetPosition(0.35f, 0.3f);

	splashScreenTimer = new GameplayTimer();

	ChangeGameState(GAME_STATE_SPLASH_SCREEN);

	return true;
}

bool SceneManager::LoadGame(VulkanInterface * vulkan)
{
	// Init shadow maps
	shadowMaps = new ShadowMaps();
	if (!shadowMaps->Init(vulkan, initCommandBuffer, camera))
	{
		gLogManager->AddMessage("ERROR: Failed to init shadow maps!");
		return false;
	}

	// Init light manager
	lightManager = new LightManager();
	if (!lightManager->Init(vulkan->GetVulkanDevice()))
	{
		gLogManager->AddMessage("ERROR: Failed to init light manager!");
		return false;
	}

	// Physics init
	physics = new Physics();
	physics->Init();

	// Init frustum culler
	frustumCuller = new FrustumCuller();

	// Light setup
	sunlight = new Sunlight();

	// Init test cubemap
	testCubemap = new Cubemap();
	if (!testCubemap->Init(vulkan->GetVulkanDevice(), initCommandBuffer, "data/cubemaps/testcubemap"))
	{
		gLogManager->AddMessage("ERROR: Failed to init cubemap!");
		return false;
	}

	if (!pipelineManager->InitGamePipelines(vulkan, shadowMaps))
	{
		gLogManager->AddMessage("ERROR: Failed to init game pipelines!");
		return false;
	}

	// Init render dummy
	renderDummy = new RenderDummy();
	if (!renderDummy->Init(vulkan, pipelineManager->GetDefault(), vulkan->GetPositionAttachment()->GetImageView(), vulkan->GetNormalAttachment()->GetImageView(),
		vulkan->GetAlbedoAttachment()->GetImageView(), vulkan->GetMaterialAttachment()->GetImageView(), vulkan->GetDepthAttachment()->GetImageView(),
		shadowMaps, lightManager, testCubemap->GetImageView()))
	{
		gLogManager->AddMessage("ERROR: Failed to init render dummy!");
		return false;
	}

	// Init skydome
	skydome = new Skydome();
	if (!skydome->Init(vulkan, pipelineManager->GetSkydome()))
	{
		gLogManager->AddMessage("ERROR: Failed to init skydome!");
		return false;
	}
	
	// Morgaet ne iz-za timecikla
	// Init timecycle
	timeCycle = new TimeCycle();
	if (!timeCycle->Init(skydome, sunlight))
	{
		gLogManager->AddMessage("ERROR: Failed to init timecycle!");
		return false;
	}
	timeCycle->SetTime(9, 0);
	timeCycle->SetWeather("sunny");

	// Load map files
	if (!LoadMapFile("data/testmap.map", vulkan))
		return false;

	if (!LoadItemsFile("data/itemList.txt", vulkan))
		return false;

	// Skinned models and animations
	male = new SkinnedModel();
	if (!male->Init("data/models/male.rcs", vulkan, initCommandBuffer))
	{
		gLogManager->AddMessage("ERROR: Failed to init male model!");
		return false;
	}

	// Animations
	idleAnim = new Animation();
	if (!idleAnim->Init("data/anims/idle.fbx", 52, true))
		return false;
	idleAnim->SetAnimationSpeed(0.0005f);

	walkAnim = new Animation();
	if (!walkAnim->Init("data/anims/walk.fbx", 52, true))
		return false;

	fallAnim = new Animation();
	if (!fallAnim->Init("data/anims/falling.fbx", 52, true))
		return false;
	fallAnim->SetAnimationSpeed(0.002f);

	jumpAnim = new Animation();
	if (!jumpAnim->Init("data/anims/jump.fbx", 52, false))
		return false;
	jumpAnim->SetAnimationSpeed(0.001f);

	runAnim = new Animation();
	if (!runAnim->Init("data/anims/run.fbx", 52, true))
		return false;


	male->SetAnimation(idleAnim);

	AnimationPack animPack;
	animPack.idleAnimation = idleAnim;
	animPack.walkAnimation = walkAnim;
	animPack.fallAnimation = fallAnim;
	animPack.jumpAnimation = jumpAnim;
	animPack.runAnimation = runAnim;

	player = new Player();
	player->Init(male, physics, animPack);
	player->SetPosition(3.0f, 2.0f, 0.0f);

	for (std::string itemName : gConnectDB.getInventoryDB())
	{
		Item * item = new Item();
		item->setItemName(itemName);
		item->setItemRarity(1);
		item->setOnMap(0);
		player->getInventory().add(item);
	}

	return true;
}

void SceneManager::Unload(VulkanInterface * vulkan)
{
	SAFE_UNLOAD(splashScreen, vulkan);

	SAFE_UNLOAD(male, vulkan);
	for (unsigned int i = 0; i < modelList.size(); i++)
		SAFE_UNLOAD(modelList[i], vulkan);
	for (unsigned int i = 0; i < itemModelList.size(); i++)
	{
		SAFE_UNLOAD(itemModelList[i], vulkan);
		itemList[i]->~Item();
	}
		
	SAFE_UNLOAD(skydome, vulkan);
	SAFE_UNLOAD(renderDummy, vulkan);

	SAFE_UNLOAD(testCubemap, vulkan->GetVulkanDevice());

	SAFE_UNLOAD(lightManager, vulkan->GetVulkanDevice());
	SAFE_UNLOAD(shadowMaps, vulkan);
	SAFE_UNLOAD(guiManager, vulkan);
	SAFE_UNLOAD(pipelineManager, vulkan);

	for (unsigned int i = 0; i < renderCommandBuffers.size(); i++)
		SAFE_UNLOAD(renderCommandBuffers[i], vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(deferredCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
	SAFE_UNLOAD(initCommandBuffer, vulkan->GetVulkanDevice(), vulkan->GetVulkanCommandPool());
}

int imageIndex = 5;

void SceneManager::Render(VulkanInterface * vulkan)
{
	// Splash screen
	if (showSplashScreen == true && splashScreenTimer)
			splashScreenTimer->StartTimer();
	
	
	if (showSplashScreen == true)
	{
		if (splashScreenTimer->TimePassed(1000))
		{
			ChangeGameState(GAME_STATE_LOADING);
			if (LoadGame(vulkan))
			{
				ChangeGameState(GAME_STATE_MAINMENU);
			}
			else
				THROW_ERROR();

			showSplashScreen = false;
			SAFE_DELETE(splashScreenTimer);
		}
	}

	if (currentGameState == GAME_STATE_MAINMENU)
	{	
		guiManager->ToggleMainMenuGUI(true);
		if (guiManager->CursorOnButton(guiManager->getStartbt(), guiManager->getCursor()) )
		{
			guiManager->SetCursorPosition();
			guiManager->ToggleMainMenuGUI(false);
			ChangeGameState(GAME_STATE_INGAME);
		}
		if (guiManager->CursorOnButton(guiManager->getExitbt(), guiManager->getCursor()))
		{
			gProgramRunning = false;
		}
	}

	if (currentGameState == GAME_STATE_INGAME)
	{
		guiManager->ToggleGUI(true);
		physics->Update();
		frustumCuller->BuildFrustum(camera->GetProjectionMatrix() * camera->GetViewMatrix());
		timeCycle->Update();

		glm::vec3 playerPos = player->GetPosition();

		if (playerPos.y < -50.0f)
			player->SetPosition(0.0f, 5.0f, 0.0f);

		if (gInput->WasKeyPressed(KEYBOARD_KEY_ESCAPE))
		{
			ChangeGameState(GAME_STATE_MAINMENU);
			guiManager->ToggleGUI(false);
			guiManager->ToggleInventoryGUI(false);
			guiManager->ToggleMainMenuGUI(true);
		}

		if (gInput->WasKeyPressed(KEYBOARD_KEY_P))
		{
			if(guiManager->getToggleInventoryGUI() == true)
				guiManager->ToggleInventoryGUI(false);
			else
				guiManager->ToggleInventoryGUI(true);
		}

		if (gInput->WasKeyPressed(KEYBOARD_KEY_G))
		{
			std::cout << player->getInventory().getNumOfItems() << "\n";
		}

		if (gInput->WasKeyPressed(KEYBOARD_KEY_O))
		{
			camera->SetCameraState(CAMERA_STATE_ORBIT_PLAYER);
			player->TogglePlayerInput(true);
		}

		if (gInput->WasKeyPressed(KEYBOARD_KEY_I))
		{
			camera->SetCameraState(CAMERA_STATE_FLY);
			player->TogglePlayerInput(false);
		}
		if (gInput->WasKeyPressed(KEYBOARD_KEY_Q))
		{
			char msg[64];
			sprintf(msg, "OBJ: %zu TXD: %zu BUF: %zu", modelList.size() + itemModelList.size(), gTextureManager->GetLoadedTexturesCount(),
				gBufferManager->GetLoadedBuffersCount());
			gLogManager->AddMessage(msg);
		}

		camera->HandleInput();

		player->Update(vulkan, camera);

		// Pich up the items
		if (gInput->WasKeyPressed(KEYBOARD_KEY_F))
		{
			glm::vec3 playerPosition = player->GetPosition();
			for (int i = 0; i < itemModelList.size(); i++)
			{
				if (itemList[i]->getOnMap() == 1) {
					glm::vec3 itemModelPosition = itemModelList[i]->GetPosition();
					if (itemModelPosition.x < (playerPosition.x + DISTANSE_TO_PICKUP_ITEMS) && itemModelPosition.x >(playerPosition.x - DISTANSE_TO_PICKUP_ITEMS))
					{
						if (itemModelPosition.y < (playerPosition.y + DISTANSE_TO_PICKUP_ITEMS) && itemModelPosition.y >(playerPosition.y - DISTANSE_TO_PICKUP_ITEMS))
						{
							if (itemModelPosition.z < (playerPosition.z + DISTANSE_TO_PICKUP_ITEMS) && itemModelPosition.z >(playerPosition.z - DISTANSE_TO_PICKUP_ITEMS))
							{
								if (player->getInventory().getNumOfItems() < player->getInventory().getCapacity())
								{
									gConnectDB.writeInventoryDB(itemList[i]->getItemName());
									itemList[i]->setOnMap(0);
									player->getInventory().add(itemList[i]);
									itemModelList[i]->SetPosition(-100.0, -100.0, -100.0);
								}
							}
						}
					}
				}
			}
		}

		inventorySize = player->getInventory().getNumOfItems();
		inv = inventoryList.size();
		if (inv < inventorySize)
		{
			inventoryList.clear();
			for (Item* item : player->getInventory().getAllItems())
			{
				inventoryList.push_back(item);
			}
			changed = true;
		}

		// Shadow pass
		shadowMaps->UpdatePartitions(vulkan, camera, sunlight);

		shadowMaps->BeginShadowPass(deferredCommandBuffer);

		float frustumCullData[SHADOW_CASCADE_COUNT];
		for (unsigned int i = 0; i < modelList.size(); i++)
		{
			// Check if model is inside shadow map bound
			if (shadowMaps->GetFrustumCuller(SHADOW_CASCADE_COUNT)->IsInsideFrustum(modelList[i]))
			{
				for (int j = 0; j < SHADOW_CASCADE_COUNT; j++)
				{
					if (shadowMaps->GetFrustumCuller(j)->IsInsideFrustum(modelList[i]))
						frustumCullData[j] = 1.0f;
					else
						frustumCullData[j] = 0.0f;
				}
				modelList[i]->SetFrustumCullData(frustumCullData);
				modelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetShadow(), NULL, shadowMaps);
			}
		}

		for (unsigned int i = 0; i < itemModelList.size(); i++)
		{
			if(itemList[i]->getOnMap())
			{
				// Check if model is inside shadow map bound
				if (shadowMaps->GetFrustumCuller(SHADOW_CASCADE_COUNT)->IsInsideFrustum(itemModelList[i]))
				{
					for (int j = 0; j < SHADOW_CASCADE_COUNT; j++)
					{
						if (shadowMaps->GetFrustumCuller(j)->IsInsideFrustum(itemModelList[i]))
							frustumCullData[j] = 1.0f;
						else
							frustumCullData[j] = 0.0f;
					}
					itemModelList[i]->SetFrustumCullData(frustumCullData);
					itemModelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetShadow(), NULL, shadowMaps);
				}
			}
		}

		player->GetModel()->Render(vulkan, deferredCommandBuffer, pipelineManager->GetShadowSkinned(), NULL, shadowMaps);

		shadowMaps->EndShadowPass(vulkan->GetVulkanDevice(), deferredCommandBuffer);

		// Deferred rendering
		vulkan->BeginSceneDeferred(deferredCommandBuffer);

		for (unsigned int i = 0; i < modelList.size(); i++)
			if (frustumCuller->IsInsideFrustum(modelList[i]))
				modelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetDeferred(), camera, NULL);

		for (unsigned int i = 0; i < itemModelList.size(); i++)
			if (itemList[i]->getOnMap())
			{
				if (frustumCuller->IsInsideFrustum(itemModelList[i]))
					itemModelList[i]->Render(vulkan, deferredCommandBuffer, pipelineManager->GetDeferred(), camera, NULL);
			}

		player->GetModel()->Render(vulkan, deferredCommandBuffer, pipelineManager->GetSkinned(), camera, NULL);

		vulkan->EndSceneDeferred(deferredCommandBuffer);
	}
	else
	{
		vulkan->BeginSceneDeferred(deferredCommandBuffer);
		vulkan->EndSceneDeferred(deferredCommandBuffer);
	}

	// Forward rendering
	for (size_t i = 0; i < vulkan->GetVulkanSwapchain()->GetSwapchainBufferCount(); i++)
	{
		vulkan->BeginSceneForward(renderCommandBuffers[i], (int)i);
		if (currentGameState == GAME_STATE_INGAME)
		{
			skydome->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetSkydome(), camera, (int)i);
			renderDummy->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetDefault(), camera->GetOrthoMatrix(), sunlight, imageIndex, camera, shadowMaps, (int)i);
		}
		else if (currentGameState == GAME_STATE_SPLASH_SCREEN) 
		{
			splashScreen->Render(vulkan, renderCommandBuffers[i], pipelineManager->GetCanvas(), camera, (int)i);
		}
		else if (currentGameState == GAME_STATE_MAINMENU) 
		{}
		else
		{
			gLogManager->AddMessage("ERROR: Unknown game state!");
			THROW_ERROR();
		}

		guiManager->Update(vulkan, renderCommandBuffers[i], pipelineManager->GetCanvas(), camera, (int)i, inventoryList, changed);
		changed = false;
		vulkan->EndSceneForward(renderCommandBuffers[i]);
	}
	
	// Present to screen
	vulkan->Present(renderCommandBuffers);
}

void SceneManager::SetProgramRunning(bool toggle) {
	gProgramRunning = toggle;
}

bool SceneManager::GetProgramRunning() {
	return gProgramRunning;
}


bool SceneManager::LoadMapFile(std::string filename, VulkanInterface * vulkan)
{
	// Static map loading
	std::string models[] = { "Map","adam" };
	for (int i = 0; i <= 1; i++)
	{
		std::string modelPath;
		std::string modelName = models[i];
		float posX = 0.0, posY = 0.0, posZ = 0.0;
		float rotX = 0.0, rotY = 0.0, rotZ = 0.0;
		float mass = 0.0;
		if (modelName == "Map")
		{
			posX = 0.0; posY = 0.0; posZ = 0.0;
			rotX = 0.0; rotY = 0.0; rotZ = 0.0;
			mass = 0.0;
		}
		else
		{
			posX = -50.0; posY = -50.0; posZ = 0.0;
			rotX = 0.0; rotY = 0.0; rotZ = 0.0;
			mass = 0.0;
		}


		modelName.append(".rcm");

		modelPath = "data/models/" + modelName;

		Model * model = new Model();
		if (!model->Init(modelPath, vulkan, initCommandBuffer, physics, mass))
		{
			gLogManager->AddMessage("ERROR: Failed to init model: " + modelName);
			return false;
		}

		model->SetPosition(posX, posY, posZ);
		model->SetRotation(rotX, rotY, rotZ);

		modelList.push_back(model);
	}

	return true;
}


bool SceneManager::LoadItemsFile(std::string filename, VulkanInterface * vulkan)
{

	srand(time(NULL));
	std::random_device rseed;
	std::mt19937 rng(rseed());
	std::uniform_int_distribution<int> dist(-30, 30);

	std::string items[] = { "box", "coin" };

	srand(time(NULL));

	int opt = rand() % 5 + 2;

	for (int i = 0; i < opt; i++)
	{
		std::string modelPath;
		std::string modelName;
		float posX, posY, posZ;
		float rotX = 0.0, rotY = 0.0, rotZ = 0.0;
		float mass;
		bool rarity;

		int tmp = rand() % (items->size()-1) + 0;

		modelName = items[tmp];
		mass = 1300.0;
		rarity = (rand() % 2 + 0);

		posX = dist(rng);

		posZ = dist(rng);
		posY = 15;

		Item * item = new Item();
		item->setItemName(modelName);
		item->setItemRarity(rarity);

		modelName.append(".rcm");

		modelPath = "data/items/models/" + modelName;

		Model * model = new Model();
		if (!model->Init(modelPath, vulkan, initCommandBuffer, physics, mass))
		{
			gLogManager->AddMessage("ERROR: Failed to init model: " + modelName);
			return false;
		}

		model->SetPosition(posX, posY, posZ);
		model->SetRotation(rotX, rotY, rotZ);

		itemModelList.push_back(model);
		itemList.push_back(item);
	}

	return true;
}

void SceneManager::ChangeGameState(GAME_STATE newGameState)
{
	lastGameState = currentGameState;
	currentGameState = newGameState;

	if (currentGameState == GAME_STATE_SPLASH_SCREEN)
		gLogManager->AddMessage("GAME STATE: SPLASH SCREEN");
	else if (currentGameState == GAME_STATE_LOADING)
		gLogManager->AddMessage("GAME STATE: LOADING");
	else if (currentGameState == GAME_STATE_MAINMENU)
		gLogManager->AddMessage("GAME STATE: MAIN MENU");
	else if (currentGameState == GAME_STATE_INGAME)
		gLogManager->AddMessage("GAME STATE: INGAME");
}

bool SceneManager::GetLoggedIn() 
{
	return loggedin;
}