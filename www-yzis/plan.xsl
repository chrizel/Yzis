<?xml version='1.0' encoding='ISO-8859-1'?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" version="4.01" doctype-public="-//W3C//DTD HTML 4.01//EN" encoding="ISO-8859-1"/>

<xsl:template match="document">
  <html>
    <head>
      <title><xsl:value-of select="titre"/></title>
        <style type="text/css">
                .hide { display: none }
        </style>
    </head>
    <body>
<xsl:for-each select="milestone">
	<xsl:sort select="version"/>
		<FONT SIZE="+1" COLOR="Red">Milestone <xsl:value-of select="version"/> (<xsl:value-of select="estimation"/>)</FONT><BR></BR>
	<TABLE>
	<xsl:for-each select="feature">
		<TR>
			<TD WIDTH="400"><FONT COLOR="blue"><xsl:value-of select="name"/></FONT></TD>
			<TD WIDTH="200"><FONT COLOR="green"><xsl:value-of select="status"/></FONT></TD>
		</TR>
	</xsl:for-each>
	</TABLE>
	<BR></BR>
</xsl:for-each>

</body>
</html>
</xsl:template>
</xsl:stylesheet>
