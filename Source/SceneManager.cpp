///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ================
// This file contains the implementation of the `SceneManager` class, which is 
// responsible for managing the preparation and rendering of 3D scenes. It 
// handles textures, materials, lighting configurations, and object rendering.
//
// AUTHOR: Brian Battersby
// INSTITUTION: Southern New Hampshire University (SNHU)
// COURSE: CS-330 Computational Graphics and Visualization
//
// INITIAL VERSION: November 1, 2023
// LAST REVISED: December 1, 2024
//
// RESPONSIBILITIES:
// - Load, bind, and manage textures in OpenGL.
// - Define materials and lighting properties for 3D objects.
// - Manage transformations and shader configurations.
// - Render complex 3D scenes using basic meshes.
//
// NOTE: This implementation leverages external libraries like `stb_image` for 
// texture loading and GLM for matrix and vector operations.
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <ctime>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
	m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	std::cout << "[DEBUG] Calling CreateGLTexture()..." << std::endl;
	bReturn = CreateGLTexture(
		"textures/Wood_table.png",
		"desk");
	bReturn = CreateGLTexture(
		"textures/lamp_body.jpg",
		"bronze");
	bReturn = CreateGLTexture(
		"textures/metal_head.jpg",
		"crome");
	bReturn = CreateGLTexture(
		"textures/rubber_holds.jpg",
		"rubber");
	bReturn = CreateGLTexture(
		"textures/book_cover.jpg",
		"cover");
	bReturn = CreateGLTexture(
		"textures/book_fabric.jpg",
		"fabric");
	bReturn = CreateGLTexture(
		"textures/fabric_black.jpg",
		"fabricB");
	bReturn = CreateGLTexture(
		"textures/clock_face.jpg",
		"clockF");
	bReturn = CreateGLTexture(
		"textures/ceiling.jpg",
		"ceilingT");
	bReturn = CreateGLTexture(
		"textures/planks.jpg",
		"planksW");
	bReturn = CreateGLTexture(
		"textures/marble.jpg",
		"marble_floor");

	if (!bReturn) {
		std::cout << "Failed to load 'desk' texture!" << std::endl;
	}
	BindGLTextures();
}
void SceneManager::SetupSceneLights()
{
	// allows the shader to use lighting
	m_pShaderManager->setBoolValue(g_UseLightingName, true);
	// sets the position of the camera
	m_pShaderManager->setVec3Value("viewPosition", glm::vec3(0.0f, -10.0f, 10.0f));

	 // Enable the directional light
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	// sets the directional light color and direction
	m_pShaderManager->setVec3Value("directionalLight.direction", glm::vec3(-0.3f, -1.0f, -0.3));
	// sets the color of the light
	m_pShaderManager->setVec3Value("directionalLight.ambient", glm::vec3(0.2f)); // Dim ambient light
	// sets the main color of the light
	m_pShaderManager->setVec3Value("directionalLight.diffuse", glm::vec3(0.6f));
	// bright highlights
	m_pShaderManager->setVec3Value("directionalLight.specular", glm::vec3(1.0f)); //shiny spot

	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
	// sets the position of the point light
	m_pShaderManager->setVec3Value("pointLights[0].position", glm::vec3(-5.0f, 6.5f, -5.0f));
	m_pShaderManager->setVec3Value("pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.5f));
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", glm::vec3(0.2f, 0.2f, 0.2f));
	m_pShaderManager->setVec3Value("pointLights[0].specular", glm::vec3(0.4f, 0.3f, 0.3f));

	// Turn on spotlight
	m_pShaderManager->setBoolValue("spotLight.bActive", true);

	// light at the tip of lamp head
	m_pShaderManager->setVec3Value("spotLight.position", glm::vec3(-2.2f, 6.5f, 2.5)); 

	// Pointed in the direction lamp head is facing
	m_pShaderManager->setVec3Value("spotLight.direction", glm::vec3(-0.7f, -1.5f, 1.0f)); // adjust as needed

	// Spotlight cutoff
	m_pShaderManager->setFloatValue("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	m_pShaderManager->setFloatValue("spotLight.outerCutOff", glm::cos(glm::radians(35.5f)));

	// Light color values
	m_pShaderManager->setVec3Value("spotLight.ambient", glm::vec3(0.001f));
	m_pShaderManager->setVec3Value("spotLight.diffuse", glm::vec3(4.0f, 4.4f, 4.0f));   // warm light
	m_pShaderManager->setVec3Value("spotLight.specular", glm::vec3(3.0f));

	// controls how far the light goes
	m_pShaderManager->setFloatValue("spotLight.constant", 1.0f);
	m_pShaderManager->setFloatValue("spotLight.linear", 0.09f);
	m_pShaderManager->setFloatValue("spotLight.quadratic", 0.032f);

}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	SetupSceneLights();
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene
	// define the materials for objects in the scene
	SceneManager::OBJECT_MATERIAL material;

	// Desk
	material.tag = "desk";
	material.diffuseColor = glm::vec3(0.8f, 0.5f, 0.2f);
	material.specularColor = glm::vec3(0.5f);
	material.shininess = 32.0f;
	m_objectMaterials.push_back(material);
	// Lamp
	material.tag = "lamp";
	material.diffuseColor = glm::vec3(0.8f);
	material.specularColor = glm::vec3(0.5f);
	material.shininess = 64.0f;
	m_objectMaterials.push_back(material);
	// Lamp Head
	material.tag = "lamp_head";
	material.diffuseColor = glm::vec3(0.5f);
	material.specularColor = glm::vec3(0.8f);
	material.shininess = 32.0f;
	m_objectMaterials.push_back(material);
	// Lamp Base
	material.tag = "lamp_base";
	material.diffuseColor = glm::vec3(0.7f);
	material.specularColor = glm::vec3(0.4f);
	material.shininess = 16.0f;
	m_objectMaterials.push_back(material);
	// hinges
	material.tag = "rubber";
	material.diffuseColor = glm::vec3(0.6f);
	material.specularColor = glm::vec3(0.3f);
	material.shininess = 16.0f;
	m_objectMaterials.push_back(material);
	// book cover
	material.tag = "cover";
	material.diffuseColor = glm::vec3(0.5f);              
	material.specularColor = glm::vec3(0.1f, 0.1f, 0.2f);   // low reflection
	material.shininess = 1.0f;                             
	m_objectMaterials.push_back(material);
	//book fabric
	material.tag = "fabric";
	material.diffuseColor = glm::vec3(0.5f);
	material.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);   // low reflection
	material.shininess = 1.0f;                             
	m_objectMaterials.push_back(material);

	material.tag = "fabricB";
	material.diffuseColor = glm::vec3(1.0f);
	material.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);   // low reflection
	material.shininess = 0.4f;                             // wide, soft highlight
	m_objectMaterials.push_back(material);
	// clock face
	material.tag = "clockF";
	material.diffuseColor = glm::vec3(1.0f);
	material.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);   // low reflection
	material.shininess = 0.4f;                             // wide, soft highlight
	m_objectMaterials.push_back(material);
	// floor
	material.tag = "marbleF";
	material.diffuseColor = glm::vec3(0.8f, 0.5f, 0.2f);
	material.specularColor = glm::vec3(1.0f);
	material.shininess = 64.0f;
	m_objectMaterials.push_back(material);
	// walls
	material.tag = "planksW";
	material.diffuseColor = glm::vec3(0.8f, 0.5f, 0.2f);
	material.specularColor = glm::vec3(0.5f);
	material.shininess = 0.5f;
	m_objectMaterials.push_back(material);
	// ceiling
	material.tag = "ceilingT";
	material.diffuseColor = glm::vec3(0.8f, 0.5f, 0.2f);
	material.specularColor = glm::vec3(0.5f);
	material.shininess = 5.0f;
	m_objectMaterials.push_back(material);

/*************************************************************
/***
/*** 
/***
****************************8*********************************/
	// load the textures into OpenGL memory
	LoadSceneTextures();
    
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	time_t now = time(0);
	tm timeinfo;
	localtime_s(&timeinfo, &now);
	

	float hourAngle = -((timeinfo.tm_hour % 12) + timeinfo.tm_min / 60.0f) * 30.0f;
	float minuteAngle = -timeinfo.tm_min * 6.0f;
	float secondAngle = -timeinfo.tm_sec * 6.0f;

	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	
	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	/****************************************************************/
	/***                                                          ***/
	/***                        Desk setup                        ***/
	/***                                                          ***/
	/****************************************************************/

	/****************************************************************/
	/*** desk (top)                                               ***/
	/****************************************************************/
	// set the color for the mesh
	SetShaderTexture("desk");
	SetShaderMaterial("desk");
	// set the UV scale for the texture mapping to 4x4 tiling
	m_pShaderManager->setVec2Value("UVscale", glm::vec2(4.0f, 4.0f));
	scaleXYZ = glm::vec3(25.0f, 0.5f, 12.0f);  // width, thickness, depth
	positionXYZ = glm::vec3(0.0f, -0.3f, 2.0f);  // lift it so top surface stays visible
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
	// Desk part 2
	scaleXYZ = glm::vec3(20.0f, 0.3f, 11.0f);  // width, thickness, depth
	positionXYZ = glm::vec3(0.0f, -0.3f, 2.0f);  
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
	
	glm::vec3 legScale = glm::vec3(0.5f, 5.0f, 0.5f);  // thin, tall leg
	float deskHeight = -0.3f;  
	float legOffsetX = 9.0f;  
	float legOffsetZ = 3.5f;   
	float legY = deskHeight - (legScale.y / 2.1f); 

	/****************************************************************/
	/*** desk legs                                                ***/
	/****************************************************************/
	// Front-left leg
	SetTransformations(legScale, 0, 0, 0, glm::vec3(-legOffsetX, legY, legOffsetZ));
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);  // dark metal or wood
	m_basicMeshes->DrawBoxMesh();

	// Front-right leg
	SetTransformations(legScale, 0, 0, 0, glm::vec3(legOffsetX, legY, legOffsetZ));
	m_basicMeshes->DrawBoxMesh();

	// Back-left leg
	SetTransformations(legScale, 0, 0, 0, glm::vec3(-legOffsetX, legY, -legOffsetZ));
	m_basicMeshes->DrawBoxMesh();

	// Back-right leg
	SetTransformations(legScale, 0, 0, 0, glm::vec3(legOffsetX, legY, -legOffsetZ));
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/***                                                          ***/
	/***                        Lamp setup                        ***/
	/***                                                          ***/
	/****************************************************************/

	/****************************************************************/
	/*** Lamp Base (bottom part)                                  ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(2.5f, 0.8f, 2.5f); // Wide and flat
	positionXYZ = glm::vec3(0.0f, 0.05f, 0.0f); // Touching the floor
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	// SetShaderColor(0.55f, 0.27f, 0.05f, 1.0f);
	SetShaderTexture("bronze");
	SetShaderMaterial("lamp_base");
	//SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh(); 

	/****************************************************************/
	/*** Bottom Vertical Stand (lamp pole)                        ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.3f, 6.6f, 0.3f); // Tall, thin cylinder
	positionXYZ = glm::vec3(0.0f, 0.7f, 0.0f); // On top of base
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	// SetShaderColor
	SetShaderTexture("bronze");
	SetShaderMaterial("lamp");
	//SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	/****************************************************************/
	/*** Top Vertical Stand (lamp pole)                           ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.3f, 2.0f, 0.3f); // Thin cylinder
	positionXYZ = glm::vec3(0.0f, 7.5f, 0.0f); // On top of bottom hinge
	SetTransformations(scaleXYZ, 75.0f, -45.0f, 0.0f, positionXYZ);
	// setShaderColor
	SetShaderTexture("bronze");
	SetShaderMaterial("lamp");
	//SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	/****************************************************************/
	/*** Bottom Hinge (sphere)                                    ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f);
	positionXYZ = glm::vec3(0.0f, 7.5f, 0.0f); 
	SetTransformations(scaleXYZ, 0.0f, 45.0f, 0.0f, positionXYZ);
	// SetShaderColor
	SetShaderTexture("rubber");
	SetShaderMaterial("rubber");
	//SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawSphereMesh(); 

	/****************************************************************/
	/*** Top Hinge (sphere)                                       ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f);
	positionXYZ = glm::vec3(-1.5f, 8.1f, 1.5f); 
	SetTransformations(scaleXYZ, 0.0f, 45.0f, 0.0f, positionXYZ);
	// SetShaderColor
	SetShaderTexture("rubber");
	SetShaderMaterial("rubber");
	//SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawSphereMesh();

	/****************************************************************/
	/*** Lamp Head (angled downward)                              ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(1.5f, 2.0f, 1.5f); // Taller taper lamp
	positionXYZ = glm::vec3(-2.2f, 6.5f, 2.5f); // Offset above connector
	SetTransformations(scaleXYZ, 35.0f, 145.0f, 0.0f, positionXYZ); // Tilted out and to the side
	// SetShaderColor
	SetShaderTexture("crome");
	SetShaderMaterial("lamp_head");
	//SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_basicMeshes->DrawConeMesh(); 
	/****************************************************************/
	/***                                                          ***/
	/***                        Book setup                        ***/
	/***                                                          ***/
	/****************************************************************/

	/****************************************************************/
	/*** Bottom Cover (flat rectangle)                            ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(4.7f, 0.2f, 3.8f);
	positionXYZ = glm::vec3(-3.0f, 0.1f, 6.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("fabricB");
	SetShaderMaterial("fabricB");
	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/*** Book Pages                                               ***/
	/****************************************************************/
	for (int i = 0; i < 8; ++i)
	{
		scaleXYZ = glm::vec3(4.65f, 0.04f, 3.7f);
		positionXYZ = glm::vec3(-3.03f, 0.21f + i * 0.045f, 6.0f);
		SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
		SetShaderColor(1.0f, 1.0f, 0.9f, 1.0f);  // Cream paper
		m_basicMeshes->DrawBoxMesh();
	}
	/****************************************************************/
	/*** Top Cover                                                ***/
	/****************************************************************/
	scaleXYZ = glm::vec3(4.7f, 0.2f, 3.8f);
	positionXYZ = glm::vec3(-3.0f, 0.64f, 6.0f);  // slightly higher
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("fabricB");
	SetShaderMaterial("fabricB");
	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/***  Page Crease Setup                                       ***/
	/****************************************************************/
	// Page crease strip (left side vertical edge)
	scaleXYZ = glm::vec3(0.2f, 0.74f, 3.8f);  // Very thin wall, same height as pages
	positionXYZ = glm::vec3(-5.45f, 0.370f, 6.0f);  // Push left along X axis
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("fabricB");
	SetShaderMaterial("fabricB");
	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f);
	m_basicMeshes->DrawBoxMesh();
	
	/****************************************************************/
	/***Book Cover Photo Setup                                    ***/
	/****************************************************************/
	SetShaderTexture("cover");
	m_pShaderManager->setVec2Value("UVscale", glm::vec2(1.0f, 1.0f));
	scaleXYZ = glm::vec3(1.90f, 0.01f, 2.35f);            
	positionXYZ = glm::vec3(-3.0f, 0.742f, 6.0f);         
	SetTransformations(scaleXYZ, 0, 90, 0, positionXYZ);
	m_basicMeshes->DrawPlaneMesh();

	/****************************************************************/
	/***                                                          ***/
	/***                        Room                              ***/
	/***                                                          ***/
	/****************************************************************/

	// Back wall
	scaleXYZ = glm::vec3(40.0f, 20.0f, 0.5f);       
	positionXYZ = glm::vec3(0.0f, 5.0f, -20.0f);  
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("planksW");
	SetShaderMaterial("planksW");
	m_basicMeshes->DrawBoxMesh();

	// left
	scaleXYZ = glm::vec3(0.5f, 20.0f, 40.0f);
	positionXYZ = glm::vec3(-20.0f, 5.0f, 0.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("planksW");
	SetShaderMaterial("planksW");
	m_basicMeshes->DrawBoxMesh();
	
	// right 
	scaleXYZ = glm::vec3(0.5f, 20.0f, 40.0f);
	positionXYZ = glm::vec3(20.0f, 5.0f, 0.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("planksW");
	SetShaderMaterial("planksW");
	m_basicMeshes->DrawBoxMesh();

	// floor
	scaleXYZ = glm::vec3(40.0f, 0.3f, 40.0f);
	positionXYZ = glm::vec3(0.0f, -5.0f, 0.0f);
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("marble_floor");
	SetShaderMaterial("marbleF");
	m_basicMeshes->DrawBoxMesh();

	// door
	scaleXYZ = glm::vec3(9.0f, 16.0f, 0.2f);  
	positionXYZ = glm::vec3(7.0f, 2.5f, -19.75f);  
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.3f, 0.2f, 0.1f, 1.0f);  // Dark wood
	m_basicMeshes->DrawBoxMesh();

	// ceiling
	scaleXYZ = glm::vec3(40.0f, 0.3f, 40.0f);
	positionXYZ = glm::vec3(0.0f, 15.0f, 0.0f);  
	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderTexture("ceilingT");
	SetShaderMaterial("ceilingT");
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	/***                                                          ***/
	/***                        Clock setup                       ***/
	/***                                                          ***/
	/****************************************************************/

	// Clock face
	SetShaderTexture("");
	SetShaderTexture("clockF");
	SetShaderMaterial("clockF");
	scaleXYZ = glm::vec3(1.0f, 0.1f, 1.0f);
	positionXYZ = glm::vec3(6.0f, 1.0f, 2.0f); // placed on the desk
	SetTransformations(scaleXYZ, 90.0f, 180.0f, 180.0f, positionXYZ);
	//SetShaderColor(0.9f, 0.9f, 0.9f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// Clock Base
	scaleXYZ = glm::vec3(0.4f, 1.0f, 0.4f);
	positionXYZ = glm::vec3(6.0f, 0.3f, 1.7f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderColor(0.3f, 0.3f, 0.3f, 1.0f); // dark gray
	m_basicMeshes->DrawBoxMesh();

	// Clock Stand
	glm::vec3 scaleBall = glm::vec3(0.4f); // uniform
	glm::vec3 ballPos = glm::vec3(6.0f, 1.0f, 1.65f);  // same X, slightly lower and behind clock
	SetTransformations(scaleBall, 90.0f, 0.0f, 0.0f, ballPos); // rotated to look like a wedge
	SetShaderColor(0.3f, 0.3f, 0.3f, 1.0f); // match base color
	m_basicMeshes->DrawSphereMesh();

	// Hour Hand
	scaleXYZ = glm::vec3(0.4f, 0.03f, 0.01f);  // long length
	// Position at the center of the clock face
	glm::vec3 hourBasePos = glm::vec3(6.0f, 1.05f, 2.008f);
	glm::mat4 hourModel = glm::mat4(1.0f);
	// Step 1: move to the clock center
	hourModel = glm::translate(hourModel, hourBasePos);
	// Step 2: rotate around Z axis to spin like a clock
	hourModel = glm::rotate(hourModel, glm::radians(hourAngle), glm::vec3(0.0f, 0.0f, 1.0f)); // spin like clock
	// Step 3: move hand forward along X by half its length
	hourModel = glm::translate(hourModel, glm::vec3(scaleXYZ.x * 0.5f, 0.0f, 0.0f));           // move hand to start at center
	// Step 4: scale to final hand shape
	hourModel = glm::scale(hourModel, scaleXYZ);
	m_pShaderManager->setMat4Value("model", hourModel);
	SetShaderColor(0.2f, 0.2f, 0.2f, 1.0f);
	m_basicMeshes->DrawBoxMesh();

	// Minute Hand
	scaleXYZ = glm::vec3(0.7f, 0.03f, 0.01f);  // medium length
	// Position at the center of the clock face	
	glm::mat4 minuteModel = glm::mat4(1.0f);
	glm::vec3 minuteBasePos = glm::vec3(6.0f, 1.05f, 2.01f);
	// Step 1: move to the clock center
	minuteModel = glm::translate(minuteModel, minuteBasePos);
	// Step 2: rotate around Z axis to spin like a clock
	minuteModel = glm::rotate(minuteModel, glm::radians(minuteAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	// Step 3: move hand forward along X by half its length
	minuteModel = glm::translate(minuteModel, glm::vec3(scaleXYZ.x * 0.5f, 0.0f, 0.0f));
	// Step 4: scale to final hand shape
	minuteModel = glm::scale(minuteModel, scaleXYZ);
	// Apply to shader
	m_pShaderManager->setMat4Value("model", minuteModel);
	SetShaderColor(0.1f, 0.1f, 0.1f, 1.0f);  // darker gray
	m_basicMeshes->DrawBoxMesh();

	// Second Hand
	scaleXYZ = glm::vec3(0.8f, 0.02f, 0.01f); // long along X
	// Position at the center of the clock face
	glm::vec3 basePos = glm::vec3(6.0f, 1.05f, 2.015f);
	glm::mat4 model = glm::mat4(1.0f);
	// Step 1: move to the clock center
	model = glm::translate(model, basePos);
	// Step 2: rotate around Z axis to spin like a clock
	model = glm::rotate(model, glm::radians(secondAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	// Step 3: move hand forward along X by half its length
	model = glm::translate(model, glm::vec3(scaleXYZ.x * 0.5f, 0.0f, 0.0f));
	// Step 4: scale to final hand shape
	model = glm::scale(model, scaleXYZ);
	// Apply to shader
	m_pShaderManager->setMat4Value("model", model);
	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh();
}