#!/bin/sh

# this script must be run from the debian/ directory
#

mv -f changelog changelog.old;

perl -pe "s/__DATE__/`date +%Y%m%d`/" templates/changelog > changelog.tmp;
perl -pe "s/__LONGDATE__/`date -R`/" changelog.tmp > changelog;
rm -f changelog.tmp;
cat changelog.old >> changelog;
rm -f changelog.old;

