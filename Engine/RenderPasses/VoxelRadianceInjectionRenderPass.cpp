#include "VoxelRadianceInjectionRenderPass.h"

#include "Managers/ShaderManager.h"

#include "Renderer/Pipeline.h"

#include "Settings/GeneralSettings.h"

VoxelRadianceInjectionRenderPass::~VoxelRadianceInjectionRenderPass ()
{

}

void VoxelRadianceInjectionRenderPass::Init ()
{
	ShaderManager::Instance ()->AddComputeShader ("VOXEL_RADIANCE_INJECTION_PASS_COMPUTE_SHADER",
		"Assets/Shaders/Voxelize/voxelRadianceInjectionCompute.glsl");
}

RenderVolumeCollection* VoxelRadianceInjectionRenderPass::Execute (Scene* scene, Camera* camera, RenderVolumeCollection* rvc)
{
	if (!GeneralSettings::Instance ()->GetIntValue ("ContinousVoxelizationPass")) {
		return rvc;
	}

	if (!GeneralSettings::Instance ()->GetIntValue ("RadianceInjection")) {
		return rvc;
	}

	/*
	* Start radiance injecting pass
	*/

	StartRadianceInjectionPass ();

	/*
	* Radiance Injecting pass
	*/

	RadianceInjectPass (rvc);

	/*
	* End radiance injecting pass
	*/

	EndRadianceInjectionPass ();

	return rvc;
}

void VoxelRadianceInjectionRenderPass::StartRadianceInjectionPass ()
{
	Pipeline::SetShader (ShaderManager::Instance ()->GetShader ("VOXEL_RADIANCE_INJECTION_PASS_COMPUTE_SHADER"));
}

void VoxelRadianceInjectionRenderPass::RadianceInjectPass (RenderVolumeCollection* rvc)
{
	/*
	 * Bind render volumes for reading
	*/

	rvc->GetRenderVolume ("ShadowMapVolume")->BindForReading ();

	rvc->GetRenderVolume ("VoxelVolume")->BindForReading ();

	/*
	 * Send custom attributes of render volumes to pipeline
	*/

	Pipeline::SendCustomAttributes ("VOXEL_RADIANCE_INJECTION_PASS_COMPUTE_SHADER",
		rvc->GetRenderVolume ("ShadowMapVolume")->GetCustomAttributes ());

	Pipeline::SendCustomAttributes ("VOXEL_RADIANCE_INJECTION_PASS_COMPUTE_SHADER",
		rvc->GetRenderVolume ("VoxelVolume")->GetCustomAttributes ());

	/*
	 * Bind voxel volume for writing
	*/

	rvc->GetRenderVolume ("VoxelVolume")->BindForWriting ();

	/*
	 * Inject radiance
	*/

	int numWorkGroups = glm::ceil (512.0f / 4.0f);
	GL::DispatchCompute (numWorkGroups, numWorkGroups, numWorkGroups);
}

void VoxelRadianceInjectionRenderPass::EndRadianceInjectionPass ()
{
	/*
	* Make sure writing to image has finished before read
	*/

	GL::MemoryBarrier (GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}
