<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:dotml="http://www.martin-loetzsch.de/DOTML"
    xmlns:me="https://www.adambaker.org/mortal-engine">

    <xsl:template name="appropriate-label">
        <xsl:choose>
            <xsl:when test="me:gloss[@lang=$gloss-writing-system]">
                <xsl:value-of select="me:gloss[@lang=$gloss-writing-system]"></xsl:value-of>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="@label"></xsl:value-of>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- draw an empty node with the given $id -->
    <xsl:template name="empty-node">
        <xsl:param name="id"></xsl:param>
        <xsl:element name="node" namespace="http://www.martin-loetzsch.de/DOTML">
            <xsl:attribute name="id">
                <xsl:value-of select="$id"/>
            </xsl:attribute>
            <xsl:attribute name="shape">point</xsl:attribute>
            <xsl:if test="$debug='true'">
                <xsl:attribute name="shape">triangle</xsl:attribute>
                <xsl:attribute name="label">
                    <xsl:value-of select="$id"/>
                </xsl:attribute>
            </xsl:if>
        </xsl:element>
    </xsl:template>

    <!-- draw an edge -->
    <xsl:template name="edge">
        <!-- $from this node -->
        <xsl:param name="from"/>
        <!-- $to this node -->
        <xsl:param name="to"/>
        <!-- if specified, the edge will leave the $from node from the given direction (e.g., n, s, e, w) -->
        <xsl:param name="direction"/>
        <!-- if specified, will be printed in error output -->
        <xsl:param name="debug"/>
        <!-- if false, the edge will not affect the layout of the nodes -->
        <xsl:param name="constraint"/>

        <!-- error checking -->
        <xsl:if test="string-length($from) = 0">
            <xsl:message>
                <xsl:text>Error: Drawing edge with empty $from. $to is: </xsl:text>
                <xsl:value-of select="$to"></xsl:value-of>
                <xsl:if test="string-length($debug) != 0">
                    <xsl:text> (</xsl:text>
                    <xsl:value-of select="$debug"></xsl:value-of>
                    <xsl:text>)</xsl:text>
                </xsl:if>
            </xsl:message>
        </xsl:if>
        <xsl:if test="string-length($to) = 0">
            <xsl:message>
                <xsl:text>Error: Drawing edge with empty $to. $from is: </xsl:text>
                <xsl:value-of select="$from"></xsl:value-of>
                <xsl:if test="string-length($debug) != 0">
                    <xsl:text> (</xsl:text>
                    <xsl:value-of select="$debug"></xsl:value-of>
                    <xsl:text>)</xsl:text>
                </xsl:if>
            </xsl:message>
        </xsl:if>

        <!-- create the edge element -->
        <xsl:if test="string-length($from) != 0 and string-length($to) != 0">
            <xsl:element name="edge" namespace="http://www.martin-loetzsch.de/DOTML">
                <xsl:attribute name="from">
                    <xsl:choose>
                        <xsl:when test="string-length($direction) > 0">
                            <xsl:value-of select="$from"/>
                            <xsl:text>:</xsl:text>
                            <xsl:value-of select="$direction"></xsl:value-of>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$from"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:attribute>
                <xsl:attribute name="to">
                    <xsl:value-of select="$to"/>
                </xsl:attribute>
                <xsl:attribute name="arrowhead">none</xsl:attribute>
                <xsl:if test="$constraint = 'false'">
                    <xsl:attribute name="constraint">false</xsl:attribute>
                </xsl:if>
            </xsl:element>
        </xsl:if>
    </xsl:template>

    <!-- draw a node with the given $shape and $label -->
    <xsl:template name="node">
        <xsl:param name="shape"/>
        <xsl:param name="label">
            <xsl:call-template name="appropriate-label"/>
        </xsl:param>

        <!-- auto-generate the node-id -->
        <xsl:variable name="id">
            <xsl:apply-templates select="current()" mode="node-id"/>
        </xsl:variable>

        <xsl:element name="node" namespace="http://www.martin-loetzsch.de/DOTML">
            <xsl:attribute name="id">
                <xsl:value-of select="$id"/>
            </xsl:attribute>
            <xsl:attribute name="label">
                <xsl:value-of select="$label"/>
            </xsl:attribute>
            <xsl:attribute name="shape">
                <xsl:value-of select="$shape"/>
            </xsl:attribute>
            <xsl:if test="$debug='true'">
                <xsl:attribute name="label">
                    <xsl:value-of select="$id"/>
                </xsl:attribute>
            </xsl:if>
        </xsl:element>
    </xsl:template>

    <!-- generate the plain node id for a node, using 'generate-id-with-prefix' mode -->
    <xsl:template match="node()" mode="node-id">
        <xsl:apply-templates select="current()" mode="generate-id-with-prefix"/>
    </xsl:template>

    <!-- generate the preceding connector node id for a node, using 'generate-id-with-prefix' mode -->
    <xsl:template match="node()" mode="preceding-connector-node-id">
        <xsl:apply-templates select="current()" mode="generate-id-with-prefix">
            <xsl:with-param name="mode">preceding-connector-node</xsl:with-param>
        </xsl:apply-templates>
    </xsl:template>

    <!-- generate the following connector node id for a node, using 'generate-id-with-prefix' mode -->
    <xsl:template match="node()" mode="following-connector-node-id">
        <xsl:apply-templates select="current()" mode="generate-id-with-prefix">
            <xsl:with-param name="mode">following-connector-node</xsl:with-param>
        </xsl:apply-templates>
    </xsl:template>

    <!-- generate a node id, either for the node itself, its preceding connector, or its following connector,
     depending on $mode -->
    <xsl:template match="node()" mode="generate-id-with-prefix">
        <xsl:param name="mode"></xsl:param>

        <!-- $mode affect the prefix -->
        <xsl:variable name="prefix">
            <xsl:choose>
                <!-- these two cases are differentiated below -->
                <xsl:when test="$mode = 'preceding-connector-node'">pc</xsl:when>
                <xsl:when test="$mode = 'following-connector-node'">pc</xsl:when>
                <!-- the plain node prefix -->
                <xsl:otherwise>n</xsl:otherwise>
            </xsl:choose>
        </xsl:variable>

        <!-- get the name of the parent -->
        <xsl:variable name="parent-name" select="local-name(parent::*)"/>
        <!-- the parent prefix is generally the id of the parent node, followed by an underscore -->
        <xsl:variable name="parent-prefix">
            <xsl:choose>
                <!-- models is the top-level, it has no prefix -->
                <xsl:when test="$parent-name = 'models'"></xsl:when>
                <xsl:otherwise>
                    <xsl:apply-templates select="parent::*" mode="node-id"/>
                    <xsl:text>_</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <!-- the id is the parent-prefix, followed by the prefix (n, pc), then a unique number -->
        <xsl:value-of select="$parent-prefix"/>
        <xsl:value-of select="$prefix"/>
        <xsl:choose>
            <xsl:when test="$mode = 'following-connector-node'">
                <xsl:value-of select="count(preceding-sibling::*) + 2"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="count(preceding-sibling::*) + 1"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>