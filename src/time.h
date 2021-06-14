#ifndef TIME_H
#define TIME_H
#include "controller.h"

#define PRESCALER_VALUE 4
typedef bool (Controller::*CtrlCallback) (void);

class TimeModule {
	public:
		static TimeModule *instance;

		uint16_t hopInterval;
		uint32_t lastAnchorPoint;
		uint32_t lastInjectionPoint;
		uint32_t switchDelay;
		int masterSCA;
		int slaveSCA;

		bool injection;
		int window;

		CtrlCallback channelCallback;
		CtrlCallback injectionCallback;
		CtrlCallback masterCallback;
		Controller* controllerInstance;

		uint32_t masterTime;
		bool masterMode;

		TimeModule();
		void followBLEConnection(uint16_t hopInterval,int masterSCA, CtrlCallback channelCallback,CtrlCallback injectionCallback,CtrlCallback masterCallback, Controller *controllerInstance);
		void setBLEAnchorPoint(uint32_t timestamp);
		void setBLEHopInterval(uint16_t hopInterval);
		uint16_t getBLEHopInterval();
		uint32_t getLastBLEAnchorPoint();
		void exitBLEConnection();

		void startBLEInjection();
		void stopBLEInjection();


		void enterBLEMasterMode();
		void exitBLEMasterMode();
		void setBLEMasterTime(uint32_t time);

		uint32_t getTimestamp();
};
#endif
