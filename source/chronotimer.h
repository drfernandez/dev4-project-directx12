#pragma once
#include <chrono>

class ChronoTimer
{
public:
	ChronoTimer();
	~ChronoTimer();

	void Restart();
	void Signal();
	float Delta();
	float TotalTime();

private:
	std::chrono::steady_clock::time_point now;
	std::chrono::steady_clock::time_point last_update;
	float delta_time;
	float total_time;
};

ChronoTimer::ChronoTimer()
{
	this->Restart();
}

ChronoTimer::~ChronoTimer()
{
}

inline void ChronoTimer::Restart()
{
	delta_time = 0.0f;
	total_time = 0.0f;
}

inline void ChronoTimer::Signal()
{
	now = std::chrono::steady_clock::now();
	delta_time = std::chrono::duration_cast<std::chrono::microseconds>(now - last_update).count() / 100000.0f;
	last_update = now;
	total_time += delta_time;
}

inline float ChronoTimer::Delta()
{
	return delta_time;
}

inline float ChronoTimer::TotalTime()
{
	return total_time;
}
