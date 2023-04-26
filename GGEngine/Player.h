#include "SkinnedModel.h"
#include "Physics.h"
#include "GameplayTimer.h"
#include "Inventory.h"

struct AnimationPack
{
	Animation * idleAnimation;
	Animation * walkAnimation;
	Animation * fallAnimation;
	Animation * jumpAnimation;
	Animation * runAnimation;
};

class Player
{
	private:
		std::string playerPublicKey;
		SkinnedModel * playerModel;

		Physics * physics;
		btConvexShape * capsuleShape;
		btRigidBody * playerBody;
		btVector3 walkDirection;
		btVector3 baseDirection;

		float walkSpeed;
		float runSpeed;

		float baseOrientation;
		float playerOrientation;
		float strafeOrientation;
		float cameraYaw;
		float cameraPitch;

		glm::mat4 worldMatrix;

		bool inputEnabled;
		bool playerMoving;
		bool isJumping;
		bool jumpTracking_FallingInitiated;
		bool velocityStopped;
		bool forwardKeyPressed;
		bool leftKeyPressed;
		bool rightKeyPressed;
		bool backwardsKeyPressed;
		bool runKeyPressed;

		AnimationPack animPack;
		Inventory inventory;

	private:
		btVector3 RotateVec3Quaternion(btVector3 vec, btQuaternion quat);
		float InterpolateRotation(float currentRotation, float targetRotation, float speed);
		int GetCircleQuarter(float rotation);
		void TrackJumpState();
	public:
		Player();
		~Player();

		void Init(SkinnedModel * model, Physics * physics, AnimationPack animPack);
		void SetPosition(float x, float y, float z);
		void Update(VulkanInterface * vulkan, Camera * camera);
		void TogglePlayerInput(bool toggle);
		void Jump();
		glm::vec3 GetPosition();
		bool IsFalling(float errorMargin = 1.0f);
		SkinnedModel * GetModel();
		Inventory& getInventory();
};