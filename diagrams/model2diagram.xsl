<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:dotml="http://www.martin-loetzsch.de/DOTML"
    xmlns:me="https://www.adambaker.org/mortal-engine">

    <xsl:output method="xml" version="1.0" encoding="UTF-8" omit-xml-declaration="no" indent="yes"/>

    <xsl:strip-space elements="*"/>

    <!-- If 'which-model' is specified, a diagram is only produced for the model with that label (e.g., “Nouns”) -->
    <xsl:param name="which-model"></xsl:param>
    <!-- The option produces debug output -->
    <xsl:param name="debug">false</xsl:param>
    <xsl:param name="gloss-writing-system"></xsl:param>

    <!-- Most of the logic is in these three include files -->
    <xsl:include href="model2diagram.nodes.xsl" />
    <xsl:include href="model2diagram.edges.xsl" />
    <xsl:include href="model2diagram.shared.xsl" />

    <!-- match the root -->
    <xsl:template match="/">
        <!-- the DotML base element -->
        <xsl:element name="graph" namespace="http://www.martin-loetzsch.de/DOTML">
            <xsl:attribute name="compound">true</xsl:attribute>

            <xsl:choose>
                <xsl:when test="$which-model">
                    <xsl:message>Only showing: <xsl:value-of select="$which-model"/>
                    </xsl:message>
                    <xsl:message>
                        <xsl:value-of select="count(/me:morphology/me:model[@label=$which-model])"/>
 models</xsl:message>
                    <!-- If $which-model is specified, select only that model -->
                    <xsl:apply-templates select="/me:morphology/me:model[@label=$which-model]" mode="make-node"/>
                    <xsl:apply-templates select="/me:morphology/me:model[@label=$which-model]" mode="edges"/>
                </xsl:when>
                <xsl:otherwise>
                    <!-- Otherwise, produce the graphs for all of the models -->
                    <!-- 'make-node' mode produces the nodes -->
                    <xsl:apply-templates select="/me:morphology/me:model" mode="make-node"/>
                    <!-- 'edges' mode produces the edges (i.e., connections) -->
                    <xsl:apply-templates select="/me:morphology/me:model" mode="edges"/>
                    <!-- 'jumps' mode connects jumps; this is only appropriate if all of the models are drawn -->
                    <xsl:apply-templates select="//me:jump" mode="jumps"/>
                </xsl:otherwise>
            </xsl:choose>

        </xsl:element>
    </xsl:template>

    <xsl:template match="node()|@*"></xsl:template>

</xsl:stylesheet>
