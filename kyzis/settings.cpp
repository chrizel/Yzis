// This file is generated by kconfig_compiler from kyzis.kcfg.
// All changes you do to this file will be lost.

#include "settings.h"

#include <kstaticdeleter.h>

Settings *Settings::mSelf = 0;
static KStaticDeleter<Settings> staticSettingsDeleter;

Settings *Settings::self()
{
  if ( !mSelf ) {
    staticSettingsDeleter.setObject( mSelf, new Settings() );
    mSelf->readConfig();
  }

  return mSelf;
}

Settings::Settings(  )
  : KConfigSkeleton( QString::fromLatin1( "kyzisrc" ) )
{
  mSelf = this;
  setCurrentGroup( QString::fromLatin1( "Preferences" ) );

  KConfigSkeleton::ItemColor  *itemcol_background;
  itemcol_background = new KConfigSkeleton::ItemColor( currentGroup(), "col_background", mCol_background, QColor( "black" ) );
  addItem( itemcol_background );
  KConfigSkeleton::ItemColor  *itemcol_foreground;
  itemcol_foreground = new KConfigSkeleton::ItemColor( currentGroup(), "col_foreground", mCol_foreground, QColor( "white" ) );
  addItem( itemcol_foreground );
  KConfigSkeleton::ItemInt  *itemswitch_key;
  itemswitch_key = new KConfigSkeleton::ItemInt( currentGroup(), "switch_key", mSwitch_key, 212 );
  addItem( itemswitch_key );
}

Settings::~Settings()
{
  if ( mSelf == this )
    staticSettingsDeleter.setObject( mSelf, 0, false );
}

