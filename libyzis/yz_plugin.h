/**
 * $Id: yz_plugin.h,v 1.2 2003/04/25 12:45:30 mikmak Exp $
 */
#ifndef YZ_PLUGIN_H
#define YZ_PLUGIN_H

/**
 * The Plugin interface :)
 * currently used to add dynamically new commands
 * it will do some more stuff later
 */

class YZPlugin {
	public:
		YZPlugin();
		virtual ~YZPlugin();

		//that's a wrapper function to add your own defined command,
		//that's the entry point in your plugin
		//all your keystrokes must callback this function
		//if you find a better way , let me know :)
		virtual QString my_function( QStringList ) = 0;

};

#endif
