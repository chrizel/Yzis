#!/bin/sh

# this script must be run from the debian/ directory
#

mv -f changelog changelog.old;

#cat templates/changelog | sed -e 's/__DATE__/`date +%d%m%Y`/' | sed -e 's/__LONGDATE__/`date -R`/' > changelog;
perl -pe "s/__DATE__/`date +%Y%d%m`/" templates/changelog > changelog.tmp;
perl -pe "s/__LONGDATE__/`date -R`/" changelog.tmp > changelog;
rm -f changelog.tmp;
cat changelog.old >> changelog;
rm -f changelog.old;

