#! /bin/sh

destdir=$1

echo "converting RST to HTML"

for f in $destdir/_sources/*.rst; do
    b=$(basename $f)
    echo $b
    rst2html "$f" "$destdir/${b%.*}.html"
done





