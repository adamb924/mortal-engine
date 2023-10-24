<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns="http://www.w3.org/1999/xhtml"
    xmlns:dotml="http://www.martin-loetzsch.de/DOTML"
    xmlns:me="https://www.adambaker.org/mortal-engine">

    <!-- These nodes are just typical shape nodes; they have no complicating behavior -->
    <xsl:template match="me:morpheme|me:sqlite-stem-list|me:stem-list|me:jump" mode="edges">
        <!-- $previous is the preceding connector node of the current node -->
        <xsl:param name="previous">
            <xsl:apply-templates select="current()" mode="preceding-connector-node-id"/>
        </xsl:param>
        <!-- $next is the following connector node of the current node -->
        <xsl:param name="next">
            <xsl:apply-templates select="current()" mode="following-connector-node-id"/>
        </xsl:param>
        <!-- $this-id is the current node -->
        <xsl:variable name="this-id">
            <xsl:apply-templates select="current()" mode="node-id"/>
        </xsl:variable>

        <!-- Connect the node to its empty-preceding node -->
        <xsl:comment>
            <xsl:text>Connection to preceding node for:</xsl:text>
            <xsl:apply-templates select="." mode="node-identifier-string"/>
        </xsl:comment>
        <xsl:call-template name="edge">
            <xsl:with-param name="from" select="$previous"/>
            <xsl:with-param name="to" select="$this-id"/>
        </xsl:call-template>

        <!-- Connect it to the following node -->
        <xsl:comment>
            <xsl:text>Connection to following node for:</xsl:text>
            <xsl:apply-templates select="." mode="node-identifier-string"/>
        </xsl:comment>
        <xsl:call-template name="edge">
            <xsl:with-param name="from" select="$this-id"/>
            <xsl:with-param name="to" select="$next"/>
        </xsl:call-template>

        <!-- Draw a skip-arc if it's optional -->
        <xsl:if test="me:optional">
            <xsl:comment>
                <xsl:text>Optionality edge for:</xsl:text>
                <xsl:apply-templates select="." mode="node-identifier-string"/>
            </xsl:comment>
            <xsl:call-template name="edge">
                <xsl:with-param name="from" select="$previous"/>
                <xsl:with-param name="to" select="$next"/>
                <xsl:with-param name="constraint">false</xsl:with-param>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <!-- A MEM node is special because it has child me:morpheme elements -->
    <xsl:template match="me:mutually-exclusive-morphemes" mode="edges">
        <!-- for a MEM, there are preceding and following nodes, 
         but the actual node is just a cluster object rather than a shape, 
         so nothing connects to it -->
        <xsl:variable name="previous">
            <xsl:apply-templates select="current()" mode="preceding-connector-node-id"/>
        </xsl:variable>
        <xsl:variable name="next">
            <xsl:apply-templates select="current()" mode="following-connector-node-id"/>
        </xsl:variable>

        <!-- for this node, the edges are drawn from the preceding and following connectors
         to the morpheme nodes themselves. i.e., the morphemes are connected to the MEM's
         preceding and following nodes rather than to their own -->
        <xsl:apply-templates select="me:morpheme" mode="edges">
            <xsl:with-param name="previous">
                <xsl:value-of select="$previous"></xsl:value-of>
            </xsl:with-param>
            <xsl:with-param name="next">
                <xsl:value-of select="$next"></xsl:value-of>
            </xsl:with-param>
        </xsl:apply-templates>

        <!-- Draw a skip-arc if it's optional -->
        <xsl:if test="me:optional">
            <xsl:comment>
                <xsl:text>Optionality edge for:</xsl:text>
                <xsl:apply-templates select="." mode="node-identifier-string"/>
            </xsl:comment>
            <xsl:call-template name="edge">
                <xsl:with-param name="from" select="$previous"/>
                <xsl:with-param name="to" select="$next"/>
                <!-- jump node connections come from the bottom of the node -->
                <xsl:with-param name="direction">
                    <xsl:if test="local-name() = 'jump'">s</xsl:if>
                </xsl:with-param>
                <xsl:with-param name="constraint">false</xsl:with-param>
            </xsl:call-template>
        </xsl:if>

    </xsl:template>

    <xsl:template match="me:fork" mode="edges">
        <!-- Create the edges for the forks -->
        <xsl:apply-templates select="me:path" mode="edges"/>

        <!-- Draw a skip-arc if it's optional -->
        <xsl:if test="me:optional">
            <xsl:comment>
                <xsl:text>Optionality edge for:</xsl:text>
                <xsl:apply-templates select="." mode="node-identifier-string"/>
            </xsl:comment>
            <xsl:call-template name="edge">
                <xsl:with-param name="from">
                    <xsl:apply-templates select="current()" mode="preceding-connector-node-id"/>
                </xsl:with-param>
                <xsl:with-param name="to">
                    <xsl:apply-templates select="current()" mode="following-connector-node-id"/>
                </xsl:with-param>
                <xsl:with-param name="constraint">false</xsl:with-param>
            </xsl:call-template>
        </xsl:if>

    </xsl:template>

    <!-- call the edges template for each of the child nodes -->
    <xsl:template match="me:model" mode="edges">
        <xsl:apply-templates select="child::node()" mode="edges"/>
    </xsl:template>

    <xsl:template match="me:path" mode="edges">
        <!-- $previous is the preceding connector node of the parent, which is a me:fork -->
        <xsl:variable name="previous">
            <xsl:apply-templates select="parent::me:fork" mode="preceding-connector-node-id"/>
        </xsl:variable>

        <!-- $first-to-link-to is the preceding connector node of the first child element -->
        <xsl:variable name="first-to-link-to">
            <xsl:apply-templates select="child::*[1]" mode="preceding-connector-node-id"/>
        </xsl:variable>

        <!-- this creates the edge from the preceding connector node of the fork
         to the first node -->
        <xsl:comment>
            <xsl:text>Connection to preceding node for:</xsl:text>
            <xsl:apply-templates select="." mode="node-identifier-string"/>
        </xsl:comment>
        <xsl:call-template name="edge">
            <xsl:with-param name="from" select="$previous"/>
            <xsl:with-param name="to" select="$first-to-link-to"/>
        </xsl:call-template>

        <!-- $next is the following connector node of the me:fork parent -->
        <xsl:variable name="next">
            <xsl:apply-templates select="parent::me:fork" mode="following-connector-node-id"/>
        </xsl:variable>

        <!-- $last is the following connector node of the last child element -->
        <xsl:variable name="last-to-link-to">
            <xsl:apply-templates select="child::*[last()]" mode="following-connector-node-id"/>
        </xsl:variable>

        <!-- this creates the edge from the following connector node of the last child,
         to the following connector node of the parent me:fork -->
        <xsl:comment>
            <xsl:text>Connection to following node for:</xsl:text>
            <xsl:apply-templates select="." mode="node-identifier-string"/>
        </xsl:comment>
        <xsl:call-template name="edge">
            <xsl:with-param name="from" select="$last-to-link-to"/>
            <xsl:with-param name="to" select="$next"/>
        </xsl:call-template>

        <!-- call the edges template for each of the child nodes -->
        <xsl:apply-templates select="child::node()" mode="edges"/>
    </xsl:template>

    <xsl:template match="me:sequence" mode="edges">
        <!-- $previous is the preceding connector node, as usual-->
        <xsl:variable name="previous">
            <xsl:apply-templates select="current()" mode="preceding-connector-node-id"/>
            <!-- <xsl:apply-templates select="preceding-sibling::*[1]" mode="following-connector-node-id"/> -->
        </xsl:variable>

        <!-- $first-to-link-to is the preceding connector node of the first child element -->
        <xsl:variable name="first-to-link-to">
            <xsl:apply-templates select="child::*[local-name() != 'optional'][1]" mode="preceding-connector-node-id"/>
        </xsl:variable>

        <!-- this creates the edge from the preceding connector node of the fork
         to the first node -->
        <xsl:comment>
            <xsl:text>Connection to preceding node for:</xsl:text>
            <xsl:apply-templates select="." mode="node-identifier-string"/>
        </xsl:comment>
        <xsl:call-template name="edge">
            <xsl:with-param name="from" select="$previous"/>
            <xsl:with-param name="to" select="$first-to-link-to"/>
            <xsl:with-param name="debug">From previous to the first node</xsl:with-param>
        </xsl:call-template>

        <!-- $next is the following connector node, as usual -->
        <xsl:variable name="next">
            <xsl:apply-templates select="current()" mode="following-connector-node-id"/>
        </xsl:variable>

        <!-- $last is the following connector node of the last child element -->
        <xsl:variable name="last-to-link-to">
            <xsl:apply-templates select="child::*[last()]" mode="following-connector-node-id"/>
        </xsl:variable>

        <!-- this creates the edge from the following connector node of the last child,
         to the following connector node of the parent me:fork -->
        <xsl:comment>
            <xsl:text>Connection to following node for:</xsl:text>
            <xsl:apply-templates select="." mode="node-identifier-string"/>
        </xsl:comment>
        <xsl:call-template name="edge">
            <xsl:with-param name="from" select="$last-to-link-to"/>
            <xsl:with-param name="to" select="$next"/>
            <xsl:with-param name="debug">From following connector to the last node</xsl:with-param>
        </xsl:call-template>

        <!-- draw the optional arc if it's optional -->
        <xsl:if test="me:optional">
            <xsl:comment>
                <xsl:text>Optionality edge for:</xsl:text>
                <xsl:apply-templates select="." mode="node-identifier-string"/>
            </xsl:comment>
            <xsl:call-template name="edge">
                <xsl:with-param name="from">
                    <xsl:apply-templates select="current()" mode="preceding-connector-node-id"/>
                </xsl:with-param>
                <xsl:with-param name="to">
                    <xsl:apply-templates select="current()" mode="following-connector-node-id"/>
                </xsl:with-param>
                <xsl:with-param name="constraint">false</xsl:with-param>
            </xsl:call-template>
        </xsl:if>

        <!-- call the edges template for each of the child nodes -->
        <xsl:apply-templates select="child::node()" mode="edges"/>
    </xsl:template>

    <!-- <optional/> can be inside sequences; ignore it -->
    <xsl:template match="me:optional" mode="edges"></xsl:template>

    <!-- not sure why it's necessary to ignore these specifically, in this mode -->
    <xsl:template match="processing-instruction()|comment()" mode="edges"/>

    <!-- This is a little code to make sure we're handling all the nodes -->
    <xsl:template match="node()" mode="edges">
        <xsl:message>Node not handled in 'edges' mode:<xsl:value-of select="local-name()"></xsl:value-of>
            <xsl:value-of select="current()"></xsl:value-of>
        </xsl:message>
    </xsl:template>

    <!-- Jump lines -->
    <xsl:template match="me:jump" mode="jumps">
        <!-- $this-id is the current node -->
        <xsl:variable name="this-id">
            <xsl:apply-templates select="current()" mode="node-id"/>
        </xsl:variable>
        <!-- $target is the node we're jumping to -->
        <xsl:variable name="target">
            <xsl:apply-templates select="//*[@id = current()/@to]" mode="preceding-connector-node-id"/>
        </xsl:variable>

        <!-- draw the connection -->
        <xsl:comment>
            <xsl:text>Jump connection for:</xsl:text>
            <xsl:apply-templates select="." mode="node-identifier-string"/>
        </xsl:comment>
        <xsl:call-template name="edge">
            <xsl:with-param name="from" select="$this-id"/>
            <xsl:with-param name="to" select="$target"/>
            <!-- jump node connections come from the bottom of the node -->
            <xsl:with-param name="direction">s</xsl:with-param>
            <!-- jumps shouldn't affect node layout -->
            <xsl:with-param name="constraint">false</xsl:with-param>
        </xsl:call-template>
    </xsl:template>

</xsl:stylesheet>