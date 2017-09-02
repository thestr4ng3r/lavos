
#include <cassert>
#include <iostream>
#include "common.h"
#include "../../demos/triangle/triangle_application.h"

static android_app *app;


// Helper class to forward the cout/cerr output to logcat derived from:
// http://stackoverflow.com/questions/8870174/is-stdcout-usable-in-android-ndk
class AndroidBuffer : public std::streambuf
{
	public:
		AndroidBuffer(android_LogPriority priority)
		{
			priority_ = priority;
			this->setp(buffer_, buffer_ + kBufferSize - 1);
		}

	private:
		static const int32_t kBufferSize = 128;
		int32_t overflow(int32_t c)
		{
			if (c == traits_type::eof())
			{
				*this->pptr() = traits_type::to_char_type(c);
				this->sbumpc();
			}
			return this->sync() ? traits_type::eof() : traits_type::not_eof(c);
		}

		int32_t sync()
		{
			int32_t rc = 0;
			if (this->pbase() != this->pptr())
			{
				char writebuf[kBufferSize + 1];
				memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
				writebuf[this->pptr() - this->pbase()] = '\0';

				rc = __android_log_write(priority_, "std", writebuf) > 0;
				this->setp(buffer_, buffer_ + kBufferSize - 1);
			}
			return rc;
		}

		android_LogPriority priority_ = ANDROID_LOG_INFO;
	char buffer_[kBufferSize];
};


void AndroidSetApp(android_app *_app)
{
	app = _app;
}

ANativeWindow *AndroidGetApplicationWindow()
{
	assert(app != nullptr);
	return app->window;
}

void AndroidGetWindowSize(int32_t *width, int32_t *height)
{
	assert(app != nullptr);
	*width = ANativeWindow_getWidth(app->window);
	*height = ANativeWindow_getHeight(app->window);
}

std::vector<char> AndroidReadSPIRVShader(const std::string shader)
{
	assert(app != nullptr);

	std::string filename = "shader/" + shader + ".spv";

	AAsset *asset = AAssetManager_open(app->activity->assetManager, filename.c_str(), AASSET_MODE_BUFFER);

	if(!asset)
		throw std::runtime_error("failed to open asset for filename " + filename);

	std::vector<char> buffer(static_cast<unsigned long>(AAsset_getLength(asset)));
	const void *asset_buffer = AAsset_getBuffer(asset);

	if(asset_buffer)
		memcpy(buffer.data(), asset_buffer, buffer.size());
	else
	{
		AAsset_close(asset);
		throw std::runtime_error("failed to read asset for filename " + filename);
	}

	AAsset_close(asset);

	return buffer;
}

void Android_handle_cmd(android_app *app, int32_t cmd)
{
	switch (cmd)
	{
		case APP_CMD_INIT_WINDOW:
			sample_main();
			LOGI("\n");
			LOGI("=================================================");
			LOGI("          The sample ran successfully!!");
			LOGI("=================================================");
			LOGI("\n");
			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			break;
		default:
			LOGI("event not handled: %d", cmd);
	}
}


bool Android_process_command()
{
	assert(app != nullptr);
	int events;
	android_poll_source *source;
	// Poll all pending events.
	if (ALooper_pollAll(0, NULL, &events, (void **)&source) >= 0) {
		// Process each polled events
		if (source != NULL) source->process(app, source);
	}
	return app->destroyRequested;
}

extern "C" void android_main(struct android_app *state)
{
	AndroidSetApp(state);

	// Set the callback to process system events
	app->onAppCmd = Android_handle_cmd;

	// Forward cout/cerr to logcat.
	std::cout.rdbuf(new AndroidBuffer(ANDROID_LOG_INFO));
	std::cerr.rdbuf(new AndroidBuffer(ANDROID_LOG_ERROR));

	// Main loop
	do {
		Android_process_command();
	}  // Check if system requested to quit the application
	while (app->destroyRequested == 0);

	TriangleApplication demo_app;

	try
	{
		demo_app.Run();
	}
	catch(const std::runtime_error &e)
	{
		LOGE("%s\n", e.what());
	}
}