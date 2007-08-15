/* This file is part of the KDE libraries
   Copyright (C) 2003, 2004 Anders Lund <anders@alweb.dk>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2001,2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//BEGIN INCLUDES
#include "katesyntaxmanager.h"

#include "line.h"
#include "katesyntaxdocument.h"
#include "schema.h"
#include "attribute.h"
#include "katehighlight.h"

#include <QtCore/QSet>
#include <QtGui/QAction>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>
//END

//BEGIN KateHlManager
KateHlManager::KateHlManager()
  : QObject()
//  , m_config ("katesyntaxhighlightingrc", KConfig::NoGlobals)
  , commonSuffixes (QString(".orig;.new;~;.bak;.BAK").split(';'))
  , syntax (new KateSyntaxDocument(/*&m_config*/))
  , dynamicCtxsCount(0)
  , forceNoDCReset(false)
{
  KateSyntaxModeList modeList = syntax->modeList();
  for (int i=0; i < modeList.count(); i++)
  {
    KateHighlighting *hl = new KateHighlighting(modeList[i]);

    int insert = 0;
    for (; insert <= hlList.count(); insert++)
    {
      if (insert == hlList.count())
        break;

      if ( QString(hlList.at(insert)->section() + hlList.at(insert)->nameTranslated()).toLower()
            > QString(hl->section() + hl->nameTranslated()).toLower() )
        break;
    }

    hlList.insert (insert, hl);
    hlDict.insert (hl->name(), hl);
  }

  // Normal HL
  KateHighlighting *hl = new KateHighlighting(0);
  hlList.prepend (hl);
  hlDict.insert (hl->name(), hl);

  lastCtxsReset.start();
}

KateHlManager::~KateHlManager()
{
  delete syntax;
  qDeleteAll(hlList);
}

KateHlManager *KateHlManager::self()
{
#warning port me
#if 0
  return KateGlobal::self ()->hlManager ();
#else
  return NULL;
#endif
}

KateHighlighting *KateHlManager::getHl(int n)
{
  if (n < 0 || n >= hlList.count())
    n = 0;

  return hlList.at(n);
}

int KateHlManager::nameFind(const QString &name)
{
  int z (hlList.count() - 1);
  for (; z > 0; z--)
    if (hlList.at(z)->name() == name)
      return z;

  return z;
}

uint KateHlManager::defaultStyles()
{
  return 14;
}

QString KateHlManager::defaultStyleName(int n, bool translateNames)
{
  static QStringList names;
  static QStringList translatedNames;

  if (names.isEmpty())
  {
    names << "Normal";
    names << "Keyword";
    names << "Data Type";
    names << "Decimal/Value";
    names << "Base-N Integer";
    names << "Floating Point";
    names << "Character";
    names << "String";
    names << "Comment";
    names << "Others";
    names << "Alert";
    names << "Function";
    // this next one is for denoting the beginning/end of a user defined folding region
    names << "Region Marker";
    // this one is for marking invalid input
    names << "Error";

#warning port me
#if 0
    translatedNames << i18n("Normal");
    translatedNames << i18n("Keyword");
    translatedNames << i18n("Data Type");
    translatedNames << i18n("Decimal/Value");
    translatedNames << i18n("Base-N Integer");
    translatedNames << i18n("Floating Point");
    translatedNames << i18n("Character");
    translatedNames << i18n("String");
    translatedNames << i18n("Comment");
    translatedNames << i18n("Others");
    translatedNames << i18n("Alert");
    translatedNames << i18n("Function");
    // this next one is for denoting the beginning/end of a user defined folding region
    translatedNames << i18n("Region Marker");
    // this one is for marking invalid input
    translatedNames << i18n("Error");
#endif
  }

  return translateNames ? translatedNames[n] : names[n];
}

void KateHlManager::getDefaults(const QString &schema, KateAttributeList &list)
{
#warning port me
#if 0
  KColorScheme scheme(KColorScheme::View);
  KColorScheme schemeSelected(KColorScheme::Selection);

  KTextEditor::Attribute::Ptr normal(new KTextEditor::Attribute());
  normal->setForeground( scheme.foreground().color() );
  normal->setSelectedForeground( schemeSelected.foreground().color() );
  list.append(normal);

  KTextEditor::Attribute::Ptr keyword(new KTextEditor::Attribute());
  keyword->setForeground( scheme.foreground().color() );
  keyword->setSelectedForeground( schemeSelected.foreground().color() );
  keyword->setFontBold(true);
  list.append(keyword);

  KTextEditor::Attribute::Ptr dataType(new KTextEditor::Attribute());
  dataType->setForeground( scheme.foreground(KColorScheme::LinkText).color() );
  dataType->setSelectedForeground( schemeSelected.foreground(KColorScheme::LinkText).color() );
  list.append(dataType);

  KTextEditor::Attribute::Ptr decimal(new KTextEditor::Attribute());
  decimal->setForeground( scheme.foreground(KColorScheme::NeutralText).color() );
  decimal->setSelectedForeground( schemeSelected.foreground(KColorScheme::NeutralText).color() );
  list.append(decimal);

  KTextEditor::Attribute::Ptr basen(new KTextEditor::Attribute());
  basen->setForeground( scheme.foreground(KColorScheme::NeutralText).color() );
  basen->setSelectedForeground( schemeSelected.foreground(KColorScheme::NeutralText).color() );
  list.append(basen);

  KTextEditor::Attribute::Ptr floatAttribute(new KTextEditor::Attribute());
  floatAttribute->setForeground( scheme.foreground(KColorScheme::NeutralText).color() );
  floatAttribute->setSelectedForeground( schemeSelected.foreground(KColorScheme::NeutralText).color() );
  list.append(floatAttribute);

  KTextEditor::Attribute::Ptr charAttribute(new KTextEditor::Attribute());
  charAttribute->setForeground( scheme.foreground(KColorScheme::ActiveText).color() );
  charAttribute->setSelectedForeground( schemeSelected.foreground(KColorScheme::ActiveText).color() );
  list.append(charAttribute);

  KTextEditor::Attribute::Ptr string(new KTextEditor::Attribute());
  string->setForeground( scheme.foreground(KColorScheme::NegativeText).color() );
  string->setSelectedForeground( schemeSelected.foreground(KColorScheme::NegativeText).color() );
  list.append(string);

  KTextEditor::Attribute::Ptr comment(new KTextEditor::Attribute());
  comment->setForeground( scheme.foreground(KColorScheme::InactiveText).color() );
  comment->setSelectedForeground( schemeSelected.foreground(KColorScheme::InactiveText).color() );
  comment->setFontItalic(true);
  list.append(comment);

  KTextEditor::Attribute::Ptr others(new KTextEditor::Attribute());
  others->setForeground( scheme.foreground(KColorScheme::PositiveText).color() );
  others->setSelectedForeground( schemeSelected.foreground(KColorScheme::PositiveText).color() );
  list.append(others);

  KTextEditor::Attribute::Ptr alert(new KTextEditor::Attribute());
  alert->setForeground( scheme.foreground(KColorScheme::NegativeText).color() );
  alert->setSelectedForeground( schemeSelected.foreground(KColorScheme::NegativeText).color() );
  alert->setFontBold(true);
  alert->setBackground( KColorUtils::tint( scheme.background().color(), scheme.foreground(KColorScheme::NegativeText).color() ) );
  list.append(alert);

  KTextEditor::Attribute::Ptr functionAttribute(new KTextEditor::Attribute());
  functionAttribute->setForeground( scheme.foreground(KColorScheme::VisitedText).color() );
  functionAttribute->setSelectedForeground( schemeSelected.foreground(KColorScheme::VisitedText).color() );
  list.append(functionAttribute);

  KTextEditor::Attribute::Ptr regionmarker(new KTextEditor::Attribute());
  regionmarker->setForeground( scheme.foreground(KColorScheme::LinkText).color() );
  regionmarker->setSelectedForeground( schemeSelected.foreground(KColorScheme::LinkText).color() );
  regionmarker->setBackground( KColorUtils::tint( scheme.background().color(), scheme.foreground(KColorScheme::LinkText).color() ) );
  list.append(regionmarker);

  KTextEditor::Attribute::Ptr error(new KTextEditor::Attribute());
  error->setForeground( scheme.foreground(KColorScheme::NegativeText) );
  error->setSelectedForeground( schemeSelected.foreground(KColorScheme::NegativeText).color() );
  error->setFontUnderline(true);
  list.append(error);

  KConfigGroup config(KateHlManager::self()->self()->getKConfig(),
                      "Default Item Styles - Schema " + schema);

  for (uint z = 0; z < defaultStyles(); z++)
  {
    KTextEditor::Attribute::Ptr i = list.at(z);
    QStringList s = config.readEntry(defaultStyleName(z), QStringList());
    if (!s.isEmpty())
    {
      while( s.count()<8)
        s << "";

      QString tmp;
      QRgb col;

      tmp=s[0]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setForeground(YZColor(col)); }

      tmp=s[1]; if (!tmp.isEmpty()) {
         col=tmp.toUInt(0,16); i->setSelectedForeground(YZColor(col)); }

      tmp=s[2]; if (!tmp.isEmpty()) i->setFontBold(tmp!="0");

      tmp=s[3]; if (!tmp.isEmpty()) i->setFontItalic(tmp!="0");

      tmp=s[4]; if (!tmp.isEmpty()) i->setFontStrikeOut(tmp!="0");

      tmp=s[5]; if (!tmp.isEmpty()) i->setFontUnderline(tmp!="0");

      tmp=s[6]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setBackground(YZColor(col));
        }
        else
          i->clearBackground();
      }
      tmp=s[7]; if (!tmp.isEmpty()) {
        if ( tmp != "-" )
        {
          col=tmp.toUInt(0,16);
          i->setSelectedBackground(YZColor(col));
        }
        else
          i->clearProperty(KTextEditor::Attribute::SelectedBackground);
      }
    }
  }
#endif
}

void KateHlManager::setDefaults(const QString &schema, KateAttributeList &list)
{
#warning port me
#if 0
  KConfigGroup config(KateHlManager::self()->self()->getKConfig(),
                      "Default Item Styles - Schema " + schema);

  for (uint z = 0; z < defaultStyles(); z++)
  {
    QStringList settings;
    KTextEditor::Attribute::Ptr p = list.at(z);

    settings<<(p->hasProperty(QTextFormat::ForegroundBrush)?QString::number(p->foreground().color().rgb(),16):"");
    settings<<(p->hasProperty(KTextEditor::Attribute::SelectedForeground)?QString::number(p->selectedForeground().color().rgb(),16):"");
    settings<<(p->hasProperty(QTextFormat::FontWeight)?(p->fontBold()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::FontItalic)?(p->fontItalic()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::FontStrikeOut)?(p->fontStrikeOut()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::FontUnderline)?(p->fontUnderline()?"1":"0"):"");
    settings<<(p->hasProperty(QTextFormat::BackgroundBrush)?QString::number(p->background().color().rgb(),16):"");
    settings<<(p->hasProperty(KTextEditor::Attribute::SelectedBackground)?QString::number(p->selectedBackground().color().rgb(),16):"");
    settings<<"---";

    config.writeEntry(defaultStyleName(z),settings);
  }

  emit changed();
#endif
}

int KateHlManager::highlights()
{
  return (int) hlList.count();
}

QString KateHlManager::hlName(int n)
{
  return hlList.at(n)->name();
}

QString KateHlManager::hlNameTranslated(int n)
{
  return hlList.at(n)->nameTranslated();
}

QString KateHlManager::hlSection(int n)
{
  return hlList.at(n)->section();
}

bool KateHlManager::hlHidden(int n)
{
  return hlList.at(n)->hidden();
}

QString KateHlManager::identifierForName(const QString& name)
{
  KateHighlighting *hl = 0;

  if ((hl = hlDict[name]))
    return hl->getIdentifier ();

  return QString();
}

bool KateHlManager::resetDynamicCtxs()
{
  if (forceNoDCReset)
    return false;

  if (lastCtxsReset.elapsed() < KATE_DYNAMIC_CONTEXTS_RESET_DELAY)
    return false;

  foreach (KateHighlighting *hl, hlList)
    hl->dropDynamicContexts();

  dynamicCtxsCount = 0;
  lastCtxsReset.start();

  return true;
}
//END

// kate: space-indent on; indent-width 2; replace-tabs on;
