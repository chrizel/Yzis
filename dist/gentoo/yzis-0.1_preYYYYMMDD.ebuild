# Copyright 1999-2005 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils

DESCRIPTION="Yzis - VI-like editor"
HOMEPAGE="http://www.yzis.org/"
LICENSE="LGPL-2 GPL-2"

IUSE="ncurses ps kde arts kdeenablefinal debug"
SLOT="0"
KEYWORDS="~x86 ~amd64"

SRC_URI="ftp://download.yzis.org/yzis/${PN}_${PV##*_pre}-1.tar.gz"

RDEPEND=">=x11-libs/qt-3.3
		ncurses? ( >=sys-libs/ncurses-5.4 )
		kde? ( >=kde-base/kdelibs-3.3 )
		ps? ( >=dev-libs/pslib-0.2.2 )
		arts? ( >=kde-base/arts-1.3 )
		>=sys-apps/file-4.0
		>=sys-devel/gettext-0.12.0
		media-libs/jpeg
		=dev-lang/lua-5*"
DEPEND="${RDEPEND}
		sys-devel/autoconf
		>=sys-devel/automake-1.7.0"

S=${WORKDIR}/${PN}-${PV##*_pre}

pkg_setup() {
	# from portage/eclass/kde.eclass :
	use kde && use arts && if ! built_with_use kdelibs arts ; then
		eerror "You are trying to compile yzis with the \"arts\" USE flag enabled."
		eerror "However, $(best_version kdelibs) was compiled with this flag disabled."
		eerror
		eerror "You must either disable this use flag, or recompile"
		eerror "$(best_version kdelibs) with this use flag enabled."
		die "kdelibs not built with arts"
	fi
	#yzis needs ncurses with wide-char-support
	use ncurses && if ! built_with_use ncurses unicode ; then
		eerror "You are trying to compile yzis with the \"ncurses\" USE flag enabled."
		eerror "However, $(best_version ncurses) was compiled with \"unicode\" disabled."
		eerror
		eerror "You must either disable this use flag, or recompile"
		eerror "$(best_version ncurses) with \"unicode\" enabled."
		die "ncurses not built with unicode"
	fi
}

src_compile() {
	local myconf="$(use_with arts) $(use_enable kde kyzis)
			$(use_enable ncurses nyzis) $(use_enable ps pslib)"

	# from portage/eclass/kde.eclass :
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
	docinto "examples"; dodoc doc/examples/*
}

