#!/bin/bash

# positional arguments
folder="$1"
blocks="$2"
block_size="$3"

create_blocks(){
	cd bloques
	n=$(( blocks-1 ))
	for i in `seq 0 $n`
	do
	    touch "$i".bin	
	    echo "$i".bin created
	done
	cd ..
}

create_metadata(){
	cd Metadata
	echo "BLOCK_SIZE=$block_size
BLOCKS=$blocks
MAGIC_NUMBER=LISSANDRA" > Metadata.bin
	echo Metadata.bin created
	cd ..
}

create_bitmap(){
	cd Metadata
	printf '0%.0s' $(seq 1 $blocks) > Bitmap.bin
	cd ..
}

# Main
if [[ $# -eq 0 ]] ; then
    echo 'Please pass arguments: first is folder where LFS should be, second is amount of blocks, third is block size'
    exit 0
fi

echo $blocks blocks
echo block size: $block_size
echo directory where the FS is: $folder

mkdir $folder
cd $folder

mkdir tables
mkdir bloques
mkdir Metadata

create_blocks
create_metadata
# create_bitmap

echo REMEMBER TO EDIT LFS.cfg TO TAKE FS NEW PATH