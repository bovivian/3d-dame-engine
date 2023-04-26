#include "Animation.h"
#include "LogManager.h"

extern LogManager * gLogManager;

Animation::Animation()
{
	scene = NULL;
}

Animation::~Animation()
{
	scene = NULL;
}

bool Animation::Init(std::string filename, uint32_t numBones, bool loopAnim)
{
	this->numBones = numBones;
	loop = loopAnim;
	isFinished = false;

	runTime = 0.0f;
	speed = 0.001f;

	scene = importer.ReadFile(filename, aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder);

	if (!scene)
	{
		gLogManager->AddMessage("ERROR: Reading animation file! ASSIMP INFO: ");
		gLogManager->AddMessage(importer.GetErrorString());
		return false;
	}

	boneTransforms.resize(numBones);
	boneTransformsGLM.resize(numBones);

	globalInverseTransform = scene->mRootNode->mTransformation;
	globalInverseTransform.Inverse();

	return true;
}

void Animation::SetAnimationSpeed(float speed)
{
	this->speed = speed;
}

void Animation::ResetAnimation()
{
	runTime = 0.0f;
	isFinished = false;
}

void Animation::Update(float time, std::vector<aiMatrix4x4>& boneOffsets, std::map<std::string, uint32_t>& boneMapping)
{
	if(!isFinished)
		runTime += time * speed;

	float ticksPerSecond = (float)(scene->mAnimations[0]->mTicksPerSecond != 0 ? scene->mAnimations[0]->mTicksPerSecond : 25.0f);
	float timeInTicks = runTime * ticksPerSecond;

	if (timeInTicks > scene->mAnimations[0]->mDuration && !loop)
	{
		isFinished = true;
		runTime -= time * speed;
	}

	float animationTime = fmod(timeInTicks, (float)scene->mAnimations[0]->mDuration);

	aiMatrix4x4 identity = aiMatrix4x4();
	ReadNodeHierarchy(animationTime, scene->mRootNode, identity, boneOffsets, boneMapping);

	for (uint32_t i = 0; i < boneTransforms.size(); i++)
		boneTransformsGLM[i] = glm::transpose(glm::make_mat4(&boneTransforms[i].a1));

	if (runTime > scene->mAnimations[0]->mDuration * ticksPerSecond)
		runTime = 0.0f;
}

std::vector<glm::mat4>& Animation::GetBoneTransforms()
{
	return boneTransformsGLM;
}

bool Animation::IsFinished()
{
	return isFinished;
}

void Animation::ReadNodeHierarchy(float animTime, const aiNode * node, const aiMatrix4x4 & parentTransform,
	std::vector<aiMatrix4x4>& boneOffsets, std::map<std::string, uint32_t>& boneMapping)
{
	std::string nodeName(node->mName.data);

	aiMatrix4x4 nodeTransform(node->mTransformation);

	const aiNodeAnim * nodeAnim = FindNodeAnim(nodeName);
	if (nodeAnim)
	{
		aiMatrix4x4 matScale = InterpolateScale(animTime, nodeAnim);
		aiMatrix4x4 matRotation = InterpolateRotation(animTime, nodeAnim);
		aiMatrix4x4 matTranslation = InterpolateTranslation(animTime, nodeAnim);

		nodeTransform = matTranslation * matRotation * matScale;
	}

	aiMatrix4x4 globalTransform = parentTransform * nodeTransform;

	if (boneMapping.find(nodeName) != boneMapping.end())
	{
		uint32_t boneIndex = boneMapping[nodeName];
		boneTransforms[boneIndex] = globalInverseTransform * globalTransform * boneOffsets[boneIndex];
	}
	
	for (uint32_t i = 0; i < node->mNumChildren; i++)
		ReadNodeHierarchy(animTime, node->mChildren[i], globalTransform, boneOffsets, boneMapping);
}

const aiNodeAnim * Animation::FindNodeAnim(std::string nodeName)
{
	aiAnimation * anim = scene->mAnimations[0];
	for (uint32_t i = 0; i < anim->mNumChannels; i++)
	{
		const aiNodeAnim* nodeAnim = anim->mChannels[i];
		if (std::string(nodeAnim->mNodeName.data) == nodeName)
			return nodeAnim;
	}

	return NULL;
}

aiMatrix4x4 Animation::InterpolateTranslation(float time, const aiNodeAnim * nodeAnim)
{
	aiVector3D translation;

	if (nodeAnim->mNumPositionKeys == 1)
		translation = nodeAnim->mPositionKeys[0].mValue;
	else
	{
		uint32_t frameIndex = 0;
		for (uint32_t i = 0; i < nodeAnim->mNumPositionKeys - 1; i++)
		{
			if (time < (float)nodeAnim->mPositionKeys[i + 1].mTime)
			{
				frameIndex = i;
				break;
			}
		}

		aiVectorKey currentFrame = nodeAnim->mPositionKeys[frameIndex];
		aiVectorKey nextFrame = nodeAnim->mPositionKeys[(frameIndex + 1) % nodeAnim->mNumPositionKeys];

		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

		const aiVector3D& start = currentFrame.mValue;
		const aiVector3D& end = nextFrame.mValue;

		translation = (start + delta * (end - start));
	}

	aiMatrix4x4 mat;
	aiMatrix4x4::Translation(translation, mat);
	return mat;
}

aiMatrix4x4 Animation::InterpolateRotation(float time, const aiNodeAnim * nodeAnim)
{
	aiQuaternion rotation;

	if (nodeAnim->mNumRotationKeys == 1)
	{
		rotation = nodeAnim->mRotationKeys[0].mValue;
	}
	else
	{
		uint32_t frameIndex = 0;
		for (uint32_t i = 0; i < nodeAnim->mNumRotationKeys - 1; i++)
		{
			if (time < (float)nodeAnim->mRotationKeys[i + 1].mTime)
			{
				frameIndex = i;
				break;
			}
		}

		aiQuatKey currentFrame = nodeAnim->mRotationKeys[frameIndex];
		aiQuatKey nextFrame = nodeAnim->mRotationKeys[(frameIndex + 1) % nodeAnim->mNumRotationKeys];

		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

		const aiQuaternion& start = currentFrame.mValue;
		const aiQuaternion& end = nextFrame.mValue;

		aiQuaternion::Interpolate(rotation, start, end, delta);
		rotation.Normalize();
	}

	aiMatrix4x4 mat(rotation.GetMatrix());
	return mat;
}

aiMatrix4x4 Animation::InterpolateScale(float time, const aiNodeAnim * nodeAnim)
{
	aiVector3D scale;

	if (nodeAnim->mNumScalingKeys == 1)
	{
		scale = nodeAnim->mScalingKeys[0].mValue;
	}
	else
	{
		uint32_t frameIndex = 0;
		for (uint32_t i = 0; i < nodeAnim->mNumScalingKeys - 1; i++)
		{
			if (time < (float)nodeAnim->mScalingKeys[i + 1].mTime)
			{
				frameIndex = i;
				break;
			}
		}

		aiVectorKey currentFrame = nodeAnim->mScalingKeys[frameIndex];
		aiVectorKey nextFrame = nodeAnim->mScalingKeys[(frameIndex + 1) % nodeAnim->mNumScalingKeys];

		float delta = (time - (float)currentFrame.mTime) / (float)(nextFrame.mTime - currentFrame.mTime);

		const aiVector3D& start = currentFrame.mValue;
		const aiVector3D& end = nextFrame.mValue;

		scale = (start + delta * (end - start));
	}

	aiMatrix4x4 mat;
	aiMatrix4x4::Scaling(scale, mat);
	return mat;
}
