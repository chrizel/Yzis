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
#include <qslider.h>
#include <qhbox.h>

#include <kfontrequester.h>
#include <kcolorbutton.h>

#include "debug.h"
#include "yzis.h"

#include "configdialog.h"
#include <qcheckbox.h>
#include "hlconfig.h"

KYZisConfigDialog::KYZisConfigDialog( QWidget* parent, const char* name, KConfigSkeleton* config, DialogType dialogType )
	: KConfigDialog( parent, name, config, dialogType ) {

	config->readConfig( );

	setShowIconsInTreeList( true );

	setupPages();

}

KYZisConfigDialog::~KYZisConfigDialog( ) {
}

void KYZisConfigDialog::setupPages() {
	unsigned int line = 0;

	/**
	 * Appearance
	 */
	QWidget* pageAppearance = new QWidget( this, "Appearance" );
	QGridLayout* layoutAppearance = new QGridLayout( pageAppearance, 2, 5, 0, 10 );

	// Font
	QLabel* label_Font = new QLabel( i18n("Text font"), pageAppearance );
	KFontRequester* kcfg_Font = new KFontRequester( pageAppearance, "kcfg_Font" );

	layoutAppearance->addWidget( label_Font, line, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_Font, line, 1 );
	++line;

	// Transparency
	QLabel* label_transparency = new QLabel( i18n("Use a transparent background"), pageAppearance );
	QCheckBox* kcfg_transparency  = new QCheckBox( pageAppearance, "kcfg_transparency" );

	layoutAppearance->addWidget( label_transparency, line, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_transparency, line, 1 );
	++line;

	// Opacity
	QLabel* label_opacity = new QLabel( i18n("Background color opacity"), pageAppearance );
	QHBox* box_opacity = new QHBox( pageAppearance );
	QLabel* label_left = new QLabel( i18n("Min"), box_opacity );
	QSlider* slide_fade = new QSlider( 0, 100, 1, 0, Qt::Horizontal, box_opacity, "kcfg_opacity" );
	QLabel* label_right = new QLabel( i18n("Max"), box_opacity );
	box_opacity->setStretchFactor( slide_fade, 1 );

	layoutAppearance->addWidget( label_opacity, line, 0, Qt::AlignRight );
	layoutAppearance->addWidget( box_opacity, line, 1 );
	++line;

	// BG Color
	QLabel* label_colorBG = new QLabel( i18n("Background Color"), pageAppearance );
	KColorButton* kcfg_colorBG = new KColorButton( pageAppearance, "kcfg_colorBG" );

	layoutAppearance->addWidget( label_colorBG, line, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_colorBG, line, 1 );
	++line;

	// FG Color
	QLabel* label_colorFG = new QLabel( i18n("Foreground Color"), pageAppearance );
	KColorButton* kcfg_colorFG = new KColorButton( pageAppearance, "kcfg_colorFG" );

	layoutAppearance->addWidget( label_colorFG, line, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_colorFG, line, 1 );
	++line;

	// Konsole
/*	QLabel* label_konsole = new QLabel( i18n("Konsole"), pageAppearance );
	QCheckBox* kcfg_konsole = new QCheckBox( pageAppearance, "kcfg_konsole" );
	layoutAppearance->addWidget( label_konsole, line, 0, Qt::AlignRight );
	layoutAppearance->addWidget( kcfg_konsole, line, 1 );
	++line;
*/
	layoutAppearance->setRowStretch( line, 1 );


	addPage( pageAppearance, i18n("Appearance"), "colorize" );

	YzisSchemaConfigPage *pageHL = new YzisSchemaConfigPage ( this );
	connect( pageHL, SIGNAL( changed() ), this, SLOT( slotChanged() ) );
	addPage( pageHL, i18n("Syntax Highlighting"), "hl" );
}

void KYZisConfigDialog::slotChanged() {
	actionButton( KDialogBase::Apply )->setEnabled( true );
}

#include "configdialog.moc"
