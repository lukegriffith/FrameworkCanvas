#include "Settings.h"
#include <fstream>
#include <cstdlib>

namespace {
	std::string envStr(const char* name, const std::string& fallback) {
		const char* v = std::getenv(name);
		return (v && *v) ? std::string(v) : fallback;
	}
	int envInt(const char* name, int fallback) {
		const char* v = std::getenv(name);
		return (v && *v) ? ofToInt(v) : fallback;
	}
}

Settings Settings::fromEnv() {
	Settings s;
	s.outputType  = envStr("OUTPUT_TYPE", s.outputType);
	s.width       = envInt("VIDEO_WIDTH", s.width);
	s.height      = envInt("VIDEO_HEIGHT", s.height);
	s.fps         = envInt("VIDEO_FPS", s.fps);
	s.duration    = envInt("VIDEO_DURATION", s.duration);
	s.renderStart = envInt("RENDER_START", s.renderStart);
	s.renderEnd   = envInt("RENDER_END", s.renderEnd);
	s.seed        = (uint32_t)envInt("SEED", (int)s.seed);
	s.paramsFile  = envStr("PARAMS_FILE", "");
	s.framesDir   = envStr("FRAMES_DIR", "");

	if (s.renderEnd < 0 || s.renderEnd >= s.totalFrames()) s.renderEnd = s.totalFrames() - 1;

	// std::ifstream, not ofFile: ofFile resolves relative paths against bin/data (see CLAUDE.md)
	s.params = ofJson::object();
	if (!s.paramsFile.empty()) {
		std::ifstream in(s.paramsFile);
		if (in) {
			try {
				in >> s.params;
			} catch (const std::exception& e) {
				ofLogError("Settings") << "bad PARAMS_FILE " << s.paramsFile << ": " << e.what();
				s.params = ofJson::object();
			}
		} else {
			ofLogError("Settings") << "cannot open PARAMS_FILE " << s.paramsFile;
		}
	}
	return s;
}
