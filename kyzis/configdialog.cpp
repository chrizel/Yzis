/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>

#include <kfontrequester.h>
#include <kcolorbutton.h>

#include "debug.h"
#include "yzis.h"

#include "configdialog.h"


KYZisConfigDialog::KYZisConfigDialog( QWidget* parent, const char* name, KConfigSkeleton* config, DialogType dialogType )
	: KConfigDialog( parent, name, config, dialogType ) {

	config->readConfig( );

	setShowIconsInTreeList( true );

	setupPages();

}

KYZisConfigDialog::~KYZisConfigDialog( ) {
}

void KYZisConfigDialog::setupPages() {

	/**
	 * Appearance
	 */
	QWidget* pageAppearance = new QWidget( this, "Appearance" );
	QGridLayout* layoutAppearance = new QGridLayout( pageAppearance, 2, 4, 0, 10 );

	// Font
	QLabel* label_Font = new QLabel( i18n("Text font"), pageAppearance );
	KFontRequester* kcfg_Font = new KFontRequester( pageAppearance, "kcfg_Font" );

	layoutAppearance->addWidget( label_Font, 0, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_Font, 0, 1 );

	// BG Color
	QLabel* label_colorBG = new QLabel( i18n("Background Color"), pageAppearance );
	KColorButton* kcfg_colorBG = new KColorButton( pageAppearance, "kcfg_colorBG" );

	layoutAppearance->addWidget( label_colorBG, 1, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_colorBG, 1, 1 );

	// FG Color
	QLabel* label_colorFG = new QLabel( i18n("Foreground Color"), pageAppearance );
	KColorButton* kcfg_colorFG = new KColorButton( pageAppearance, "kcfg_colorFG" );

	layoutAppearance->addWidget( label_colorFG, 2, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_colorFG, 2, 1 );

	layoutAppearance->setRowStretch( 3, 1 );


	addPage( pageAppearance, i18n("Appearance"), "colorize" );
}

