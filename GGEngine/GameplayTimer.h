#pragma once

class GameplayTimer
{
	private:
		unsigned long timeStart;
		bool timerRunning;
	public:
		GameplayTimer();

		void StartTimer();
		void ResetTimer();
		bool TimePassed(float timeInMs);
		bool IsTimerRunning();
		float GetTimeProgress();
};