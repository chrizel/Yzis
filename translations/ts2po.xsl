<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

	<xsl:output method="text" omit-xml-declaration="yes" encoding="utf-8"/>
	<xsl:strip-space elements="*"/>

	<xsl:param name="language" />

	<xsl:template match="/">
<xsl:text># This file is distributed under the same license as the PACKAGE package.
# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER.
# Mickael Marchand &lt;marchand@kde.org&gt;, 2005.
#
msgid ""
msgstr ""
"Project-Id-Version: Yzis\n"
"Report-Msgid-Bugs-To: \n"
"POT-Creation-Date: 2005-02-08 20:21+0100\n"
"PO-Revision-Date: 2005-02-08 20:03+0100\n"
"Last-Translator: Mickael Marchand &lt;marchand@kde.org&gt;\n"
"Language-Team: &lt;i18n@yzis.org&gt;\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"X-Generator: ts2po\n"
</xsl:text>
		<xsl:apply-templates select="//message"/>
	</xsl:template>

	<xsl:template match="message">
<xsl:text>
msgid "</xsl:text><xsl:value-of select="source"/><xsl:text>"
msgstr "</xsl:text><xsl:value-of select="translation"/><xsl:text>"
</xsl:text>
	</xsl:template>

</xsl:stylesheet>
