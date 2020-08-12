#!/bin/bash

TAG=`git describe --tags`
# clean out any previous build
rm -rf v??.*
rm -rf $TAG
rm -rf v??.*
rm -rf ${TAG}.version
mkdir $TAG

ls

for olddir in 'OS='*
do
    newdir=$TAG/${olddir/CAFAna/}
    newdir=${newdir/OS=SLF6/slf6.x86_64}
    newdir=${newdir/OS=SLF7/slf7.x86_64}
    newdir=${newdir/QUALIFIER=/}
    newdir=${newdir//,/.}
    newdir=${newdir//:/.}
    echo mv $olddir $newdir
    mv $olddir $newdir
    mv $newdir/CAFAna/lib $newdir/$lib
    # will overwrite each other but should all be identical
    mv $newdir/CAFAna/ups $TAG/
    sed -i s/vXX.YY/$TAG/g ${TAG}/ups/osclib.table
    for k in `find $newdir/CAFAna -name '*.h'`
    do
        fname=${k/$newdir/}
        mkdir -p $TAG/include/`dirname $fname`
        echo cp $k $TAG/include/$fname
        cp $k $TAG/include/$fname
    done
    for k in `find $newdir/CAFAna -name '*.h' -o -name '*.cxx' -o -name '*.cc'`
    do
        fname=${k/$newdir/}
        mkdir -p $TAG/src/`dirname $fname`
        echo mv $k $TAG/src/$fname
        mv $k $TAG/src/$fname
    done
    rm -r $newdir/CAFAna
done

cp -r jenkins/version ${TAG}.version
sed -i s/vXX.YY/$TAG/g ${TAG}.version/*
