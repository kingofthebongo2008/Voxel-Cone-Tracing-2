#include "VoxelVolume.h"

#include "Pipeline.h"

#include "Wrappers/OpenGL/GL.h"

#include "Managers/ShaderManager.h"
#include "Systems/Window/Window.h"

#include "Settings/GeneralSettings.h"

VoxelVolume::VoxelVolume() :
	_volumeTexture(0),
	_volumeFbo(0),
	_volumeSize(0)
{
	/*
	* Voxelization shader init
	*/

	ShaderManager::Instance()->AddShader("VOXELIZATION_SHADER",
		"Assets/Shaders/Voxelize/voxelizeVertex.glsl",
		"Assets/Shaders/Voxelize/voxelizeFragment.glsl",
		"Assets/Shaders/Voxelize/voxelizeGeometry.glsl");
}

VoxelVolume::~VoxelVolume()
{
	Clear();
}

void VoxelVolume::Init(std::size_t volumeSize)
{
	/*
	* Clear current volume if needed
	*/

	Clear();

	/*
	* Keep new current volume size
	*/

	_volumeSize = volumeSize;

	/*
	* Create the 3D texture to keep the voxel volume
	*/

	GL::GenTextures (1, &_volumeTexture);
	GL::BindTexture (GL_TEXTURE_3D, _volumeTexture);
	GL::TexImage3D (GL_TEXTURE_3D, 0, GL_RGBA8, _volumeSize, _volumeSize, _volumeSize,
		0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	GL::TexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GL::TexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GL::TexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	GL::TexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GL::TexParameteri (GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	/*
	* Initialize mipmaps
	*/

	GL::GenerateMipmap (GL_TEXTURE_3D);

	/*
	* Create an fbo for clearing the 3D texture.
	*/

	GL::GenFramebuffers(1, &_volumeFbo);

	GL::BindFramebuffer (GL_FRAMEBUFFER, _volumeFbo);
	GL::FramebufferTexture (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _volumeTexture, 0);
	GL::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VoxelVolume::StartVoxelizationPass()
{
	/*
	* Render to window but mask out all color.
	*/

	GL::Disable(GL_CULL_FACE);

	GL::ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	GL::DepthMask(GL_FALSE);
	GL::Viewport(0, 0, _volumeSize, _volumeSize);

	/*
	* Lock voxelization shader for geomtry rendering
	*/

	Pipeline::LockShader(ShaderManager::Instance()->GetShader("VOXELIZATION_SHADER"));
}

void VoxelVolume::EndVoxelizationPass()
{
	// GL::MemoryBarrier(GL_ALL_BARRIER_BITS);

	/*
	* Clear settings
	*/

	GL::Viewport(0, 0, Window::GetWidth(), Window::GetHeight());
	GL::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	GL::DepthMask(GL_TRUE);

	GL::Enable(GL_CULL_FACE);

	/*
	* Unlock voxelization shader
	*/

	Pipeline::UnlockShader();
}

void VoxelVolume::BindForVoxelizationPass()
{
	GL::BindImageTexture(0, _volumeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
	GL::Uniform1i(0, 0);

	UpdateVoxelizationPipelineAttributes();
}

void VoxelVolume::ClearVoxels()
{
	GL::BindFramebuffer(GL_FRAMEBUFFER, _volumeFbo);
	GL::ClearColor(0, 0, 0, 0);
	GL::Clear(GL_COLOR_BUFFER_BIT);
	GL::BindFramebuffer(GL_FRAMEBUFFER, 0);
}

void VoxelVolume::UpdateBoundingBox(const glm::vec3& minVertex, const glm::vec3& maxVertex)
{
	_minVertex = minVertex;
	_maxVertex = maxVertex;

	float sizeX = _maxVertex.x - _minVertex.x;
	float sizeY = _maxVertex.y - _minVertex.y;
	float sizeZ = _maxVertex.z - _minVertex.z;

	float difX, difY, difZ;
	difX = difY = difZ = 0;

	if (sizeX >= sizeY && sizeX >= sizeZ) {
		difY = sizeX - sizeY;
		difZ = sizeX - sizeZ;
	}
	else if (sizeY >= sizeZ) {
		difX = sizeY - sizeX;
		difZ = sizeZ - sizeX;
	}
	else {
		difX = sizeZ - sizeX;
		difY = sizeZ - sizeY;
	}

	_minVertex -= glm::vec3(difX / 2.0f, difY / 2.0f, difZ / 2.0f);
	_maxVertex += glm::vec3(difX / 2.0f, difY / 2.0f, difZ / 2.0f);
}

void VoxelVolume::StartRayTracePass()
{
	/*
	* Load voxel ray trace shader
	*/

	ShaderManager::Instance()->AddShader("VOXEL_RAY_TRACE_SHADER",
		"Assets/Shaders/Voxelize/voxelRayTraceVertex.glsl",
		"Assets/Shaders/Voxelize/voxelRayTraceFragment.glsl",
		"Assets/Shaders/Voxelize/voxelRayTraceGeometry.glsl");

	/*
	* Use voxel ray trace shader
	*/

	Pipeline::SetShader(ShaderManager::Instance()->GetShader("VOXEL_RAY_TRACE_SHADER"));
}

void VoxelVolume::BindForRayTracePass()
{
	Pipeline::SendCamera(Camera::Main());
	Pipeline::ClearObjectTransform();
	Pipeline::UpdateMatrices(ShaderManager::Instance()->GetShader("VOXEL_RAY_TRACE_SHADER"));

	GL::ActiveTexture (GL_TEXTURE10);
	GL::BindTexture (GL_TEXTURE_3D, _volumeTexture);

	UpdateVoxelRayTracePipelineAttributes();
}

void VoxelVolume::StartConeTracePass()
{

}

void VoxelVolume::BindForConeTraceLightPass()
{
	GL::ActiveTexture (GL_TEXTURE10);
	GL::BindTexture (GL_TEXTURE_3D, _volumeTexture);

	GL::TexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GL::TexParameteri (GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void VoxelVolume::StartVoxelMipmapPass ()
{
	ShaderManager::Instance ()->AddComputeShader ("VOXEL_MIPMAP_COMPUTE_SHADER",
		"Assets/Shaders/Voxelize/voxelMipmapCompute.glsl");

	Pipeline::SetShader (ShaderManager::Instance ()->GetShader ("VOXEL_MIPMAP_COMPUTE_SHADER"));
}

void VoxelVolume::GenerateMipmaps()
{
	std::size_t dstMipRes = _volumeSize >> 1;
	Shader* computeShader = ShaderManager::Instance ()->GetShader ("VOXEL_MIPMAP_COMPUTE_SHADER");

	BindForRadianceInjectionPass ();

	for (int mipLevel = 0; mipLevel < MIP_MAP_LEVELS - 1; mipLevel++) {
		GL::Uniform1i (computeShader->GetUniformLocation ("SrcMipLevel"), mipLevel);
		GL::Uniform1i (computeShader->GetUniformLocation ("DstMipRes"), dstMipRes);

		GL::BindImageTexture (0, _volumeTexture, mipLevel + 1, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

		int numWorkGroups = glm::ceil (dstMipRes / 4.0);
		GL::DispatchCompute (numWorkGroups, numWorkGroups, numWorkGroups);

		GL::MemoryBarrier (GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		dstMipRes >>= 1;
	}
}

void VoxelVolume::EndVoxelMipmapPass ()
{
	/*
	* Make sure writing to image has finished before read
	*/

	GL::MemoryBarrier (GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void VoxelVolume::BindForMipmapPass ()
{
	GL::ActiveTexture (GL_TEXTURE0);
	GL::BindTexture (GL_TEXTURE_3D, _volumeTexture);
}

void VoxelVolume::StartVoxelRadianceInjectionPass ()
{
	ShaderManager::Instance ()->AddComputeShader ("VOXEL_RADIANCE_INJECTION_COMPUTE_SHADER",
		"Assets/Shaders/Voxelize/voxelRadianceInjectionCompute.glsl");

	Pipeline::SetShader (ShaderManager::Instance ()->GetShader ("VOXEL_RADIANCE_INJECTION_COMPUTE_SHADER"));
}

void VoxelVolume::BindForRadianceInjectionPass ()
{
	GL::ActiveTexture (GL_TEXTURE10);
	GL::BindTexture (GL_TEXTURE_3D, _volumeTexture);
}

void VoxelVolume::InjectRadiance ()
{
	BindForMipmapPass ();

	UpdateVoxelRadianceInjectionPipelineAttributes ();

	GL::BindImageTexture (0, _volumeTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	int numWorkGroups = glm::ceil (_volumeSize / 4.0);
	GL::DispatchCompute (numWorkGroups, numWorkGroups, numWorkGroups);
}

void VoxelVolume::EndVoxelRadianceInjectionPass ()
{
	/*
	* Make sure writing to image has finished before read
	*/

	GL::MemoryBarrier (GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void VoxelVolume::UpdateVoxelizationPipelineAttributes()
{
	std::vector<PipelineAttribute> attributes = GetVolumeAdditionalPipelineAttributes();

	Pipeline::SendCustomAttributes("VOXELIZATION_SHADER", attributes);
}

void VoxelVolume::UpdateVoxelRayTracePipelineAttributes()
{
	std::vector<PipelineAttribute> attributes;

	PipelineAttribute volumeTexture;
	PipelineAttribute minVertex;
	PipelineAttribute maxVertex;
	PipelineAttribute volumeSize;
	PipelineAttribute volumeMipmapLevel;

	volumeTexture.type = PipelineAttribute::AttrType::ATTR_1I;
	minVertex.type = PipelineAttribute::AttrType::ATTR_3F;
	maxVertex.type = PipelineAttribute::AttrType::ATTR_3F;
	volumeSize.type = PipelineAttribute::AttrType::ATTR_3I;
	volumeMipmapLevel.type = PipelineAttribute::AttrType::ATTR_1I;

	volumeTexture.name = "volumeTexture";
	minVertex.name = "minVertex";
	maxVertex.name = "maxVertex";
	volumeSize.name = "volumeSize";
	volumeMipmapLevel.name = "volumeMipmapLevel";

	volumeTexture.value.x = 10;
	minVertex.value = _minVertex;
	maxVertex.value = _maxVertex;
	volumeSize.value = glm::vec3((float)_volumeSize);
	volumeMipmapLevel.value.x = GeneralSettings::Instance ()->GetIntValue ("VoxelVolumeMipmapLevel");

	attributes.push_back (volumeTexture);
	attributes.push_back(minVertex);
	attributes.push_back(maxVertex);
	attributes.push_back(volumeSize);
	attributes.push_back(volumeMipmapLevel);

	Pipeline::SendCustomAttributes("VOXEL_RAY_TRACE_SHADER", attributes);
}

void VoxelVolume::UpdateVoxelMipmapPipelineAttributes ()
{
	std::vector<PipelineAttribute> attributes;

	//PipelineAttribute

	Pipeline::SendCustomAttributes ("VOXEL_MIPMAP_COMPUTE_SHADER", attributes);
}

void VoxelVolume::UpdateVoxelRadianceInjectionPipelineAttributes ()
{
	std::vector<PipelineAttribute> attributes;

	PipelineAttribute volumeTexture;
	PipelineAttribute minVertex;
	PipelineAttribute maxVertex;
	PipelineAttribute volumeSize;

	volumeTexture.type = PipelineAttribute::AttrType::ATTR_1I;
	minVertex.type = PipelineAttribute::AttrType::ATTR_3F;
	maxVertex.type = PipelineAttribute::AttrType::ATTR_3F;
	volumeSize.type = PipelineAttribute::AttrType::ATTR_3I;

	volumeTexture.name = "volumeTexture";
	minVertex.name = "minVertex";
	maxVertex.name = "maxVertex";
	volumeSize.name = "volumeSize";

	volumeTexture.value.x = 10;
	minVertex.value = _minVertex;
	maxVertex.value = _maxVertex;
	volumeSize.value = glm::vec3 ((float) _volumeSize);

	attributes.push_back (volumeTexture);
	attributes.push_back (minVertex);
	attributes.push_back (maxVertex);
	attributes.push_back (volumeSize);

	Pipeline::SendCustomAttributes ("VOXEL_RADIANCE_INJECTION_COMPUTE_SHADER", attributes);
}


//void VoxelVolume::UpdateVoxelRayTracePipelineAttributes()
//{
//	std::vector<PipelineAttribute> attributes;
//
//	std::vector<PipelineAttribute> mipmapAttributes = GetMipmapVolumesPipelineAttributes();
//	std::vector<PipelineAttribute> additionalAttributes = GetVolumeAdditionalPipelineAttributes();
//
//	attributes.insert(attributes.end(), mipmapAttributes.begin(), mipmapAttributes.end());
//	attributes.insert(attributes.end(), additionalAttributes.begin(), additionalAttributes.end());
//
//	PipelineAttribute volumeMipmapLevel;
//
//	volumeMipmapLevel.type = PipelineAttribute::AttrType::ATTR_1I;
//
//	volumeMipmapLevel.name = "volumeMipmapLevel";
//
//	volumeMipmapLevel.value.x = GeneralSettings::Instance()->GetIntValue("VoxelVolumeMipmapLevel");
//
//	attributes.push_back(volumeMipmapLevel);
//
//	Pipeline::SendCustomAttributes("VOXEL_RAY_TRACE_SHADER", attributes);
//}

std::vector<PipelineAttribute> VoxelVolume::GetVoxelConeTracePipelineAttributes()
{
	std::vector<PipelineAttribute> attributes;

	PipelineAttribute volumeTexture;
	PipelineAttribute minVertex;
	PipelineAttribute maxVertex;
	PipelineAttribute volumeSize;

	volumeTexture.type = PipelineAttribute::AttrType::ATTR_1I;
	minVertex.type = PipelineAttribute::AttrType::ATTR_3F;
	maxVertex.type = PipelineAttribute::AttrType::ATTR_3F;
	volumeSize.type = PipelineAttribute::AttrType::ATTR_3I;

	volumeTexture.name = "volumeTexture";
	minVertex.name = "minVertex";
	maxVertex.name = "maxVertex";
	volumeSize.name = "volumeSize";

	volumeTexture.value.x = 10;
	minVertex.value = _minVertex;
	maxVertex.value = _maxVertex;
	volumeSize.value = glm::vec3((float)_volumeSize);

	attributes.push_back (volumeTexture);
	attributes.push_back(minVertex);
	attributes.push_back(maxVertex);
	attributes.push_back(volumeSize);

	return attributes;
}

//std::vector<PipelineAttribute> VoxelVolume::GetVoxelConeTracePipelineAttributes()
//{
//	std::vector<PipelineAttribute> attributes = GetVolumeAdditionalPipelineAttributes();
//
//	std::vector<PipelineAttribute> mipmapAttributes = GetMipmapVolumesPipelineAttributes();
//	std::vector<PipelineAttribute> additionalAttributes = GetVolumeAdditionalPipelineAttributes();
//
//	attributes.insert(attributes.end(), mipmapAttributes.begin(), mipmapAttributes.end());
//	attributes.insert(attributes.end(), additionalAttributes.begin(), additionalAttributes.end());
//
//	return attributes;
//}

std::vector<PipelineAttribute> VoxelVolume::GetVolumeAdditionalPipelineAttributes()
{
	std::vector<PipelineAttribute> attributes;

	PipelineAttribute minPosition;
	PipelineAttribute maxPosition;
	PipelineAttribute volumeSize;

	minPosition.type = PipelineAttribute::AttrType::ATTR_3F;
	maxPosition.type = PipelineAttribute::AttrType::ATTR_3F;
	volumeSize.type = PipelineAttribute::AttrType::ATTR_3I;

	minPosition.name = "minPosition";
	maxPosition.name = "maxPosition";
	volumeSize.name = "volumeSize";

	minPosition.value = _minVertex;
	maxPosition.value = _maxVertex;
	volumeSize.value = glm::vec3((float)_volumeSize);

	attributes.push_back(minPosition);
	attributes.push_back(maxPosition);
	attributes.push_back(volumeSize);

	return attributes;
}

void VoxelVolume::Clear()
{
	GL::DeleteTextures(1, &_volumeTexture);
	GL::DeleteFramebuffers(1, &_volumeFbo);
}