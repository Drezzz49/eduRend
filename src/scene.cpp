
#include "Scene.h"
#include "QuadModel.h"
#include "OBJModel.h"
#include "../Cube.h"

Scene::Scene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	m_dxdevice(dxdevice),
	m_dxdevice_context(dxdevice_context),
	m_window_width(window_width),
	m_window_height(window_height)
{ }

void Scene::OnWindowResized(
	int new_width,
	int new_height)
{
	m_window_width = new_width;
	m_window_height = new_height;
}

OurTestScene::OurTestScene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	Scene(dxdevice, dxdevice_context, window_width, window_height)
{ 
	InitTransformationBuffer();
	// + init other CBuffers
	InitLightCameraBuffer();
	InitMaterialBuffer();
}

//
// Called once at initialization
///
void OurTestScene::Init()
{
	m_camera = new Camera(
		45.0f * fTO_RAD,		// field-of-view (radians)
		(float)m_window_width / m_window_height,	// aspect ratio
		1.0f,					// z-near plane (everything closer will be clipped/removed)
		500.0f);				// z-far plane (everything further will be clipped/removed)

	// Move camera to (0,0,5)
	m_camera->MoveTo({ 0, 0, 5 });

	// Create objects
	m_quad = new QuadModel(m_dxdevice, m_dxdevice_context);
	m_cube = new Cube(m_dxdevice, m_dxdevice_context);
	m_cube_earth = new Cube(m_dxdevice, m_dxdevice_context);
	m_cube_moon = new Cube(m_dxdevice, m_dxdevice_context);
	m_sponza = new OBJModel("assets/crytek-sponza/sponza.obj", m_dxdevice, m_dxdevice_context);
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void OurTestScene::Update(
	float dt,
	const InputHandler& input_handler)
{
	// Basic camera control
	if (input_handler.IsKeyPressed(Keys::Up) || input_handler.IsKeyPressed(Keys::W)) //fram
		m_camera->Move(m_camera->GetForward() * m_camera_velocity * dt);

	if (input_handler.IsKeyPressed(Keys::Down) || input_handler.IsKeyPressed(Keys::S))//bak
		m_camera->Move(-m_camera->GetForward() * m_camera_velocity * dt);

	if (input_handler.IsKeyPressed(Keys::Right) || input_handler.IsKeyPressed(Keys::D))//höger
		m_camera->Move(m_camera->GetRight() * m_camera_velocity * dt);

	if (input_handler.IsKeyPressed(Keys::Left) || input_handler.IsKeyPressed(Keys::A))//vänster
		m_camera->Move(-m_camera->GetRight() * m_camera_velocity * dt);

	if(input_handler.IsKeyPressed(Keys::Space))//upp
		m_camera->Move(m_camera->GetUp() * m_camera_velocity * dt);

	if(input_handler.IsKeyPressed(Keys::LCtrl)) //ner
		m_camera->Move(-m_camera->GetUp() * m_camera_velocity * dt);

	if(input_handler.IsKeyPressed(Keys::Esc)) //quit
		PostQuitMessage(0);

	// Mouse look
	long mousedx = input_handler.GetMouseDeltaX();
	long mousedy = input_handler.GetMouseDeltaY();
	float mouse_sensitivity = 0.002f;
	m_camera->Rotate(-mousedx * mouse_sensitivity, -mousedy * mouse_sensitivity);


	// Now set/update object transformations
	// This can be done using any sequence of transformation matrices,
	// but the T*R*S order is most common; i.e. scale, then rotate, and then translate.
	// If no transformation is desired, an identity matrix can be obtained 
	// via e.g. Mquad = linalg::mat4f_identity; 

	// Quad model-to-world transformation
	m_quad_transform = mat4f::translation(0, 0, 0) *			// No translation
		mat4f::rotation(-m_angle, 0.0f, 1.0f, 0.0f) *	// Rotate continuously around the y-axis
		mat4f::scaling(1.5, 1.5, 1.5);				// Scale uniformly to 150%



	//om man har rotation för translation roterar de runt föräldern
	//om man har translation för rotation roterar de runt sin egen axel
	//ordningen händer bakifrån och framåt

	m_cube_transform = //kuben "solen"
		mat4f::translation(0, 0, 0) *			//translation eller flyttning
		mat4f::rotation(-m_angle, 0.0f, 1.0f, 0.0f) *	// rotera med farten -m_angle, runt y-axeln
		mat4f::scaling(1.5, 1.5, 1.5);				// skala
	
	m_cube_earth_transform = // jorden
		mat4f::rotation(m_angle * 2.5f, 0.0f, 1.0f, 0.0f) *   //vi måste göra rotation först för att den ska rotera runt solen och inte sig själv, för matrisernas ordning spelar roll
		mat4f::translation(2.0f, 0.0f, 0.0f) *
		mat4f::scaling(0.7f, 0.7f, 0.7f);              

	m_cube_moon_transform =	// månen
		mat4f::rotation(-m_angle * 2.5f, 0.0f, 0.0f, 1.0f) *
		mat4f::translation(1.0f, 0.0f, 0.0f) *
		mat4f::scaling(0.4f, 0.4f, 0.4f);

	//nu är jorden och månen utplacerad runt origo, men roterar med m_angle * x runt origo och inte runt sin egen axel
	//vi flyttar sen rotationspunkten för jorden och månen till solen respektive jorden genom att multiplicera med föräldrarnas transform
	m_cube_earth_transform = m_cube_transform * m_cube_earth_transform;
	m_cube_moon_transform = m_cube_earth_transform * m_cube_moon_transform;





	// Sponza model-to-world transformation
	m_sponza_transform = mat4f::translation(0, -5, 0) *		 // Move down 5 units
		mat4f::rotation(fPI / 2, 0.0f, 1.0f, 0.0f) * // Rotate pi/2 radians (90 degrees) around y
		mat4f::scaling(0.05f);						 // The scene is quite large so scale it down to 5%

	// Increment the rotation angle.
	m_angle += m_angular_velocity * dt * 0.4;

	// Print fps
	m_fps_cooldown -= dt;
	if (m_fps_cooldown < 0.0)
	{
		std::cout << "fps " << (int)(1.0f / dt) << std::endl;
//		printf("fps %i\n", (int)(1.0f / dt));
		m_fps_cooldown = 2.0;
	}
}

//
// Called every frame, after update
//
void OurTestScene::Render()
{
	// Bind transformation_buffer to slot b0 of the VS
	m_dxdevice_context->VSSetConstantBuffers(0, 1, &m_transformation_buffer);
	m_dxdevice_context->PSSetConstantBuffers(0, 1, &m_light_buffer);
	m_dxdevice_context->PSSetConstantBuffers(1, 1, &m_material_buffer);	

	UpdateLightCameraBuffer(vec4f(m_light_position, 1.0f) , vec4f(m_camera->Position(), 1.0f)); //lägger till 1.0 för att göra vec3's till vec4


	// Obtain the matrices needed for rendering from the camera
	m_view_matrix = m_camera->WorldToViewMatrix(); //view-matrisen, som är den som gör att det ser ut som att kameran rör sig i scenen
	m_projection_matrix = m_camera->ProjectionMatrix();

	// Load matrices + the Quad's transformation to the device and render it
	UpdateTransformationBuffer(m_quad_transform, m_view_matrix, m_projection_matrix);
	//m_quad->Render();


	//ambient är hur skuggfärgen, färgen som materialet har i skugga (R, G, B, A)
	//diffuse är den färg som materialet har i ljus (R, G, B, A)
	//specular är den färg som materialet har i glans (R, G, B, A) där rbg är färgen på glansen och a är hur stark glansen är


	//KUBEN
	MaterialBuffer cubeMat;
	cubeMat.AmbientColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	cubeMat.DiffuseColor = { 0.0f, 0.0f, 1.0f, 1.0f }; //blå
	cubeMat.SpecularColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	UpdateMaterialBuffer(cubeMat);
	UpdateTransformationBuffer(m_cube_transform, m_view_matrix, m_projection_matrix);
	m_cube->Render();

	//JORDEN
	MaterialBuffer earthMat;
	earthMat.AmbientColor = { 0.0f, 0.0f, 0.1f, 1.0f }; 
	earthMat.DiffuseColor = { 0.0f, 1.0f, 0.0f, 1.0f }; //grön
	earthMat.SpecularColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	UpdateMaterialBuffer(earthMat);
	UpdateTransformationBuffer(m_cube_earth_transform, m_view_matrix, m_projection_matrix);
	m_cube_earth->Render();

	//MÅNE
	MaterialBuffer moonMat;
	moonMat.AmbientColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	moonMat.DiffuseColor = { 1.0f, 0.0f, 0.0f, 1.0f }; //röd
	moonMat.SpecularColor = { 0.2f, 0.2f, 0.2f, 1.0f }; 
	UpdateMaterialBuffer(moonMat);
	UpdateTransformationBuffer(m_cube_moon_transform, m_view_matrix, m_projection_matrix);
	m_cube_moon->Render();

	// SPONZA
	MaterialBuffer sponzaMat;
	sponzaMat.AmbientColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	sponzaMat.DiffuseColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	sponzaMat.SpecularColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	UpdateMaterialBuffer(sponzaMat);
	UpdateTransformationBuffer(m_sponza_transform, m_view_matrix, m_projection_matrix);
	m_sponza->Render();
}

void OurTestScene::Release()
{
	SAFE_DELETE(m_quad);
	SAFE_DELETE(m_sponza);
	SAFE_DELETE(m_camera);
	
	
	SAFE_DELETE(m_cube);
	SAFE_DELETE(m_cube_earth);
	SAFE_DELETE(m_cube_moon);

	SAFE_RELEASE(m_transformation_buffer);
	// + release other CBuffers

	SAFE_RELEASE(m_light_buffer);
	SAFE_RELEASE(m_material_buffer);
}

void OurTestScene::OnWindowResized(
	int new_width,
	int new_height)
{
	if (m_camera)
		m_camera->SetAspect(float(new_width) / new_height);

	Scene::OnWindowResized(new_width, new_height);
}

void OurTestScene::InitTransformationBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC matrixBufferDesc = { 0 };
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(TransformationBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	ASSERT(hr = m_dxdevice->CreateBuffer(&matrixBufferDesc, nullptr, &m_transformation_buffer));
}

void OurTestScene::InitLightCameraBuffer() //samma som InitTransformation buffer fast utbytta variabler för LightCameraBuffer
{
	HRESULT hr;
	D3D11_BUFFER_DESC LightCameraBufferDesc = { 0 };
	LightCameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	LightCameraBufferDesc.ByteWidth = sizeof(LightCameraBuffer);
	LightCameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	LightCameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	LightCameraBufferDesc.MiscFlags = 0;
	LightCameraBufferDesc.StructureByteStride = 0;
	ASSERT(hr = m_dxdevice->CreateBuffer(&LightCameraBufferDesc, nullptr, &m_light_buffer));
}

void OurTestScene::InitMaterialBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC MaterialBufferDesc = { 0 };
	MaterialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	MaterialBufferDesc.ByteWidth = sizeof(MaterialBuffer);
	MaterialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MaterialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	MaterialBufferDesc.MiscFlags = 0;
	MaterialBufferDesc.StructureByteStride = 0;
	ASSERT(hr = m_dxdevice->CreateBuffer(&MaterialBufferDesc, nullptr, &m_material_buffer));
}


void OurTestScene::UpdateTransformationBuffer(
	mat4f ModelToWorldMatrix,
	mat4f WorldToViewMatrix,
	mat4f ProjectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	m_dxdevice_context->Map(m_transformation_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	TransformationBuffer* matrixBuffer = (TransformationBuffer*)resource.pData;
	matrixBuffer->ModelToWorldMatrix = ModelToWorldMatrix;
	matrixBuffer->WorldToViewMatrix = WorldToViewMatrix;
	matrixBuffer->ProjectionMatrix = ProjectionMatrix;
	m_dxdevice_context->Unmap(m_transformation_buffer, 0);
}


void OurTestScene::UpdateLightCameraBuffer(
	linalg::vec4f LightPosition,
	linalg::vec4f CameraPosition)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	m_dxdevice_context->Map(m_light_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	LightCameraBuffer * lightCamBuffer = (LightCameraBuffer*)resource.pData;
	lightCamBuffer->CameraPosition = CameraPosition;
	lightCamBuffer->LightPosition = LightPosition;
	m_dxdevice_context->Unmap(m_light_buffer, 0);
}

void OurTestScene::UpdateMaterialBuffer(MaterialBuffer material)
{
	D3D11_MAPPED_SUBRESOURCE resource;
	m_dxdevice_context->Map(m_material_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	MaterialBuffer* materialBuffer = (MaterialBuffer*)resource.pData;
	materialBuffer->AmbientColor = material.AmbientColor;
	materialBuffer->DiffuseColor = material.DiffuseColor;
	materialBuffer->SpecularColor = material.SpecularColor;
	m_dxdevice_context->Unmap(m_material_buffer, 0);
}