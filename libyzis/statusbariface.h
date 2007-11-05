#ifndef YZ_STATUSBAR_IFACE
#define YZ_STATUSBAR_IFACE

#include "yzismacros.h"

class QString;

/** Abstract class for a status bar.
 * This class represents a status bar, the way to export information
 * about the buffer and the view to the user. It should be implemented
 * by the GUI frontends.
 */

class YZIS_EXPORT YStatusBarIface
{
public:
    virtual ~YStatusBarIface()
    {};

    /** Used to display the mode we are currently in.
     */
    virtual void setMode(const QString& mode) = 0;

    /** Used to display current buffer's file name.
     */
    virtual void setFileName(const QString& fileName) = 0;

    /** Used to display current buffer's status information.
     * @param isNew the buffer is associated to a temporary file
     * @param isModified the buffer contains unsaved changes
     */
    virtual void setFileInfo(bool isNew, bool isModified) = 0;

    /** Used to display the line and column number where the main cursor is
     * and which part of the buffer is being dispayed.
     * @param bufferLine the line we are in the buffer.
     * @param bufferColumn the position in the buffer's line. First column is one.
     * @param screenColumn the number of screen cells from the left side of
     *                     the window. The leftmost column is one. It is equal
     *                     to bufferColumn when no tab characters are present
     *                     on the line.
     * @param percentage which part of the buffer is being displayed. It can be:
     *                   - a number from 0 to 100, which is the position of the top of
     *                     the screen within the buffer
     *                   - the string "Top", which means we can see the top of the buffer
     *                   - the string "Bot", which means we can see the bottom of the buffer
     *                   - the string "All", which means we see all the contents of the buffer
     */
    virtual void setLineInfo(int bufferLine, int bufferColumn, int screenColumn, QString percentage) = 0;

    /** Used to display informational messages to the user.
     */
    virtual void setMessage(const QString& message) = 0;
};

#endif

