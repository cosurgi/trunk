// 2015 © Janek Kozicki <cosurgi@gmail.com>
#pragma once

/*! @brief This class provides two modes of operation:
 *
 *  1. messageAllowed(waitTime)     ← only call this and nothing else.
 *     it is used only if you want to print some messages, but not too frequently - to avoid flooding
 *
 *  2. readWallClock()              ← first call this
 *     tooLong(waitTime)            ← then call this to determine if your algorithm is taking too much time
 *     it is used for doing non-critical calculations and making sure that they do not take too much time
 */

#include <time.h>

class TimeLimit
{
	public:
		TimeLimit():lastMessage(0),lastWallClock(0){};

		//! report message no more frequent than once per "waitTime" seconds.
		bool messageAllowed(double waitTime) {
			if ( (getClock() - lastMessage) > waitTime) {
				lastMessage=getClock();
				return true;
			} else {
				return false;
			};
		};

		//! prepare for doing calculations by calling readWallClock(), then use tooLong() to see if they took too much time
		void readWallClock() {
			lastWallClock=getClock();
		};
		bool tooLong(double waitTime) {
			return (getClock() - lastWallClock) > waitTime;
		};
	private:
		double lastMessage;
		double lastWallClock;
		double getClock() {
			timeval tp;
			gettimeofday(&tp,NULL);
			return tp.tv_sec+tp.tv_usec/1e6;
		}

};

