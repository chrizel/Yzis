# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

DESCRIPTION="Yzis - VI-like editor"
HOMEPAGE="http://www.yzis.org/"
LICENSE="LGPL-2 GPL-2"

IUSE="ncurses pslib kde arts kdeenablefinal debug"
SLOT="0"
KEYWORDS="~x86 ~amd64"

SRC_URI="ftp://download.yzis.org/yzis/${PN}_${PV##*_pre}-1.tar.gz"
RESTRICT="nomirror"

RDEPEND="
	>=x11-libs/qt-3.3
	ncurses? >=sys-libs/ncurses-5.4
	kde? >=kde-base/kdelibs-3.3
	pslib? >=dev-libs/pslib-0.2.2
	sys-apps/file
	=dev-lang/lua-5*"

DEPEND="
	>=sys-devel/automake-1.7.0
	sys-devel/autoconf
	${RDEPEND}"

S=${WORKDIR}/${PN}-${PV##*_pre}

src_compile() {
	myconf=""
	if ! use kde; then
		myconf="${myconf} --disable-kyzis"
	fi
	if ! use ncurses; then
		myconf="${myconf} --disable-nyzis"
	fi
	myconf="$myconf $(use_with arts)"

	if useq kdeenablefinal; then
		myconf="$myconf --enable-final"
	else
		myconf="$myconf --disable-final"
	fi
	[ -z "$UNSERMAKE" ] && myconf="$myconf --disable-dependency-tracking"
	if use debug ; then
		myconf="$myconf --enable-debug=full --with-debug"
	else
		myconf="$myconf --disable-debug --without-debug"
	fi

	# from portage/eclass/kde.eclass :
	# fix the sandbox errors "can't write to .kde or .qt" problems.
	# this is a fake homedir that is writeable under the sandbox, so that the build process
	# can do anything it wants with it.
	REALHOME="$HOME"
	mkdir -p $T/fakehome/.kde
	mkdir -p $T/fakehome/.qt
	export HOME="$T/fakehome"
	addwrite "${QTDIR}/etc/settings"
	# things that should access the real homedir
	[ -d "$REALHOME/.ccache" ] && ln -sf "$REALHOME/.ccache" "$HOME/"	
	[ -n "$UNSERMAKE" ] && addwrite "/usr/kde/unsermake"

	if [ ! -f "./configure" ] || [ -n "$UNSERMAKE" ]; then
		make -f Makefile.cvs
	fi
	econf ${myconf} || die "Configuration failed."
	emake || die "Build failed."
}

src_install() {
	emake DESTDIR=${D} install
	dodoc TODO README AUTHORS ChangeLog COPYING COPYING.LGPL doc/VI-COMPATIBILITY
	insinto "usr/share/doc/${PF}"; doins -r doc/examples
}

