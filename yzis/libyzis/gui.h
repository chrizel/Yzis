#ifndef GUI_H
#define GUI_H
#include "yz_events.h"

/**
 * This is the interface that all GUIs must implement
 * Once you created an instance of it after initializing your GUI
 * you will register it into the lib.
 * Then the lib will be able to talk with the GUIs using this itf :)
 */ 
class Gui {
	public:

		/**
		 * libyzis posts events to the GUI
		 */
		virtual void postEvent(yz_event) = 0;
};

#endif // GUI_H

