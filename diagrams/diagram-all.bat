xsltproc --param gloss-writing-system "'en-US'"  -o %1-diagram.xml model2diagram.xsl %1.xml
xsltproc  -o %1.dot dotml2dot.xsl %1-diagram.xml
dot -Tpng %1.dot -o %1.png
