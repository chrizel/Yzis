/*
*  Copyright (c) 2006 Loïc Paulevé <panard@backzone.net>
*
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of version 2 of the GNU General Public
*  License as published by the Free Software Foundation
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*/


#include <QDialog>

#include "ui_configuredialog.h"

class QYZConfigureDialog : public QDialog, private Ui::ConfigureDialog
{
    Q_OBJECT

public :
    QYZConfigureDialog( QWidget* parent = 0 );
};


