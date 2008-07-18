/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
*  Copyright (C) 2003-2004 Philippe Fremy <pfremy@freehackers.org>
*  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#ifndef YZ_BUFFER_H
#define YZ_BUFFER_H

#include <QStringList>
#include <QVector>
#include <QList>

#include "yzismacros.h"
#include "mark.h"

class YZUndoBuffer;
class YZAction;
class YDocMark;
class YCursor;
class YSwapFile;
class YLine;
class YView;
class YViewId;
class YInterval;

class YzisHighlighting;

typedef QVector<YLine*> YBufferData;

typedef QStringList YRawData;


/**
 * A buffer is the implementation of the content of a file.
 * 
 * A buffer can have multiple views. Every buffer is registered in a
 * @ref YSession
 * @short An abstract class to handle the content of a file.
 */
class YZIS_EXPORT YBuffer
{
public:
    //-------------------------------------------------------
    // ----------------- Constructor/Destructor
    //-------------------------------------------------------

    /**
     * Creates a new buffer
     */
    YBuffer();

    /**
     * Default destructor
     */
    virtual ~YBuffer();

    /** Return a string description of the buffer.
      *
      * The string description contains:
      * - the filename
      * - whether the buffer is currently modified.
      */
    QString toString() const;

    //-------------------------------------------------------
    // ----------------- Content Operations
    //-------------------------------------------------------

	/*
	 * Inserts data into the buffer at given position
	 * @param begin : position where to insert data
	 * @param data : data to insert, may start and end with YRawData_endline
	 * @returns position of char after inserted data
	 */
	YCursor insertRegion(const YCursor& begin, const YRawData& data);

	/*
	 * Remove data contained in the given interval
	 * @param bi : interval to remove
	 */
	void deleteRegion(const YInterval& bi);

	/*
	 * Shortcut for deleteRegion + insertRegion
	 * @param bi : interval to remove
	 * @param data : data to insert, may start and end with YRawData_endline
	 */
	YCursor replaceRegion(const YInterval& bi, const YRawData& data);

	YRawData dataRegion( const YInterval& bi ) const;


    //-------------------------------------------------------
    // ----------------- Character Operations
    //-------------------------------------------------------

    /**
     * Inserts a character into the buffer
     * @param pos : the position where to insert the character
     * @param c the character to add
     */
    void insertChar (YCursor pos, const QString& c);

    /**
     * Deletes a character in the buffer
     * @param pos : the position where to insert the character
     * @param count number of characters to delete
     */
    void delChar (YCursor pos, int count);

    //-------------------------------------------------------
    // ----------------- Line Operations
    //-------------------------------------------------------

    /**
     * Appends a new line at the end of file
     * @param l the line of text to be appended
     *
     * Note: the line is not supposed to contain '\n'
     */
    void appendLine(const QString &l);

    /**
     * Insert the text l in the current line
     * @param l the text to insert
     * @param line the line which is changed
     */
    void insertLine(const QString &l, int line);

    /**
     * Break a new line at the indicated position, moving rest of the line onto
     * a line of its own.
     * @param pos The position to add '\n' in.
     */
    void insertNewLine( YCursor pos);

    /**
     * Deletes the given line
     * @param line the line number to delete
     *
     * Note: the valid line numbers are between 0 and lineCount()-1
     */
    void deleteLine( int line );

    /**
     * Replaces the line at @param line with the given string @param l
     */
    void replaceLine( const QString& l, int line );

	/**
	 * Get a list of strings between two cursors
	 * @param from the origin cursor
	 * @param to the end cursor
	 * @return a list of strings
	 */
	QStringList getText(const YCursor from, const YCursor to) const;
	QStringList getText(const YInterval& i) const;


    /**
     * Finds the @ref YLine pointer for a line in the buffer
     * @param line the line to return
     * @param noHL if set to false, the highlighting of the line is initialised XXX (need proof-reading)
     * @return a YLine pointer or 0 if none
     *
     * Note: the valid line numbers are between 0 and lineCount()-1
     */
    YLine * yzline(int line, bool noHL = true);
    const YLine * yzline(int line) const;

    /**
     * Replaces the given regexp @arg what with the given string @arg with on the specified @arg line
     * Repeat the change on the line if @arg wholeline is true
     * @return true if a change was done
     */
    bool substitute( const QString& what, const QString& with, bool wholeline, int line );

    /**
     * Get the length of a line
     * @param line the line number
     * @return a int with the length of the line
     *
     * Note: the valid line numbers are between 0 and lineCount()-1
     */
    int getLineLength(int line) const;

    /**
     * Finds a line in the buffer
     * @param line the line to search for
     * @return a QString reference on the line or NULL
     *
     * Note: the valid line numbers are between 0 and lineCount()-1
     */
    const QString textline(int line) const;

    /**
     * Return the column of the first non-blank character in the line
     */
    int firstNonBlankChar( int line ) const;

    //-------------------------------------------------------
    // ----------------- Buffer content
    //-------------------------------------------------------

    /**
     * Return true if the buffer is empty
     */
    bool isEmpty() const;

    /**
     * Get the whole text of the buffer
     * @return a QString containing the texts
     */
    QString getWholeText() const;

    /**
     * Get the length of the entire buffer
     * @return an int with the length of the buffer
     */
    int getWholeTextLength() const;

    /**
     * Remove all text
     * @return void
     */
    void clearText();

    void loadText( QString* content );


    /**
     * Get the character at the given cursor position.
     */
    QChar getCharAt( const YCursor at ) const;

    /**
     * Get entire word at given cursor position. Currently behaves like '*' in vim
     */
    QString getWordAt( const YCursor at ) const;

    /**
     * Number of lines in the buffer
     * @return the number of lines
     *
     * Note that empty buffer always have one empty line.
     */
    int lineCount() const;

    //-------------------------------------------------------
    // --------------------- Cursors
    //-------------------------------------------------------

    /**
     * Returns a cursor at the beginning of the buffer
     */
    YCursor begin() const;

    /**
     * Returns a cursor at the end of the buffer
     */
    YCursor end() const;

    //-------------------------------------------------------
    // --------------------- File Operations
    //-------------------------------------------------------

    /**
     * Opens the file and fills the buffer with its content
     */
    void load(const QString& file = QString());

    /**
     * Save the buffer content into the current filename
     * @return whether or not the file was saved correctly
     */
    bool save();

     /**
     * Get the absolute filename of the buffer
     * @return the filename
     */
    const QString& fileName() const;

    /**
     * Get the filename of the buffer ( not the path )
     * @return the filename
     */
    const QString fileNameShort() const;

    /**
     * Changes the filename
     * @param _path the new filename ( and path )
     */
    void setPath( const QString& _path );

    /**
     * Called whenever the filename is changed
     */
    void filenameChanged();

    /**
     * Is this file a new file
     */
    bool fileIsNew() const;

    /**
     * Is the file modified
     */
    bool fileIsModified() const;

    /**
     * Change the modified flag of the file
     */
    void setChanged(bool v);

    void setEncoding( const QString& name );
    const QString& encoding() const;

    /**
     * Write all text for all buffers into swap file.  The
     * original file is no longer needed for recovery.
     */
    void preserve();

    /**
     * Open a blank new file
     */
    void openNewFile();

    //-------------------------------------------------------
    // -------------------------- View Operations
    //-------------------------------------------------------

    /**
     * Adds a new view to the buffer
     * @param v the view to be added
     */
    void addView (YView *v);

    /**
     * Removes a view from this buffer
     * @param v the view to be removed
     */
    void rmView (YView *v);

    /**
     * The list of view for this buffer
     * @return a QValuelist of pointers to the views
     */
    QList<YView*> views() const;

    /**
     * Find the first view of this buffer
     * Temporary function
     */
    YView* firstView() const;

    /**
     * Refresh all views
     */
    void updateAllViews();

    //-------------------------------------------------------
    // ------------ Sub-object accessors
    //-------------------------------------------------------

    YZUndoBuffer * undoBuffer() const;
    YZAction* action() const;
    YViewMarker* viewMarks() const;
    YDocMark* docMarks() const;
    YzisHighlighting* highlight() const;

    //-------------------------------------------------------
    // ------------ Highlighting
    //-------------------------------------------------------

    /**
     * Sets the highlighting mode for this buffer.
        *
        * This will also try to load the indent script for this type of file.
        *
     * @param mode the highlighting mode to use
     * @param warnGUI emit signal highlightingChanged to GUI so they can reload the view if necessary
     */
    void setHighLight(int mode, bool warnGUI = true);

    /** Set highlighting mode by name.
      *
      * The highlighting mode is looked up using YzisHlManager::nameFind(). If
      * found, setHighLight( mode, true ) is called.
      */
    void setHighLight( const QString& name );

	/*
	 * update highlight from given line
	 * @param line : line number to start the HL update
	 * @returns first line number not affected by the update
	 */
    int updateHL(int line);

    void initHL( int line );

    /**
     * Notify GUIs that HL changed
     */
    virtual void highlightingChanged();

    /**
     * Detects the correct syntax highlighting for the current file
     */
    void detectHighLight();

    void makeAttribs();

    //-------------------------------------------------------
    // ------------ Local Options Management
    //-------------------------------------------------------

    /**
     * Retrieve an int option
     */
    int getLocalIntegerOption( const QString& option ) const;

    /**
     * Retrieve a bool option
     */
    bool getLocalBooleanOption( const QString& option ) const;

    /**
     * Retrieve a string option
     */
    QString getLocalStringOption( const QString& option ) const;

    /**
     * Retrieve a qstringlist option
     */
    QStringList getLocalListOption( const QString& option ) const;

    //-------------------------------------------------------
    // ------------ Buffer State
    //-------------------------------------------------------

    enum BufferState
    {
        BufferActive,
        BufferHidden,
        BufferInactive,
    };

    void setState( BufferState state );
    BufferState getState() const;

    void saveYzisInfo( YView* view );

        /*
         * Checks if a file needs a recovery
         */
        bool checkRecover();

    //-------------------------------------------------------
    // ------------ Static
    //-------------------------------------------------------

    static QString tildeExpand( const QString& path );

    /** Parses a string containing filename and possibly line col information.
     *
     * Input can be: 
     * \li filename
     * \li filename:line
     * \li filename:line:col
     * 
     * If not NULL, the @p gotoPos is adjusted to the target line,col or (0,0)
     * if there is no (line,col) information.
     *
     * @param filename a string containing filename and maybe line,col
     * information
     * @param gotoPos a cursor that receives the line,col information if any
     * @return filename stripped from line,col information.
     */
    static QString parseFilename( const QString& filename, YCursor* gotoPos = NULL );


    /** Get the cursor initial position for a filename.
     *
     * If @a parseFilename is true, the filename is first parsed with
     * parseFilename() to look for format filename:line:col . 
     *
     * If there is no line/col information in the filename string, the
     * function looks into YzisInfo for the last position in that file.
     *
     * @param filename filename that may contain line:col information
     * @param parseFilename If true, use parseFilename() to look for line:col
     * information. If false, just look into YzisInfo file for last cursor
     * position.
     * @return a cursor containing line:col that was found (if any).
     */
    static YCursor getStartPosition( const QString& filename, bool parseFilename = true );

protected:
    /**
     * Sets the line @param line to @param l
     * @param line is between 0 and lineCount()-1
     * @param l may not contain '\n'
     */
    void setTextline( int line, const QString & l );

private:
    /**
     * This function is to be overridden by subclasses that have
     * extra work to do when the state is changed to BufferInactive
     */
    virtual void makeInactive()
    {}

    /**
     * This function is to be overridden by subclasses that have
     * extra work to do when the state is changed to HIDDEN
     */
    virtual void makeHidden()
    {}

    /**
     * This function is to be overridden by subclasses that have
     * extra work to do when the state is changed to BufferActive
     */
    virtual void makeActive()
    {}

    struct Private;
    Private *d;
};

#endif /*  YZ_BUFFER_H */

