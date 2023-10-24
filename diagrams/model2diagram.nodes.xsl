<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:dotml="http://www.martin-loetzsch.de/DOTML"
    xmlns:me="https://www.adambaker.org/mortal-engine">

    <!-- In this mode, the node, along with its preceding connector node, are created.
     The actual (shape) node is drawn in 'make-node' mode -->
    <xsl:template match="node()" mode="nodes">
        <!-- the default is to create a preceding connector node (the dot-node that occurs
         before the 'proper' node, but this can be disabled -->
        <xsl:param name="create-preceding-node" select="true()"/>

        <!-- Draw the preceding connector node -->
        <xsl:if test="$create-preceding-node">
            <xsl:call-template name="empty-node">
                <xsl:with-param name="id">
                    <xsl:apply-templates select="current()" mode="preceding-connector-node-id"/>
                </xsl:with-param>
            </xsl:call-template>
        </xsl:if>

        <!-- Create a comment to identify the node in the DotML file -->
        <xsl:apply-templates mode="identify-node" select="."></xsl:apply-templates>
        <!-- Draw the node itself -->
        <xsl:apply-templates mode="make-node" select="."></xsl:apply-templates>

    </xsl:template>

    <!-- A me:model is a cluster element rather than a node with a shape -->
    <xsl:template match="me:model" mode="make-node">
        <xsl:element name="cluster" namespace="http://www.martin-loetzsch.de/DOTML">
            <xsl:attribute name="id">
                <xsl:apply-templates select="current()" mode="node-id"/>
            </xsl:attribute>
            <xsl:attribute name="label">
                <xsl:value-of select="@label"/>
            </xsl:attribute>
            <!-- <xsl:attribute name="style">invis</xsl:attribute> -->

            <!-- the insides of the me:model are handled the same as the insides of paths; hence the special mode -->
            <xsl:apply-templates select="current()" mode="path-like-nodes"/>
        </xsl:element>
    </xsl:template>

    <!-- A me:path is handled with a cluster group -->
    <xsl:template match="me:path|me:sequence" mode="make-node">
        <xsl:variable name="this-id">
            <xsl:apply-templates select="current()" mode="node-id"/>
        </xsl:variable>

        <!-- create the cluster -->
        <xsl:element name="cluster" namespace="http://www.martin-loetzsch.de/DOTML">
            <xsl:attribute name="id">
                <xsl:value-of select="$this-id"/>
            </xsl:attribute>
            <xsl:attribute name="label">
                <xsl:value-of select="@label"/>
            </xsl:attribute>
            <xsl:attribute name="style">invis</xsl:attribute>
            <xsl:if test="$debug='true'">
                <xsl:attribute name="label">
                    <xsl:value-of select="$this-id"/>
                </xsl:attribute>
                <xsl:attribute name="style">dotted</xsl:attribute>
            </xsl:if>

            <!-- Process the inside of the path in 'path-like-nodes' mode -->
            <xsl:apply-templates select="current()" mode="path-like-nodes"/>
        </xsl:element>

    </xsl:template>

    <xsl:template match="me:model|me:path|me:sequence" mode="path-like-nodes">
        <!-- Select the child nodes from the appropriate model -->
        <xsl:variable name="children" select="child::*[local-name() != 'optional']"/>

        <!-- Create the nodes -->
        <xsl:apply-templates select="$children" mode="nodes"></xsl:apply-templates>

        <!-- Create the final following connector node -->
        <xsl:call-template name="empty-node">
            <xsl:with-param name="id">
                <xsl:apply-templates select="$children[last()]" mode="following-connector-node-id"/>
            </xsl:with-param>
        </xsl:call-template>
    </xsl:template>

    <!-- morpheme node; with a specific shape -->
    <xsl:template match="me:morpheme" mode="make-node">
        <xsl:call-template name="node">
            <xsl:with-param name="shape">oval</xsl:with-param>
        </xsl:call-template>
    </xsl:template>

    <!-- sqlite-stem-list and stem-list node; with a specific shape -->
    <xsl:template match="me:sqlite-stem-list|me:stem-list" mode="make-node">
        <xsl:call-template name="node">
            <xsl:with-param name="shape">square</xsl:with-param>
        </xsl:call-template>
    </xsl:template>

    <!-- mutually-exclusive-morphemes node -->
    <xsl:template match="me:mutually-exclusive-morphemes" mode="make-node">
        <xsl:variable name="this-id">
            <xsl:apply-templates select="current()" mode="node-id"/>
        </xsl:variable>

        <!-- create a cluster element -->
        <xsl:element name="cluster" namespace="http://www.martin-loetzsch.de/DOTML">
            <xsl:attribute name="id">
                <xsl:value-of select="$this-id"/>
            </xsl:attribute>
            <xsl:attribute name="style">invis</xsl:attribute>
            <xsl:if test="$debug='true'">
                <xsl:attribute name="label">
                    <xsl:value-of select="$this-id"/>
                </xsl:attribute>
                <xsl:attribute name="style">dotted</xsl:attribute>
            </xsl:if>

            <!-- create nodes for child me:morpheme nodes, but don't draw the preceding connector
             nodes, because the me:morpheme nodes will be connected to the preceding connector
             node for the MEM -->
            <xsl:apply-templates select="me:morpheme" mode="nodes">
                <xsl:with-param name="create-preceding-node" select="false()"/>
            </xsl:apply-templates>

        </xsl:element>
    </xsl:template>

    <!-- fork node -->
    <xsl:template match="me:fork" mode="make-node">
        <xsl:variable name="this-id">
            <xsl:apply-templates select="current()" mode="node-id"/>
        </xsl:variable>

        <!-- create the fork cluster element -->
        <xsl:element name="cluster" namespace="http://www.martin-loetzsch.de/DOTML">
            <xsl:attribute name="id">
                <xsl:value-of select="$this-id"/>
            </xsl:attribute>
            <xsl:attribute name="label">
                <xsl:value-of select="@label"/>
            </xsl:attribute>
            <xsl:attribute name="style">invis</xsl:attribute>
            <xsl:if test="$debug='true'">
                <xsl:attribute name="label">
                    <xsl:value-of select="$this-id"/>
                </xsl:attribute>
                <xsl:attribute name="style">dotted</xsl:attribute>
            </xsl:if>

            <!-- process the paths -->
            <!-- As in the MEM, the first nodes will connect to the preceding
             node elements of the parent me:fork -->
            <xsl:apply-templates mode="nodes" select="me:path">
                <xsl:with-param name="create-preceding-node" select="false()"/>
            </xsl:apply-templates>

        </xsl:element>

    </xsl:template>

    <!-- jump nodes get their own special shape -->
    <xsl:template match="me:jump" mode="make-node">
        <xsl:call-template name="node">
            <xsl:with-param name="shape">invtriangle</xsl:with-param>
            <xsl:with-param name="width">0.5</xsl:with-param>
            <xsl:with-param name="label">Jump</xsl:with-param>
        </xsl:call-template>
    </xsl:template>

    <!-- copy-of node -->
    <xsl:template match="me:copy-of" mode="make-node">
        <xsl:variable name="target-id" select="@target-id"/>
        <xsl:call-template name="node">
            <xsl:with-param name="shape">hexagon</xsl:with-param>
            <xsl:with-param name="label">
                <xsl:text>Copy of </xsl:text>
                <xsl:value-of select="$target-id"/>
            </xsl:with-param>
        </xsl:call-template>
    </xsl:template>

    <!-- <optional/> can be inside sequences; ignore it -->
    <xsl:template match="me:optional" mode="make-node"></xsl:template>

    <!-- This is a little code to make sure we're handling all the nodes -->
    <xsl:template match="me:*" mode="make-node">
        <xsl:message>Node not handled in 'make-node' mode:<xsl:value-of select="local-name()"></xsl:value-of>
        </xsl:message>
    </xsl:template>


</xsl:stylesheet>