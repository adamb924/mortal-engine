# Morphological Diagrams
These are XSL files for producing [GraphViz](https://graphviz.org/) diagrams from morphological models. Depending on the complexity of the files these can be helpful, or just discouraging. 

The XSLT file `model2diagram.xsl` will translate a model into a [DotML](http://www.martin-loetzsch.de/DOTML/) file. You can then convert the DotML file to a `.dot` file with Martin Loetzsch's `dotml2dot.xml`. (I have included a copy of this file in the respository, along with its license, for convenience.) Then use `dot` to generate the final output file.

```
xsltproc --param gloss-writing-system "'en-US'"  -o mymodel-diagram.xml model2diagram.xsl mymodel.xml
xsltproc  -o mymodel.dot dotml2dot.xsl mymodel-diagram.xml
dot -Tpng mymodel.dot -o mymodel.png
```

You can add the `which-model` parameter, in which case only the specified model will be included in the diagram.
```
xsltproc --param which-model "'%2'" --param gloss-writing-system "'en-US'"  -o mymodel-diagram.xml model2diagram.xsl mymodel.xml
```


## Convenience .bat files
These are the bat files I use for my convenience.

Produce a diagram of `mymodel.xml` in PNG format:

```
./diagram-all mymodel
```

Produce a diagram of the `Nouns` model in `mymodel.xml` in PNG format:
```
./diagram mymodel Nouns
```

Note that the `.bat` files have the hardcoded `--param gloss-writing-system "'en-US'"`, which you may not like.