<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<!--
	Simple documantation index generator, just to test ;)
	The output format is :
	<keyword>	#<element id>

	If a keyword match the search, we have to scroll to the corresponding <element id>
-->

	<xsl:output method="text" encoding="utf-8" />

	<xsl:strip-space elements="*" />

	<xsl:template name="index">
		<xsl:param name="keyword" />
		<xsl:param name="ref" />
		<xsl:value-of select="$keyword"/><xsl:text>	#</xsl:text><xsl:value-of select="$ref" /><xsl:text>
</xsl:text>
	</xsl:template>

	<!-- 
		extract commands
	-->
	<xsl:template match="listitem/simplelist[@type='inline']/member">
		<xsl:call-template name="index">
			<xsl:with-param name="ref"><xsl:value-of select="../../@id" /></xsl:with-param>
			<xsl:with-param name="keyword"><xsl:value-of select="."/></xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	<!--
		extract all others elements id
	-->
	<xsl:template match="//*[@id]">
		<xsl:call-template name="index">
			<xsl:with-param name="ref"><xsl:value-of select="@id" /></xsl:with-param>
			<xsl:with-param name="keyword">
				<!-- remove common id prefix -->
				<xsl:choose>
					<xsl:when test="starts-with(@id,'documentation-') or starts-with(@id,'command-')">
						<xsl:value-of select="substring-after(@id,'-')" />
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="@id"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:with-param>
		</xsl:call-template>
		<xsl:apply-templates />
	</xsl:template>


	<!-- ignore these elements -->
	<xsl:template match="text()" />

	<!-- continue -->
	<xsl:template match="*">
		<xsl:apply-templates />
	</xsl:template>

</xsl:stylesheet>

