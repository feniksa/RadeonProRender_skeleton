#include "Plugin.h"
#include "Context.h"
#include "FrameBuffer.h"
#include "Scene.h"
#include "Camera.h"
#include "Shape.h"
#include "LightPoint.h"
#include "ContextUtils.h"
#include "FrameBufferSysmem.h"
#include "math/mathutils.h"
#include "Environment.h" // <- generated by cmake
#include "StreamRedirector.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace rprf;

struct vertex
{
    rpr_float pos[3];
    rpr_float norm[3];
    rpr_float tex[2];
};


static const vertex cube_data[] =
{
   { -1.0f, 1.0f, -1.0f,    0.f, 1.f, 0.f,    0.f, 0.f },
   {  1.0f, 1.0f, -1.0f,    0.f, 1.f, 0.f,    1.f, 0.f },
   {  1.0f, 1.0f, 1.0f,     0.f, 1.f, 0.f,    1.f, 1.f },
   {  -1.0f, 1.0f, 1.0f,    0.f, 1.f, 0.f,    0.f, 1.f},

   {  -1.0f, -1.0f, -1.0f,  0.f, -1.f, 0.f,     0.f, 0.f },
   {  1.0f, -1.0f, -1.0f,   0.f, -1.f, 0.f,     1.f, 0.f },
   {  1.0f, -1.0f, 1.0f,    0.f, -1.f, 0.f,     1.f, 1.f },
   {  -1.0f, -1.0f, 1.0f,   0.f, -1.f, 0.f,     0.f, 1.f },

   {  -1.0f, -1.0f, 1.0f,   -1.f, 0.f, 0.f,     0.f, 0.f },
   {  -1.0f, -1.0f, -1.0f,  -1.f, 0.f, 0.f,     1.f, 0.f },
   {  -1.0f, 1.0f, -1.0f,   -1.f, 0.f, 0.f,     1.f, 1.f },
   {  -1.0f, 1.0f, 1.0f,    -1.f, 0.f, 0.f,     0.f, 1.f },

   {  1.0f, -1.0f, 1.0f,     1.f, 0.f, 0.f,    0.f, 0.f },
   {  1.0f, -1.0f, -1.0f,    1.f, 0.f, 0.f,    1.f, 0.f },
   {  1.0f, 1.0f, -1.0f,     1.f, 0.f, 0.f,    1.f, 1.f },
   {  1.0f, 1.0f, 1.0f,      1.f, 0.f, 0.f,    0.f, 1.f },

   {  -1.0f, -1.0f, -1.0f,    0.f, 0.f, -1.f ,    0.f, 0.f },
   {  1.0f, -1.0f, -1.0f,     0.f, 0.f, -1.f ,    1.f, 0.f },
   {  1.0f, 1.0f, -1.0f,      0.f, 0.f, -1.f,     1.f, 1.f },
   {  -1.0f, 1.0f, -1.0f,     0.f, 0.f, -1.f,     0.f, 1.f },

   {  -1.0f, -1.0f, 1.0f,   0.f, 0.f, 1.f,     0.f, 0.f },
   {  1.0f, -1.0f, 1.0f,    0.f, 0.f, 1.f,     1.f, 0.f },
   {  1.0f, 1.0f, 1.0f,     0.f, 0.f, 1.f,     1.f, 1.f },
   {  -1.0f, 1.0f, 1.0f,    0.f, 0.f, 1.f,     0.f, 1.f },
};

static const rpr_int indices[] =
{
    3,1,0,
    2,1,3,

    6,4,5,
    7,4,6,

    11,9,8,
    10,9,11,

    14,12,13,
    15,12,14,

    19,17,16,
    18,17,19,

    22,20,21,
    23,20,22
};

static const rpr_int num_face_vertices[] =
{
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};


struct TestNorthstar : public ::testing::Test
{
    void SetUp()
    {
		std::string dynlibname = "libNorthstar64.so";
		m_plugin = std::make_unique<Plugin>(dynlibname);
    }

    void TearDown()
    {
    	m_plugin.reset();
    }

protected:
	std::unique_ptr<Plugin> m_plugin;

	int GetCreationFlags(const gpu_list_t& devices)
	{
		rpr_creation_flags flags = 0;

		std::for_each(devices.begin(), devices.end(),
			[&flags](const gpu_list_t::value_type& p) {
				if (p.first != RPR_CREATION_FLAGS_ENABLE_CPU)
					flags |= p.first;
		});

		if (flags == 0)
			flags = RPR_CREATION_FLAGS_ENABLE_CPU;

		return flags;
	}
};

TEST_F(TestNorthstar, context_creation)
{
	ASSERT_TRUE(m_plugin);

	auto gpus = getAvailableDevices(*m_plugin, "");
 	printAvailableDevices(gpus, std::cout);

	Context context(*m_plugin, nullptr, GetCreationFlags(gpus));
}

TEST_F(TestNorthstar, scene_creation)
{
	ASSERT_TRUE(m_plugin);

	auto gpus = getAvailableDevices(*m_plugin, "");
	Context context(*m_plugin, nullptr, GetCreationFlags(gpus));

	Scene scene(context);
	context.setScene(scene);

	Camera camera(context);
	camera.lookAt(0, 5, 10, 0,0,0, 0,1,0);

	scene.setCamera(camera);

	Shape cube(context,
			reinterpret_cast<rpr_float const*>(&cube_data[0]),                         24, sizeof(vertex),
			reinterpret_cast<rpr_float const*>(&cube_data[0] + sizeof(rpr_float) * 3), 24, sizeof(vertex),
			reinterpret_cast<rpr_float const*>(&cube_data[0] + sizeof(rpr_float) * 6), 24, sizeof(vertex),
			static_cast<rpr_int const*>(indices), sizeof(rpr_int),
			static_cast<rpr_int const*>(indices), sizeof(rpr_int),
			static_cast<rpr_int const*>(indices), sizeof(rpr_int),
			num_face_vertices, 12);

	scene.attachShape(cube);

	rprf_math::matrix m = rprf_math::translation(rprf_math::float3(-2, 1, 0));
	cube.setTransform(m, true);

	LightPoint pointLight(context);
	pointLight.setRadianPower(150, 150, 150);

	scene.attachLight(pointLight);

	FrameBuffer frameBuffer(context, 800, 600);
	FrameBuffer frameBufferResolved(frameBuffer.clone());

	context.setAOV(frameBuffer);
	context.setParameter1u(RPR_CONTEXT_ITERATIONS, 60);

	context.render();
	context.resolve(&frameBuffer, &frameBufferResolved, true);

	frameBufferResolved.saveToFile(L"/tmp/out.png");
}