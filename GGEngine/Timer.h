#pragma once

#include <Windows.h>

class Timer
{
	private:
		INT64 frequency, startTime;
		INT64 benchmarkStartTime;
		float ticksPerMs, delta;
		float benchmarkResult;
		int fps, fpsCounter;
		unsigned long fpsStartTime;
	public:
		bool Init();
		void Update();
		int GetFPS();
		float GetDelta();
		void BenchmarkCodeStart();
		void BenchmarkCodeEnd();
		float GetBenchmarkResult();
};