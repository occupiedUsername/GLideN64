#include <string>
#include <streambuf>
#include <fstream>
#include "date.h"
#include "VI.h"
#include "Config.h"
#include "Performance.h"

Performance perf;

Performance::Performance()
	: m_vi(0)
	, m_frames(0)
	, m_fps(0)
	, m_vis(0)
	, m_enabled(false) {
}

void Performance::reset()
{
	m_vi = 0;
	m_frames = 0;
	m_fps = 0;
	m_vis = 0;
	m_enabled = (config.onScreenDisplay.fps | config.onScreenDisplay.vis | config.onScreenDisplay.percent) != 0;
	if (m_enabled)
		m_startTime = std::chrono::steady_clock::now();
}

f32 Performance::getFps() const
{
	return m_fps;
}

f32 Performance::getVIs() const
{
	return m_vis;
}

f32 Performance::getPercent() const
{
	const f32 scale = VI.PAL ? 0.5f : 0.6f;
	return m_vis / scale;
}

void Performance::increaseVICount()
{
	if (!m_enabled)
		return;
	m_vi++;
	const std::chrono::steady_clock::time_point curTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration time_span = curTime - m_startTime;
	const double elapsed = double(time_span.count()) * std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den;
	if (elapsed < 0.5)
		return;
	m_vis = (f32)(m_vi / elapsed);
	m_fps = (f32)(m_frames / elapsed);
	m_vi = 0;
	m_frames = 0;
	m_startTime = curTime;
}

void Performance::increaseFramesCount()
{
	if (!m_enabled)
		return;
	m_frames++;

	// opens a file ... is this done every time getFPS is called? Perhaps make it conditional on non-existance of file
	std::ofstream outfile;
	outfile.open("frame+temp-pairs.txt", std::ios_base::app);

	// appending stamp to above file -- "<< std::chrono::system_clock::now()" throws error. Internet says "operator<<" needs to be "overloaded"
	using namespace std::chrono;
	outfile << date::format("%F %T\n", time_point_cast<milliseconds>(system_clock::now()));

	// opening stream from CPU-temp file
	std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
	
	// creating string (str) consisting of all elements from f
	std::string str((std::istreambuf_iterator<char>(f)),
			 std::istreambuf_iterator<char>());

	// appending contents of str into outfile
	outfile << str << std::endl;
}
