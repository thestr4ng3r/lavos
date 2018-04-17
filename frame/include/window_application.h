
#ifndef LAVOS_FRAME_APPLICATION_H
#define LAVOS_FRAME_APPLICATION_H

#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <lavos/engine.h>
#include <lavos/renderer.h>
#include <lavos/swapchain.h>

namespace lavosframe
{

class WindowApplication
{
	private:
		GLFWwindow *window;
		static void OnWindowResized(GLFWwindow *window, int width, int height);

		lavos::Engine *engine;

		vk::SurfaceKHR surface;

		lavos::Swapchain *swapchain;

		vk::Semaphore image_available_semaphore;
		vk::Semaphore render_finished_semaphore;

		std::chrono::high_resolution_clock::time_point last_frame_time;
		float delta_time = 0.0f;

		virtual void InitWindow(int width, int height, std::string title);
		virtual void InitVulkan(bool enable_layers);

		virtual void OnWindowResized(int width, int height);


		void CreateEngine(bool enable_layers);

		void CreateSurface();

		void RecreateSwapchain();

		void Cleanup();

		void CreateSemaphores();

	public:
		WindowApplication(int width, int height, std::string title, bool enable_layers);
		~WindowApplication();

		void BeginFrame();
		void Update();
		void Render(lavos::Renderer *renderer);
		void EndFrame();

		GLFWwindow *GetWindow() const			{ return window; }
		float GetDeltaTime() const 				{ return delta_time; }

		lavos::Engine *GetEngine() const		{ return engine; }
		lavos::Swapchain *GetSwapchain() const 	{ return swapchain; }
};

}

#endif //LAVOS_FRAME_APPLICATION_H
